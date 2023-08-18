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

#pragma once

#include <fmt/format.h>
#include <kstd/option.hpp>
#include <kstd/result.hpp>
#include <kstd/streams/stream.hpp>
#include <unordered_set>

#ifdef PLATFORM_WINDOWS
#include <Winsock2.h>
#include <iphlpapi.h>
#include <kstd/types.hpp>
#else
#include <ifaddrs.h>
#include <netdb.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#endif

namespace kstd::platform {

    enum class AddressFamily : u8 {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX,
        IPX = AF_IPX,
        APPLE_TALK = AF_APPLETALK
    };

    enum class RoutingScheme : u8 {
        UNICAST,
        MULTICAST,
        ANYCAST,
        UNKNOWN
    };

#ifdef PLATFORM_WINDOWS
    enum class InterfaceType : u16 {
        LOOPBACK = IF_TYPE_SOFTWARE_LOOPBACK,
        ETHERNET = IF_TYPE_ETHERNET_CSMACD,
        WIRELESS = IF_TYPE_IEEE80211,
        PPP = IF_TYPE_PPP,
        ATM = IF_TYPE_ATM,
        TUNNEL = IF_TYPE_TUNNEL
    };
#else
    enum class InterfaceType : u16 {
        LOOPBACK = 772,
        ETHERNET = 1,
        WIRELESS = 65535,
        PPP = 512,
        ATM = 19,
        TUNNEL = 768
    };
#endif

    struct InterfaceAddress final {
        Option<std::string> address;
        AddressFamily family;
        RoutingScheme routing_scheme;

        [[nodiscard]] inline auto operator==(const InterfaceAddress& other) const noexcept -> bool {
            return address == other.address && family == other.family && routing_scheme == other.routing_scheme;
        }

        [[nodiscard]] inline auto operator!=(const InterfaceAddress& other) const noexcept -> bool {
            return !(*this == other);
        }
    };

    [[nodiscard]] inline auto get_interface_type_name(const InterfaceType type) noexcept -> std::string {
        switch(type) {
            case InterfaceType::LOOPBACK: return "Loopback";
            case InterfaceType::ETHERNET: return "Ethernet";
            case InterfaceType::WIRELESS: return "Wireless";
            case InterfaceType::TUNNEL: return "Tunnel";
            case InterfaceType::PPP: return "PPP";
            case InterfaceType::ATM: return "ATM";
            default: return "Unknown";
        }
    }

    [[nodiscard]] inline auto get_address_family_name(const AddressFamily family) noexcept -> std::string {
        switch(family) {
            case AddressFamily::IPv4: return "IPv4";
            case AddressFamily::IPv6: return "IPv6";
            case AddressFamily::UNIX: return "UNIX";
            case AddressFamily::IPX: return "IPX";
            case AddressFamily::APPLE_TALK: return "AppleTalk";
            default: return "Unknown";
        }
    }

    [[nodiscard]] inline auto get_routing_scheme_name(const RoutingScheme type) noexcept -> std::string {
        switch(type) {
            case RoutingScheme::UNICAST: return "Unicast";
            case RoutingScheme::MULTICAST: return "Multicast";
            case RoutingScheme::ANYCAST: return "Anycast";
            default: return "Unknown";
        }
    }
}// namespace kstd::platform

KSTD_DEFAULT_HASH((kstd::platform::InterfaceAddress), value.address, value.family, value.routing_scheme);

namespace kstd::platform {
    struct NetworkInterface final {
        std::string name;
        std::string description;
        std::unordered_set<InterfaceAddress> addresses;
        Option<kstd::usize> link_speed;
        InterfaceType type;

        [[nodiscard]] inline auto has_addresses_by_family(const AddressFamily family) const noexcept -> bool {
            // clang-format off
            return streams::stream(addresses).map(KSTD_FIELD_FUNCTOR(family)).find_first([&](auto value) {
                return value == family;
            }).has_value();
            // clang-format on
        }

        [[nodiscard]] inline auto has_addresses_with_routing_scheme(const RoutingScheme scheme) const noexcept -> bool {
            // clang-format off
            return streams::stream(addresses).map(KSTD_FIELD_FUNCTOR(routing_scheme)).find_first([&](auto value) {
                return value == scheme;
            }).has_value();
            // clang-format on
        }
    };

    [[nodiscard]] auto enumerate_interfaces() noexcept -> Result<std::vector<NetworkInterface>>;
}// namespace kstd::platform
