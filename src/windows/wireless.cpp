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

#ifdef PLATFORM_WINDOWS

#include "kstd/platform/wireless.hpp"
#include <wlanapi.h>

#undef interface ^

namespace kstd::platform {

    auto enumerate_wlan_networks(const NetworkInterface& interface) noexcept
            -> Result<std::unordered_set<WlanNetwork>> {
        using namespace std::string_literals;
        // Cancel the function if the type of the interface is not wireless
        if(interface.get_type() != InterfaceType::WIRELESS) {
            return Error {fmt::format("The interface type is not wireless ({})",
                                      get_interface_type_name(interface.get_type()))};
        }

        // Open WLAN handle
        HANDLE client_handle = nullptr;
        DWORD current_version = 0;
        if(FAILED(WlanOpenHandle(2, nullptr, &current_version, &client_handle))) {
            return Error {"Unable to enumerate Wi-Fi networks: Unable to open handle for Windows WLAN API"s};
        }

        // Get a list of all WLAN-capable interfaces
        PWLAN_INTERFACE_INFO_LIST interface_list = nullptr;
        if(FAILED(WlanEnumInterfaces(client_handle, nullptr, &interface_list))) {
            return Error {"Unable to enumerate Wi-Fi networks: Unable to get a list of all WLAN-capable interfaces"s};
        }

        // Enumerate interfaces and get information from the right interface
        std::unordered_set<WlanNetwork> available_networks {};
        for(int i = 0; i < interface_list->dwNumberOfItems; ++i) {
            const auto* wlan_info = &interface_list->InterfaceInfo[i];// NOLINT
            if(utils::to_mbs({static_cast<const WCHAR*>(wlan_info->strInterfaceDescription)}) !=
               interface.get_description()) {
                continue;
            }

            // Get available Wi-Fi networks by interface
            PWLAN_AVAILABLE_NETWORK_LIST wlan_networks = nullptr;
            if(FAILED(WlanGetAvailableNetworkList(client_handle, &wlan_info->InterfaceGuid, 0, nullptr,
                                                  &wlan_networks))) {
                return Error {"Unable to enumerate Wi-Fi networks: Unable to get list of available Wi-Fi networks"s};
            }

            for(int j = 0; j < wlan_networks->dwNumberOfItems; ++j) {
                PWLAN_AVAILABLE_NETWORK wlan_network = &wlan_networks->Network[j];// NOLINT

                // Get SSID
                std::string ssid_string {};
                for(int k = 0; k < wlan_network->dot11Ssid.uSSIDLength; ++k) {
                    ssid_string += static_cast<char>(wlan_network->dot11Ssid.ucSSID[k]);// NOLINT
                }
                Option<std::string> ssid = ssid_string;
                if(ssid->empty()) {
                    ssid = {};
                }

                // Get BSS list
                PWLAN_BSS_LIST bss_list = nullptr;
                if(FAILED(WlanGetNetworkBssList(client_handle, &wlan_info->InterfaceGuid, &wlan_network->dot11Ssid,
                                                wlan_network->dot11BssType, wlan_network->bSecurityEnabled, nullptr,
                                                &bss_list))) {

                    const auto ssid_name = ssid.get_or("Hidden Network");
                    return Error {
                            fmt::format("Unable to enumerate Wi-Fi networks: Unable to get BSS of {}", ssid_name)};
                }

                // Get MAC address
                std::string mac_addr {};
                const auto first_bss_entry = bss_list->wlanBssEntries[0];// Better method for the MAC addr?
                for(int k = 0; k < 6; ++k) {
                    if(k == 0) {
                        mac_addr.append(fmt::format("{:02X}", first_bss_entry.dot11Bssid[k]));
                    }
                    else {
                        mac_addr.append(fmt::format(":{:02X}", first_bss_entry.dot11Bssid[k]));
                    }
                }

                // Push and cleanup
                available_networks.insert(
                        {mac_addr, ssid, first_bss_entry.ulChCenterFrequency / 1000, wlan_network->wlanSignalQuality, false});
                WlanFreeMemory(bss_list);
            }

            WlanFreeMemory(wlan_networks);
            break;
        }

        // Cleanup
        WlanFreeMemory(interface_list);
        CloseHandle(client_handle);
        return {available_networks};
    }


}// namespace kstd::platform

#endif