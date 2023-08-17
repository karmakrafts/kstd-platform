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
 * @since 17/08/2023
 */

#ifdef PLATFORM_LINUX

#include "kstd/platform/network.hpp"
#include "kstd/platform/platform.hpp"
#include <unistd.h>
#include <ifaddrs.h>
#include <iostream>
#include <array>

namespace kstd::platform {
    using namespace std::string_literals;
    using InterfaceAddress = struct ifaddrs;

    auto enumerate_interfaces() noexcept -> kstd::Result<std::vector<NetworkInterface>> {
        InterfaceAddress *addresses = nullptr;
        if (getifaddrs(&addresses) < 0) {

        }

        std::vector<NetworkInterface> interfaces {};
        for (const auto *addr = addresses; addr != nullptr; addr = addr->ifa_next) {
            const auto description = std::string {addr->ifa_name};

            std::array<char, max_path> buffer {};
            auto exe_path = fmt::format("/sys/class/net/{}", description);
            if(::readlink(exe_path.c_str(), buffer.data(), max_path) == -1) {
                return Error {get_last_error()};
            }

            std::unordered_set<AddressFamily> address_families {};
            if (addr->ifa_addr != nullptr) {
                address_families.insert(static_cast<AddressFamily>(addr->ifa_addr->sa_family));
            }

            interfaces.push_back(NetworkInterface {buffer.data(), description, address_families});
        }


        freeifaddrs(addresses);
        return interfaces;
    }
}// namespace kstd::platform

#endif