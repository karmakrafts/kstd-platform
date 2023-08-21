// Copyright 2023 Karma Krafts & associates
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @author Cedric Hammes
 * @since 12/08/2023
 */

#ifdef PLATFORM_WINDOWS

#define NOMINMAX

#include "kstd/platform/network.hpp"
#include "kstd/platform/platform.hpp"

#undef CALLBACK

#include <kstd/slice.hpp>

namespace kstd::platform {
    [[nodiscard]] inline auto get_address_string_by_address(LPSOCKADDR address) -> Option<std::string> {
        DWORD length = INET6_ADDRSTRLEN;
        std::array<wchar_t, INET6_ADDRSTRLEN> address_buffer {};
        switch(address->sa_family) {
            case AF_INET: {
                ::InetNtopW(AF_INET, &reinterpret_cast<sockaddr_in*>(address)->sin_addr, address_buffer.data(),// NOLINT
                            length);                                                                           // NOLINT
                break;
            }
            case AF_INET6: {
                ::InetNtopW(AF_INET6, &reinterpret_cast<sockaddr_in6*>(address)->sin6_addr,// NOLINT
                            address_buffer.data(), length);
                break;
            }
            default: return {};
        }
        return kstd::utils::to_mbs({address_buffer.data(), static_cast<usize>(length)});
    }

