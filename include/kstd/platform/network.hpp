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
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#endif

namespace kstd::platform {

    enum class AddressFamily : u8 {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX,
        IPX = AF_IPX,
        APPLE_TALK = AF_APPLETALK,
#ifdef PLATFORM_WINDOWS
        MAC = AF_LINK
#else
        MAC = AF_PACKET
#endif
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

        /**
         * This constructor constructs the address structure with all needed information. These are the literal address
         * as string, the address family and the routing scheme.
         *
         * @param address        The literal address
         * @param family         The address family
         * @param routing_scheme The routing schem
         */
        inline InterfaceAddress(Option<std::string> address, AddressFamily family,
                                RoutingScheme routing_scheme) noexcept :
                _address {std::move(address)},
                _family {family},
                _routing_scheme {routing_scheme} {
        }

        KSTD_DEFAULT_MOVE_COPY(InterfaceAddress, InterfaceAddress, inline)
        ~InterfaceAddress() noexcept = default;

        /**
         * This function returns the literal address of the address structure.
         *
         * @return The literal address as optional string
         */
        [[nodiscard]] inline auto get_address() const noexcept -> const Option<std::string>& {
            return _address;
        }

        /**
         * This function returns the address family of this address structure.
         *
         * @return The address family
         */
        [[nodiscard]] inline auto get_family() const noexcept -> AddressFamily {
            return _family;
        }

        /**
         * This function returns the routing schema of this address structure. This can be Unicast, Multicast oder
         * Anycast.
         *
         * @return The routing schema of the address
         */
        [[nodiscard]] inline auto get_routing_scheme() const noexcept -> RoutingScheme {
            return _routing_scheme;
        }

        [[nodiscard]] inline auto operator==(const InterfaceAddress& other) const noexcept -> bool {
            return _address == other._address && _family == other._family && _routing_scheme == other._routing_scheme;
        }

        [[nodiscard]] inline auto operator!=(const InterfaceAddress& other) const noexcept -> bool {
            return !(*this == other);
        }
    };

    /**
     * This function returns the literal name of the interface type by the specified interface type enum value. This
     * function supports Loopback, Ethernet, Wireless, Tunnel, PPP and ATM. Any other value will be returned as Unknown.
     *
     * @param type The specified type of some interface
     * @return     The literal name of the interface type
     */
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

    /**
     * This function returns the literal name of the address family by the specified address family enum value. This
     * function supports IPv4, IPv6, UNIX, IPX, MAC and Apple Talk. Any other value will be returned as Unknown.
     *
     * @param family The specified address family
     * @return       The literal name of the address family
     */
    [[nodiscard]] inline auto get_address_family_name(const AddressFamily family) noexcept -> std::string {
        switch(family) {
            case AddressFamily::IPv4: return "IPv4";
            case AddressFamily::IPv6: return "IPv6";
            case AddressFamily::UNIX: return "UNIX";
            case AddressFamily::IPX: return "IPX";
            case AddressFamily::MAC: return "MAC";
            case AddressFamily::APPLE_TALK: return "AppleTalk";
            default: return "Unknown";
        }
    }

    /**
     * This function returns the literal name of the routing scheme by the specified routing scheme enum value. This
     * function supports Unicast, Multicast and Anycast. Any other value will be returned as Unknown.
     *
     * @param scheme The specified routing scheme
     * @return       The literal name of the routing scheme
     * @author       Cedric Hammes
     * @since        17/08/2023
     */
    [[nodiscard]] inline auto get_routing_scheme_name(const RoutingScheme scheme) noexcept -> std::string {
        switch(scheme) {
            case RoutingScheme::UNICAST: return "Unicast";
            case RoutingScheme::MULTICAST: return "Multicast";
            case RoutingScheme::ANYCAST: return "Anycast";
            default: return "Unknown";
        }
    }
}// namespace kstd::platform

KSTD_DEFAULT_HASH((kstd::platform::InterfaceAddress), value._address, value._family, value._routing_scheme)

namespace kstd::platform {
    /**
     * This class represents all information about interfaces with this API. This interface structure can give you the
     * following values:
     * - Name: The name of the interface. Mostly the path to the interface on the (file)system
     * - Description: The description of the interface. Mostly the user-friendly name of the interface
     * - Addresses: A list of all addresses, which are assigned to this interface (MAC address included)
     * - Link Speed: The optional speed of the link. "Not set" if the interface is virtual
     * - Type: The type of the interface like Loopback, Ethernet etc.
     * - MTU: The maximum transmission unit (MTU) of the interface adapter.
     */
    class NetworkInterface final {
        std::string _name;
        std::string _description;
        std::unordered_set<InterfaceAddress> _addresses;
        Option<usize> _link_speed;
        InterfaceType _type;
        usize _mtu;

        public:
        friend struct std::hash<NetworkInterface>;
        friend auto enumerate_interfaces() noexcept -> Result<std::unordered_set<NetworkInterface>>;

