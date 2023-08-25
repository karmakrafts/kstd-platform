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
#include <filesystem>
#include <fstream>
#include <kstd/safe_alloc.hpp>

#define INT_FILE_CAST_FUNCTOR(t)                                                                                       \
    [](auto value) noexcept -> auto {                                                                                  \
        return static_cast<t>(std::stoi(value));                                                                       \
    }

namespace kstd::platform {
    using namespace std::string_literals;
    using NativeInterfaceAddress = struct ifaddrs;

    auto read_file(const std::filesystem::path& path) -> Option<std::string> {
        if(!std::filesystem::exists(path)) {
            return {};
        }

        std::ifstream stream {path};
        std::string content {};
        stream >> content;
        if(content.empty()) {
            return {};
        }

        return content;
    }

    auto is_multicast(usize addr_family, std::string address) -> bool {
        switch(addr_family) {
            case AF_INET: return (ntohl(inet_addr(address.data())) & 0xF0000000) == 0xE0000000;
            case AF_INET6: {
                sockaddr_in6 sockaddr6 {};
                inet_pton(AF_INET6, address.data(), &(sockaddr6.sin6_addr));
                return (sockaddr6.sin6_addr.__in6_u.__u6_addr8[0] == 0xff);// NOLINT
            }
            default: return false;
        }
    }

    auto enumerate_interfaces(const InterfaceInfoFlags flags) noexcept// NOLINT
            -> Result<std::unordered_set<NetworkInterface>> {
        NativeInterfaceAddress* addresses = nullptr;
        if(::getifaddrs(&addresses) < 0) {
            return Error {get_last_error()};
        }

        // Open socket
        const auto socket_descriptor = open_socket();
        if(!socket_descriptor) {
            return Error {socket_descriptor.get_error()};
        }

        // Enumerate interfaces
        std::vector<NetworkInterface> interfaces {};
        for(const auto* addr = addresses; addr != nullptr; addr = addr->ifa_next) {
            // Get description
            const auto* description {addr->ifa_name};

            // clang-format off
            auto original_interface = streams::stream(interfaces).find_first([&](auto& interface) {
                return interface.get_description() == description;
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
                        if(::inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(addr->ifa_addr)->sin_addr,// NOLINT
                                       data.data(), sizeof(data)) == nullptr) {
                            break;
                        }
                        address = std::string {data.cbegin(), data.cend()};
                        break;
                    }
                    case AF_INET6: {
                        std::array<char, INET6_ADDRSTRLEN> data {};
                        if(::inet_ntop(AF_INET6, &reinterpret_cast<sockaddr_in6*>(addr->ifa_addr)->sin6_addr,// NOLINT
                                       data.data(), data.size()) == nullptr) {
                            break;
                        }
                        address = std::string {data.cbegin(), data.cend()};
                        break;
                    }
                    case AF_PACKET: {
                        const auto* mac = reinterpret_cast<sockaddr_ll*>(addr->ifa_addr);// NOLINT
                        address = fmt::format("{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}", mac->sll_addr[0],
                                              mac->sll_addr[1], mac->sll_addr[2], mac->sll_addr[3], mac->sll_addr[4],
                                              mac->sll_addr[5]);
                    }
                    default: break;
                }
                address->resize(libc::get_string_length(address->c_str()));

                // TODO: Look into the routing table and check if the address is an Anycast address
                // Check if routing scheme is Multicast or Unicast
                auto routing_scheme = RoutingScheme::UNKNOWN;
                if(address.has_value()) {
                    if(is_multicast(addr_family, *address)) {
                        routing_scheme = RoutingScheme::MULTICAST;
                    }
                    else {
                        routing_scheme = RoutingScheme::UNICAST;
                    }
                }

                // Add address to original interface if there is one. If not, add the address to the addrs set
                if(original_interface.has_value()) {
                    original_interface->_addresses.insert(
                            InterfaceAddress {address, static_cast<AddressFamily>(addr_family), routing_scheme});
                }
                else {
                    addrs.insert(InterfaceAddress {address, static_cast<AddressFamily>(addr_family), routing_scheme});
                }
            }

            // Add interface only if there is no original interface
            if(original_interface.is_empty()) {
                // Create if path
                std::array<char, max_path> buffer {};
                const auto path = fmt::format("/sys/class/net/{}", description);
                if(::readlink(path.c_str(), buffer.data(), max_path) == -1) {
                    return Error {get_last_error()};
                }

                // Create path string
                std::filesystem::current_path("/sys/class/net");
                const auto if_path = std::filesystem::canonical(std::string {buffer.data()});

                // clang-format off
                // Interface Speed
                auto speed = read_file(if_path / "speed").map(INT_FILE_CAST_FUNCTOR(usize));
                if (speed.has_value() && *speed == std::numeric_limits<usize>::max()) {
                    speed = {};
                }

                // Get type
                auto type = read_file(if_path / "type").map(INT_FILE_CAST_FUNCTOR(InterfaceType))
                    .get_or(InterfaceType::UNKNOWN);
                // clang-format on

                // Detect if interface has installed IEEE 802.11 extensions. If yes, collect all information with
                // syscalls
                iwreq request {};
                libc::copy_string(static_cast<char*>(request.ifr_ifrn.ifrn_name), description);// NOLINT
                if(sys::ioctl(static_cast<NativeFileHandle>(*socket_descriptor), SIOCGIWNAME, &request) >= 0) {
                    type = InterfaceType::WIRELESS;
                }

                // Get MTU
                const usize mtu = read_file(if_path / "mtu").map(INT_FILE_CAST_FUNCTOR(usize)).get_or(0);

                // Push new interface
                interfaces.emplace_back(if_path, description, std::move(addrs), speed, type, mtu);
            }
        }

        freeifaddrs(addresses);
        close(static_cast<int>(*socket_descriptor));
        return std::unordered_set<NetworkInterface> {interfaces.begin(), interfaces.end()};
    }
}// namespace kstd::platform

#endif
