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

#include <kstd/slice.hpp>

namespace kstd::platform {

    auto enumerate_interfaces() noexcept -> kstd::Result<std::vector<NetworkInterface>> {
        using namespace std::string_literals;

        // Determine size of adapter addresses
        kstd::usize adapter_addresses_size = 0;
        if(FAILED(GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr,
                                       reinterpret_cast<PULONG>(&adapter_addresses_size)))) {// NOLINT
            return Error {"Unable to allocate adapter addresses information: Unable to determine size of buffer"s};
        }

        // Allocate adapter addresses holder and get address information
        auto* adapter_addresses = static_cast<PIP_ADAPTER_ADDRESSES>(libc::malloc(adapter_addresses_size));// NOLINT
        if(FAILED(GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, adapter_addresses,
                                       reinterpret_cast<PULONG>(&adapter_addresses_size)))) {// NOLINT
            libc::free(adapter_addresses);                                                   // NOLINT
            return Error {fmt::format("Unable to allocate interface Table: Unable to get Interface table ({})",
                                      get_last_error())};
        }

        // Determine size of MIB Interface table
        kstd::usize mib_if_size = 0;
        if(GetIfTable(nullptr, reinterpret_cast<PULONG>(&mib_if_size), FALSE) != ERROR_INSUFFICIENT_BUFFER) {// NOLINT
            libc::free(adapter_addresses);                                                                   // NOLINT
            return Error {"Unable to allocate interface table: Unable to determine size of buffer"s};
        }

        // Allocate table and get interface table
        auto* table = static_cast<MIB_IFTABLE*>(libc::malloc(mib_if_size));           // NOLINT
        if(FAILED(GetIfTable(table, reinterpret_cast<PULONG>(&mib_if_size), FALSE))) {// NOLINT
            // Free information and return error
            libc::free(table);            // NOLINT
            libc::free(adapter_addresses);// NOLINT
            return Error {fmt::format("Unable to allocate interface Table: Unable to get Interface table ({})",
                                      get_last_error())};
        }

        // Construct interface vector
        std::vector<NetworkInterface> interfaces {};
        for(const auto row : Slice {static_cast<PMIB_IFROW>(table->table), table->dwNumEntries * sizeof(MIB_IFROW)}) {
            // Generate name and description
            const auto description =
                    std::string {reinterpret_cast<const char*>(row.bDescr), row.dwDescrLen - 1};// NOLINT
            const auto name = utils::to_mbs(static_cast<const WCHAR*>(row.wszName));

            // Enumerate over adapter addresses information and save the correct data
            PIP_ADAPTER_ADDRESSES curr_addresses = nullptr;
            for(auto* addresses = adapter_addresses; addresses != nullptr; addresses = addresses->Next) {
                if(name.find(addresses->AdapterName) == std::string::npos) {
                    continue;
                }

                curr_addresses = addresses;
                break;
            }

            // If some adapter addresses are found, parse the address information
            std::unordered_set<AddressFamily> address_families {};
            if(curr_addresses != nullptr) {
                // Enumerate Unicast addresses and insert address families
                for(const auto* addr = curr_addresses->FirstUnicastAddress; addr != nullptr; addr = addr->Next) {
                    address_families.insert(static_cast<AddressFamily>(addr->Address.lpSockaddr->sa_family));
                }

                // Enumerate Multicast addresses and insert address families
                for(const auto* addr = curr_addresses->FirstMulticastAddress; addr != nullptr; addr = addr->Next) {
                    address_families.insert(static_cast<AddressFamily>(addr->Address.lpSockaddr->sa_family));
                }

                // Enumerate Anycast addresses and insert address families
                for(const auto* addr = curr_addresses->FirstAnycastAddress; addr != nullptr; addr = addr->Next) {
                    address_families.insert(static_cast<AddressFamily>(addr->Address.lpSockaddr->sa_family));
                }
            }
            interfaces.push_back(NetworkInterface {name, description, std::move(address_families)});
        }

        // Free information and return interfaces
        libc::free(table);            // NOLINT
        libc::free(adapter_addresses);// NOLINT
        return interfaces;
    }

}// namespace kstd::platform

#endif