        /**
         * This constructor constructs the interface with all needed values. This constructor is used by default in the
         * enumerate_interfaces function.
         *
         * @param name        The name of the interface (Mostly the path to the interface on the system)
         * @param description The description of the interface (The user-friendly name)
         * @param addresses   The list of addresses of the interface
         * @param link_speed  The optional speed of the interface adapter
         * @param type        The type of the interface
         * @param _mtu        The MTU (Maximum Transmission Unit) of the interface
         *
         * @author             Cedric Hammes
         * @since              18/08/2023
         */
        inline NetworkInterface(std::string name, std::string description,
                                std::unordered_set<InterfaceAddress> addresses, Option<usize> link_speed,
                                InterfaceType type, usize mtu) noexcept :
                _name {std::move(name)},
                _description {std::move(description)},
                _addresses {std::move(addresses)},
                _link_speed {link_speed},
                _type {type},
                _mtu {mtu} {
        }

        KSTD_DEFAULT_MOVE_COPY(NetworkInterface, NetworkInterface, inline)
        ~NetworkInterface() noexcept = default;

#ifdef PLATFORM_LINUX
        inline auto insert_address(const InterfaceAddress address) noexcept -> void {
            _addresses.insert(address);
        }
#endif

        /**
         * This function enumerates all collected addresses of the interface, selects the first MAC address found and
         * returns the literal address as string reference.
         *
         * @return The MAC address of the adapter for this interface.
         */
        [[nodiscard]] inline auto get_mac_address() const noexcept -> const std::string& {
            // clang-format off
            return streams::stream(_addresses).find_first([](auto& address) {
                return address._family == AddressFamily::MAC;
            })->_address.get();
            // clang-format on
        }

        /**
         * This function enumerates all collected addresses of the interface an checks if there is some address with the
         * specified family.
         *
         * @param family The address family for that should run this lookup
         * @return       Is there one or more addresses with the specified family
         */
        [[nodiscard]] [[maybe_unused]] inline auto has_addresses_by_family(const AddressFamily family) const noexcept
                -> bool {
            // clang-format off
            return streams::stream(_addresses).map(KSTD_FIELD_FUNCTOR(_family)).find_first([&](auto& value) {
                return value == family;
            }).has_value();
            // clang-format on
        }

        /**
         * This function enumerates all collected addresses of the interface an checks if there is some address with the
         * specified routing scheme.
         *
         * @param scheme The routing scheme for that should run this lookup
         * @return       Is there one or more addresses with the specified routing scheme
         */
        [[nodiscard]] [[maybe_unused]] inline auto
        has_addresses_with_routing_scheme(const RoutingScheme scheme) const noexcept -> bool {
            // clang-format off
            return streams::stream(_addresses).map(KSTD_FIELD_FUNCTOR(_routing_scheme)).find_first([&](auto& value) {
                return value == scheme;
            }).has_value();
            // clang-format on
        }

        /**
         * This function returns the name of the interface. The name is in every implementation the path to the
         * interface/adapter.
         *
         * @return The name/path of the interface/adapter
         */
        [[nodiscard]] auto get_name() const noexcept -> const std::string& {
            return _name;
        }

        /**
         * This function returns the description of the interface. The description is the user-friendly name of the
         * interface/adapter.
         *
         * @return The description/friendly name of the interface/adapter
         */
        [[nodiscard]] auto get_description() const noexcept -> const std::string& {
            return _description;
        }

        /**
         * This function returns a reference to the const set of all addresses in the interface. Every address structure
         * contains the address family, routing scheme and the address itself.
         *
         * @return A reference to the address set
         */
        [[nodiscard]] auto get_addresses() const noexcept -> const std::unordered_set<InterfaceAddress>& {
            return _addresses;
        }

        /**
         * This function returns an optional of the link speed. The virtual interfaces doesn't have a link speed, so
         * there can be some interfaces without a link speed.
         *
         * @return A option for the link speed
         */
        [[nodiscard]] auto get_link_speed() const noexcept -> const Option<usize>& {
            return _link_speed;
        }

        /**
         * This function returns the type of the interface.
         *
         * @return The interface type
         */
        [[nodiscard]] auto get_type() const noexcept -> InterfaceType {
            return _type;
        }

        /**
         * This function returns the maximum transmission unit (MTU) for packets in bytes of this interface/adapter. The
         * default MTU is mostly 1500 bytes.
         *
         * @return The MTU of the interface
         */
        [[nodiscard]] auto get_mtu() const noexcept -> usize {
            return _mtu;
        }

        [[nodiscard]] inline auto operator==(const NetworkInterface& other) const noexcept -> bool {
            return _name == other._name && _description == other._description && _addresses == other._addresses &&
                   _link_speed == other._link_speed && _type == other._type;
        }

        [[nodiscard]] inline auto operator!=(const NetworkInterface& other) const noexcept -> bool {
            return !(*this == other);
        }
    };

    /**
     * This function uses internally different APIs to enumerate all interfaces and read most of their values and give
     * the user a convenient way to interact with the interfaces. This function should not be called synchronously too
     * often, since collecting and collating the information can create an increased load, depending on the number of
     * interfaces and system, which is avoidable.
     *
     * Possible Errors:
     * - Errors while trying to determine the size of the information for the allocation
     * - Errors while allocating data for the information
     * - System/API errors while calling some API functions
     *
     * @return A collection of all interfaces or an error
     */
    [[nodiscard]] auto enumerate_interfaces() noexcept -> Result<std::unordered_set<NetworkInterface>>;
}// namespace kstd::platform

KSTD_HASH((kstd::platform::NetworkInterface), [&] {
    auto result = kstd::hash(value._name, value._description, value._link_speed, value._type);
    kstd::combined_hash_into(result, kstd::hash_range(value._addresses.cbegin(), value._addresses.cend()));
    return result;
}())
