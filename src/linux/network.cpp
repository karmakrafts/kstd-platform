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
#include <filesystem>
#include <fstream>
#include <ifaddrs.h>
#include <iostream>
#include <kstd/safe_alloc.hpp>
#include <unistd.h>

namespace kstd::platform {
    using namespace std::string_literals;
    using InterfaceAddresses = struct ifaddrs;

    auto enumerate_interfaces() noexcept -> Result<std::vector<NetworkInterface>> {
        InterfaceAddresses* addresses = nullptr;
        if(getifaddrs(&addresses) < 0) {
            return Error {get_last_error()};
        }

        std::vector<NetworkInterface> interfaces {};
        for(const auto* addr = addresses; addr != nullptr; addr = addr->ifa_next) {
            // Get description
            const auto description = std::string {addr->ifa_name};

            // clang-format off
            Option<NetworkInterface&> original_interface = streams::stream(interfaces).find_first([&](auto& interface) {
                return interface.description == description;
            });
            // clang-format on

            // Get address family of interface
            std::unordered_set<InterfaceAddress> addrs {};
            if(addr->ifa_addr != nullptr) {
                Option<std::string> address {};
                const auto addr_family = addr->ifa_addr->sa_family;

                // Format address to string
                switch(addr->ifa_addr->sa_family) {
                    case AF_INET: {
                        std::array<char, INET_ADDRSTRLEN> data {};
                        if(inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(addr->ifa_addr)->sin_addr, data.data(),
                                     sizeof(data)) == nullptr) {
                            break;
                        }
                        address = std::string {data.cbegin(), data.cend()};
                        break;
                    }
                    case AF_INET6: {
                        std::array<char, INET6_ADDRSTRLEN> data {};
                        if(inet_ntop(AF_INET6, &reinterpret_cast<sockaddr_in6*>(addr->ifa_addr)->sin6_addr, data.data(),
                                     data.size()) == nullptr) {
                            break;
                        }
                        address = std::string {data.cbegin(), data.cend()};
                        break;
                    }
                    default: break;
                }

                // Add address to original interface if there is one. If not, add the address to the addrs set
                if (original_interface.has_value()) {
                    original_interface->addresses.insert(InterfaceAddress {address, static_cast<AddressFamily>(addr_family), RoutingScheme::UNICAST});
                } else {
                    addrs.insert(InterfaceAddress {address, static_cast<AddressFamily>(addr_family), RoutingScheme::UNICAST});
                }
            }

            // Add interface only if there is no original interface
            if (original_interface.is_empty()) {
                // Create if path
                std::array<char, max_path> buffer {};
                const auto path = fmt::format("/sys/class/net/{}", description);
                if(::readlink(path.c_str(), buffer.data(), max_path) == -1) {
                    return Error {get_last_error()};
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
                    if(!speed.empty()) {
                        interface_speed = std::stoi(speed);
                    }
                }

                // Push new interface
                interfaces.push_back(NetworkInterface {if_path, description, std::move(addrs), interface_speed});
            }
        }

        freeifaddrs(addresses);
        return interfaces;
    }
}// namespace kstd::platform

#endif