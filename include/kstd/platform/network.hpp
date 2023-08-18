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
#include <utility>
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <sys/socket.h>
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

    enum class InterfaceType : u16 {
#ifdef PLATFORM_WINDOWS
        LOOPBACK = IF_TYPE_SOFTWARE_LOOPBACK,
        ETHERNET = IF_TYPE_ETHERNET_CSMACD,
        WIRELESS = IF_TYPE_IEEE80211,
        PPP = IF_TYPE_PPP,
        ATM = IF_TYPE_ATM,
        TUNNEL = IF_TYPE_TUNNEL,
#else
        LOOPBACK = 772,
        ETHERNET = 1,
        WIRELESS = 65535,
        PPP = 512,
        ATM = 19,
        TUNNEL = 768,
#endif
        UNKNOWN = 65535
    };

    class InterfaceAddress final {
        Option<std::string> _address;
        AddressFamily _family;
        RoutingScheme _routing_scheme;

        public:
        friend struct std::hash<InterfaceAddress>;
        friend struct NetworkInterface;

        inline InterfaceAddress(Option<std::string> address, AddressFamily family,
                                RoutingScheme routing_scheme) noexcept :
                _address {std::move(address)},
                _family {family},
                _routing_scheme {routing_scheme} {
        }

        KSTD_DEFAULT_MOVE_COPY(InterfaceAddress, InterfaceAddress, inline)
        ~InterfaceAddress() noexcept = default;

        [[nodiscard]] inline auto operator==(const InterfaceAddress& other) const noexcept -> bool {
            return _address == other._address && _family == other._family && _routing_scheme == other._routing_scheme;
        }

        [[nodiscard]] inline auto operator!=(const InterfaceAddress& other) const noexcept -> bool {
            return !(*this == other);
        }

        [[nodiscard]] inline auto get_address() const noexcept -> const Option<std::string>& {
            return _address;
        }

        [[nodiscard]] inline auto get_family() const noexcept -> AddressFamily {
            return _family;
        }

        [[nodiscard]] inline auto get_routing_scheme() const noexcept -> RoutingScheme {
            return _routing_scheme;
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

KSTD_DEFAULT_HASH((kstd::platform::InterfaceAddress), value._address, value._family, value._routing_scheme);

namespace kstd::platform {
    class NetworkInterface final {
        std::string _name;
        std::string _description;
        std::unordered_set<InterfaceAddress> _addresses;
        Option<usize> _link_speed;
        InterfaceType _type;
        usize _max_transfer;

        public:
        friend struct std::hash<NetworkInterface>;

        inline NetworkInterface(std::string name, std::string description,
                                std::unordered_set<InterfaceAddress> addresses, Option<usize> link_speed,
                                InterfaceType type, usize max_transfer) noexcept :
                _name {std::move(name)},
                _description {std::move(description)},
                _addresses {std::move(addresses)},
                _link_speed {link_speed},
                _type {type},
                _max_transfer {max_transfer} {
        }

        KSTD_DEFAULT_MOVE_COPY(NetworkInterface, NetworkInterface, inline)
        ~NetworkInterface() noexcept = default;

        [[nodiscard]] inline auto operator==(const NetworkInterface& other) const noexcept -> bool {
            return _name == other._name && _description == other._description && _addresses == other._addresses &&
                   _link_speed == other._link_speed && _type == other._type;
        }

        [[nodiscard]] inline auto operator!=(const NetworkInterface& other) const noexcept -> bool {
            return !(*this == other);
        }

        [[nodiscard]] inline auto has_addresses_by_family(const AddressFamily family) const noexcept -> bool {
            // clang-format off
            return streams::stream(_addresses).map(KSTD_FIELD_FUNCTOR(_family)).find_first([&](auto value) {
                return value == family;
            }).has_value();
            // clang-format on
        }

        [[nodiscard]] inline auto has_addresses_with_routing_scheme(const RoutingScheme scheme) const noexcept -> bool {
            // clang-format off
            return streams::stream(_addresses).map(KSTD_FIELD_FUNCTOR(_routing_scheme)).find_first([&](auto value) {
                return value == scheme;
            }).has_value();
            // clang-format on
        }

        inline auto insert_address(const InterfaceAddress address) noexcept -> void {
            _addresses.insert(address);
        }

        [[nodiscard]] auto get_name() const noexcept -> const std::string& {
            return _name;
        }

        [[nodiscard]] auto get_description() const noexcept -> const std::string& {
            return _description;
        }

        [[nodiscard]] auto get_addresses() const noexcept -> const std::unordered_set<InterfaceAddress>& {
            return _addresses;
        }

        [[nodiscard]] auto get_link_speed() const noexcept -> Option<usize> {
            return _link_speed;
        }

        [[nodiscard]] auto get_type() const noexcept -> InterfaceType {
            return _type;
        }

        [[nodiscard]] auto get_max_transfer() const noexcept -> usize {
            return _max_transfer;
        }
    };

    [[nodiscard]] auto enumerate_interfaces() noexcept -> Result<std::unordered_set<NetworkInterface>>;
}// namespace kstd::platform

KSTD_HASH((kstd::platform::NetworkInterface), [&] {
    auto result = kstd::hash(value._name, value._description, value._link_speed, value._type);
    kstd::combined_hash_into(result, kstd::hash_range(value._addresses.cbegin(), value._addresses.cend()));
    return result;
}())
