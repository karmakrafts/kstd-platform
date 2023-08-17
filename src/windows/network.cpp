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
#include <kstd/streams/stream.hpp>

namespace kstd::platform {

    auto enumerate_interfaces() noexcept -> Result<std::vector<NetworkInterface>> {
        using namespace std::string_literals;

        // Determine size of adapter addresses
        usize adapter_addresses_size = 0;
        if(FAILED(GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr,
                                       reinterpret_cast<PULONG>(&adapter_addresses_size)))) {// NOLINT
            return Error {"Unable to allocate adapter addresses information: Unable to determine size of buffer"s};
        }

        // Allocate adapter addresses holder and get address information
        auto* adapter_addresses = static_cast<PIP_ADAPTER_ADDRESSES>(libc::malloc(adapter_addresses_size));// NOLINT
        if(FAILED(GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, adapter_addresses,
                                       reinterpret_cast<PULONG>(&adapter_addresses_size)))) {// NOLINT
            libc::free(adapter_addresses);                                                   // NOLINT
            return Error {get_last_error()};
        }

        // Determine size of MIB Interface table
        usize mib_if_size = 0;
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
            return Error {get_last_error()};
        }

        // Construct interface vector
        std::vector<NetworkInterface> interfaces {};
        for(const auto row : Slice {static_cast<PMIB_IFROW>(table->table), table->dwNumEntries * sizeof(MIB_IFROW)}) {
            // Generate name and description
            const auto description =
                    std::string {reinterpret_cast<const char*>(row.bDescr), row.dwDescrLen - 1};// NOLINT
            const auto name = utils::to_mbs(static_cast<const WCHAR*>(row.wszName));

            // Enumerate over adapter addresses information and save the correct data
            auto addresses =
                    streams::stream(adapter_addresses, KSTD_PTR_FIELD_FUNCTOR(Next)).find_first([&](auto& addr) {
                        return name.find(addr.AdapterName) != std::string::npos;
                    });

            // If some adapter addresses are found, parse the address information
            std::unordered_set<AddressFamily> address_families {};
            if(addresses) {
                constexpr auto map_function = KSTD_SCAST_FIELD_FUNCTOR(Address.lpSockaddr->sa_family, AddressFamily);

                // Enumerate Unicast addresses and insert address families
                streams::stream(addresses->FirstUnicastAddress, KSTD_PTR_FIELD_FUNCTOR(Next))
                        .map(map_function)
                        .collect_into(address_families, streams::collectors::insert);

                // Enumerate Multicast addresses and insert address families
                streams::stream(addresses->FirstMulticastAddress, KSTD_PTR_FIELD_FUNCTOR(Next))
                        .map(map_function)
                        .collect_into(address_families, streams::collectors::insert);

                // Enumerate Anycast addresses and insert address families
                streams::stream(addresses->FirstAnycastAddress, KSTD_PTR_FIELD_FUNCTOR(Next))
                        .map(map_function)
                        .collect_into(address_families, streams::collectors::insert);
            }

            // Push interface (Speed from bits to megabytes)
            Option<usize> speed = row.dwSpeed / 1024 / 1024;
            if (*speed == 0) {
                speed = {};
            }

            interfaces.push_back(NetworkInterface {name, description, std::move(address_families), speed});
        }

        // Free information and return interfaces
        libc::free(table);            // NOLINT
        libc::free(adapter_addresses);// NOLINT
        return interfaces;
    }

}// namespace kstd::platform

#endif