    auto enumerate_interfaces(const InterfaceInfoFlags flags) noexcept -> Result<std::unordered_set<NetworkInterface>> {
        using namespace std::string_literals;

        constexpr auto adapter_addrs_flags =
                GAA_FLAG_INCLUDE_GATEWAYS | GAA_FLAG_INCLUDE_ALL_INTERFACES | GAA_FLAG_INCLUDE_PREFIX;

        // Determine size of adapter addresses
        usize adapter_addresses_size = 0;
        if(FAILED(::GetAdaptersAddresses(AF_UNSPEC, adapter_addrs_flags, nullptr, nullptr,
                                         reinterpret_cast<PULONG>(&adapter_addresses_size)))) {// NOLINT
            return Error {"Unable to allocate adapter addresses information: Unable to determine size of buffer"s};
        }

        // Allocate adapter addresses holder and get address information
        auto* adapter_addresses = static_cast<PIP_ADAPTER_ADDRESSES>(libc::malloc(adapter_addresses_size));// NOLINT
        if(FAILED(::GetAdaptersAddresses(AF_UNSPEC, adapter_addrs_flags, nullptr, adapter_addresses,
                                         reinterpret_cast<PULONG>(&adapter_addresses_size)))) {// NOLINT
            libc::free(adapter_addresses);                                                     // NOLINT
            return Error {get_last_error()};
        }

        // Determine size of MIB Interface table
        usize mib_if_size = 0;
        if(::GetIfTable(nullptr, reinterpret_cast<PULONG>(&mib_if_size), FALSE) != ERROR_INSUFFICIENT_BUFFER) {// NOLINT
            libc::free(adapter_addresses);                                                                     // NOLINT
            return Error {"Unable to allocate interface table: Unable to determine size of buffer"s};
        }

        // Allocate table and get interface table
        auto* table = static_cast<MIB_IFTABLE*>(libc::malloc(mib_if_size));             // NOLINT
        if(FAILED(::GetIfTable(table, reinterpret_cast<PULONG>(&mib_if_size), FALSE))) {// NOLINT
            // Free information and return error
            libc::free(table);            // NOLINT
            libc::free(adapter_addresses);// NOLINT
            return Error {get_last_error()};
        }

        // Open Windows WLAN client handle
        auto const wlan_flag_set = (flags & InterfaceInfoFlags::WIRELESS) == InterfaceInfoFlags::WIRELESS;

        HANDLE wlan_client_handle = nullptr;
        DWORD client_version = 2;
        DWORD current_version = 0;
        if(wlan_flag_set && FAILED(WlanOpenHandle(client_version, nullptr, &current_version, &wlan_client_handle))) {
            return Error {get_last_error()};
        }

        // Enumerate WLAN interfaces
        PWLAN_INTERFACE_INFO_LIST wlan_interface_list = nullptr;
        if(wlan_flag_set && FAILED(WlanEnumInterfaces(wlan_client_handle, nullptr, &wlan_interface_list))) {
            return Error {get_last_error()};
        }

        // Construct interface vector
        std::unordered_set<NetworkInterface> interfaces {};
        for(const auto row : Slice {static_cast<PMIB_IFROW>(table->table), table->dwNumEntries * sizeof(MIB_IFROW)}) {
            // Generate name and description
            const auto desc = std::string {reinterpret_cast<const char*>(row.bDescr), row.dwDescrLen - 1};// NOLINT
            const auto name = utils::to_mbs(static_cast<const WCHAR*>(row.wszName));

            // Enumerate over adapter addresses information and save the correct data
            auto addresses =
                    streams::stream(adapter_addresses, KSTD_PTR_FIELD_FUNCTOR(Next)).find_first([&](auto& addr) {
                        return name.find(addr.AdapterName) != std::string::npos;
                    });

            // If some adapter addresses are found, parse the address information
            std::unordered_set<InterfaceAddress> if_addrs {};
            auto current_address_type = RoutingScheme::UNICAST;
            if(addresses) {
                // Create function to map from Windows address construct to Interface address
                const auto map_function = [&](auto& value) noexcept {
                    Option<std::string> address {};
                    // Check if address family can be converted by the API
                    switch(value.Address.lpSockaddr->sa_family) {
                        case AF_INET:
                        case AF_INET6: {
                            // Format IPv4 and IPv6 addresses to string if possible
                            const auto addr = get_address_string_by_address(value.Address.lpSockaddr);
                            if(!addr) {
                                break;
                            }

                            address = *addr;
                            break;
                        }
                        default: break;
                    }

                    // Construct interface address
                    return InterfaceAddress {address, static_cast<AddressFamily>(value.Address.lpSockaddr->sa_family),
                                             current_address_type};
                };

                // Enumerate Unicast addresses and insert address families
                streams::stream(addresses->FirstUnicastAddress, KSTD_PTR_FIELD_FUNCTOR(Next))
                        .map(map_function)
                        .collect_into(if_addrs, streams::collectors::insert);

                // Enumerate Multicast addresses and insert address families
                current_address_type = RoutingScheme::MULTICAST;
                streams::stream(addresses->FirstMulticastAddress, KSTD_PTR_FIELD_FUNCTOR(Next))
                        .map(map_function)
                        .collect_into(if_addrs, streams::collectors::insert);

                // Enumerate Anycast addresses and insert address families
                current_address_type = RoutingScheme::ANYCAST;
                streams::stream(addresses->FirstAnycastAddress, KSTD_PTR_FIELD_FUNCTOR(Next))
                        .map(map_function)
                        .collect_into(if_addrs, streams::collectors::insert);
            }

            const auto type = static_cast<InterfaceType>(row.dwType);

            // Query information about Wireless network and connection if flags are set
            Option<WirelessInformation> wireless_information {};
            if(wlan_flag_set && type == InterfaceType::WIRELESS) {
                // TODO: Add wireless information if interface is wireless
                // https://learn.microsoft.com/de-de/windows/win32/api/wlanapi/nf-wlanapi-wlangetavailablenetworklist

                for(int i = 0; i < wlan_interface_list->dwNumberOfItems; ++i) {
                    if (wireless_information.has_value()) {
                        break;
                    }

                    PWLAN_INTERFACE_INFO wlan_info = &wlan_interface_list->InterfaceInfo[i];// NOLINT

                    // Get all available WLAN networks
                    PWLAN_AVAILABLE_NETWORK_LIST available_network_list = nullptr;
                    if(FAILED(WlanGetAvailableNetworkList(wlan_client_handle, &wlan_info->InterfaceGuid, 0, nullptr,
                                                          &available_network_list))) {
                        return Error {fmt::format("Unable to enumerate available networks for {} => {}", desc,
                                                  get_last_error())};
                    }

                    // Enumerate all available WLAN networks
                    for(int j = 0; j < available_network_list->dwNumberOfItems; ++j) {
                        PWLAN_AVAILABLE_NETWORK available_network = &available_network_list->Network[j];// NOLINT

                        // Check if device is currently connected to device
                        if((available_network->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED) ==
                           WLAN_AVAILABLE_NETWORK_CONNECTED) {
                            // Get SSID of network
                            const auto ssid_length = available_network->dot11Ssid.uSSIDLength;
                            std::string ssid(ssid_length + 1, '\0');
                            if(ssid_length == 0) {
                                ssid = "Hidden Network";
                            }
                            else {
                                libc::memcpy(ssid.data(),
                                             reinterpret_cast<const char*>(available_network->dot11Ssid.ucSSID),// NOLINT
                                             ssid_length);
                                ssid.resize(libc::get_string_length(ssid.c_str()));
                            }

                            // Insert wireless information
                            wireless_information = {ssid};
                            break;
                        }
                    }

                    // Cleanup available WLAN network list
                    if(available_network_list != nullptr) {
                        WlanFreeMemory(available_network_list);
                    }
                }
            }

            // Add MAC address to addresses
            auto const phys_addr_len = row.dwPhysAddrLen;
            std::string mac_address = phys_addr_len > 0 ? "" : "00:00:00:00:00:00";
            for(int i = 0; i < phys_addr_len; i++) {
                if(i < phys_addr_len - 1) {
                    mac_address.append(fmt::format("{:02X}:", row.bPhysAddr[i]));// NOLINT
                }
                else {
                    mac_address.append(fmt::format("{:02X}", row.bPhysAddr[i]));// NOLINT
                }
            }

            if_addrs.insert({mac_address, AddressFamily::MAC, RoutingScheme::UNKNOWN});

            // Push interface (Speed from bits to megabytes)
            Option<usize> speed = row.dwSpeed / 1024 / 1024;
            if(*speed == 0) {
                speed = {};
            }

            interfaces.insert({name, desc, std::move(if_addrs), wireless_information, speed, type, row.dwMtu});
        }

        // Free wlan interface list in memory
        if(wlan_interface_list != nullptr) {
            WlanFreeMemory(wlan_interface_list);
        }

        // Close handle for WLAN client, if handle is valid
        if(wlan_client_handle != nullptr) {
            CloseHandle(wlan_client_handle);
        }

        // Free information and return interfaces
        libc::free(table);            // NOLINT
        libc::free(adapter_addresses);// NOLINT
        return interfaces;
    }

}// namespace kstd::platform

#endif