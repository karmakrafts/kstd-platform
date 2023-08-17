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
#include "kstd/platform/file_mapping.hpp"
#include "kstd/platform/platform.hpp"
#include <array>
#include <ifaddrs.h>
#include <fstream>
#include <filesystem>
#include <kstd/safe_alloc.hpp>
#include <unistd.h>

namespace kstd::platform {
    using namespace std::string_literals;
    using InterfaceAddress = struct ifaddrs;

    auto enumerate_interfaces() noexcept -> Result<std::vector<NetworkInterface>> {
        InterfaceAddress* addresses = nullptr;
        if(getifaddrs(&addresses) < 0) {
            return Error {get_last_error()};
        }

        std::vector<NetworkInterface> interfaces {};
        for(const auto* addr = addresses; addr != nullptr; addr = addr->ifa_next) {
            const auto description = std::string {addr->ifa_name};

            // Create if path
            std::array<char, max_path> buffer {};
            const auto path = fmt::format("/sys/class/net/{}", description);
            if(::readlink(path.c_str(), buffer.data(), max_path) == -1) {
                return Error {get_last_error()};
            }

            // Get address family of interface
            std::unordered_set<AddressFamily> address_families {};
            if(addr->ifa_addr != nullptr) {
                address_families.insert(static_cast<AddressFamily>(addr->ifa_addr->sa_family));
            }

            // Create path string
            std::filesystem::current_path("/sys/class/net");
            const auto if_path = std::filesystem::canonical(std::string {buffer.data()});

            // Get interface speed
            const auto speed_path = if_path / "speed";
            Option<usize> interface_speed {};
            if(std::filesystem::exists(speed_path)) {
                std::ifstream stream {speed_path};
                std::string speed {};
                stream >> speed;
                if (!speed.empty()) {
                    interface_speed = std::stoi(speed);
                }
            }

            // Push new interface
            interfaces.push_back(NetworkInterface {if_path, description, address_families, interface_speed});
        }

        freeifaddrs(addresses);
        return interfaces;
    }
}// namespace kstd::platform

#endif