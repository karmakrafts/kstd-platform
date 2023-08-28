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

#include <kstd/bitflags.hpp>
#include <kstd/macros.hpp>
#include <kstd/result.hpp>
#include <unordered_set>
#include <vector>

#ifndef PLATFORM_WINDOWS
#include <libnl3/netlink/attr.h>
#include <libnl3/netlink/errno.h>
#include <libnl3/netlink/genl/ctrl.h>
#include <libnl3/netlink/genl/genl.h>
#include <libnl3/netlink/handlers.h>
#include <libnl3/netlink/msg.h>
#include <libnl3/netlink/netlink.h>
#include <libnl3/netlink/socket.h>
#include <linux/nl80211.h>
#endif

#include "kstd/platform/network.hpp"
#undef interface

namespace kstd::platform {
#ifdef PLATFORM_LINUX
    namespace {
        namespace nl {
            KSTD_DEFAULT_DELETER(MessageDeleter, nlmsg_free)
            KSTD_DEFAULT_DELETER(CallbackDeleter, nl_cb_put)
        }
    }// namespace ::nl
#endif

    KSTD_BITFLAGS(u8, AuthAlgorithm, WPA = 0b0001, WPA2 = 0b0010, WPA3 = 0b0100, SHARED_KEY = 0b1000)// NOLINT
    KSTD_BITFLAGS(u8, CipherAlgorithm, WEP40 = 0b0001, TKIP = 0b0010, CCMP = 0b0100, WEP104 = 0b1000)   // NOLINT

    class WifiBand {
        std::string _mac_address;
        usize _frequency;
        usize _signal_strength;
        bool _signal_strength_unspec;

        public:
        friend struct std::hash<WifiBand>;

        inline WifiBand(const std::string mac_address, const usize frequency, const usize signal_strength,// NOLINT
                        const bool signal_strength_unspec) :
                _mac_address {std::move(mac_address)},
                _frequency {frequency},
                _signal_strength {signal_strength},
                _signal_strength_unspec {signal_strength_unspec} {
        }

        KSTD_DEFAULT_MOVE_COPY(WifiBand, WifiBand, inline)
        ~WifiBand() noexcept = default;

        /**
         * This function returns the MAC address of the band as string reference.
         *
         * @return The band's MAC address
         */
        [[nodiscard]] inline auto get_mac_address() const noexcept -> const std::string& {
            return _mac_address;
        }

        /**
         * This function returns the frequency of the band as usize.
         *
         * @return The network frequency.
         */
        [[nodiscard]] inline auto get_frequency() const noexcept -> usize {
            return _frequency;
        }

        /**
         * This function returns the strength of the signal as usize. If {@see is_signal_strength_unit_unspecified}
         * returns true, the strength is specified in an unknown unit otherwise the signal is specified in dBm.
         *
         * @return The signal strength as numerical (usize)
         */
        [[nodiscard]] inline auto get_signal_strength() const noexcept -> usize {
            return _signal_strength;
        }

        /**
         * This function defines if the signal strength of the band is specified with the unit dBm. If this function
         * returns true, the unit is dBm. If this function returns false, the unit is unknown.
         *
         * @return Whether the unit of signal strength is specified
         */
        [[nodiscard]] inline auto is_signal_strength_unit_unspecified() const noexcept -> bool {
            return _signal_strength_unspec;
        }

        [[nodiscard]] inline auto operator==(const WifiBand& other) const noexcept -> bool {
            return _mac_address == other._mac_address && _frequency == other._frequency &&
                   _signal_strength == other._signal_strength &&
                   _signal_strength_unspec == other._signal_strength_unspec;
        }

        [[nodiscard]] inline auto operator!=(const WifiBand& other) const noexcept -> bool {
            return !(*this == other);
        }
    };

    class WifiNetwork final {
        Option<std::string> _ssid;
        std::vector<WifiBand> _bands;

        public:
        friend struct std::hash<WifiNetwork>;
        friend auto enumerate_wlan_networks(const NetworkInterface& interface) noexcept
                -> Result<std::unordered_set<WifiNetwork>>;

        inline WifiNetwork(const Option<std::string> ssid, const std::vector<WifiBand> bands) noexcept :
                _ssid {std::move(ssid)},
                _bands {std::move(bands)} {
        }

        KSTD_DEFAULT_MOVE_COPY(WifiNetwork, WifiNetwork, inline)
        ~WifiNetwork() noexcept = default;

        /**
         * This function returns the SSID of the network, if there is a SSID. The SSID is returned as Option.
         *
         * @return The network's SSID
         */
        [[nodiscard]] inline auto get_ssid() const noexcept -> const Option<std::string>& {
            return _ssid;
        }

        /**
         * This function returns a reference of a vector with all received network bands.
         *
         * @return The network's bands
         */
        [[nodiscard]] inline auto get_bands() const noexcept -> const std::vector<WifiBand>& {
            return _bands;
        }

        [[nodiscard]] inline auto operator==(const WifiNetwork& other) const noexcept -> bool {
            return _ssid == other._ssid && _bands == other._bands;
        }

        [[nodiscard]] inline auto operator!=(const WifiNetwork& other) const noexcept -> bool {
            return !(*this == other);
        }
    };

    [[nodiscard]] inline auto get_auth_algorithm_names(const AuthAlgorithm algorithm) noexcept
            -> std::vector<std::string> {
        std::vector<std::string> names {};
        if((algorithm & AuthAlgorithm::SHARED_KEY) == AuthAlgorithm::SHARED_KEY) {
            names.emplace_back("Shared Key");
        }

        if((algorithm & AuthAlgorithm::WPA) == AuthAlgorithm::WPA) {
            names.emplace_back("WPA");
        }

        if((algorithm & AuthAlgorithm::WPA2) == AuthAlgorithm::WPA2) {
            names.emplace_back("WPA2");
        }

        if((algorithm & AuthAlgorithm::WPA3) == AuthAlgorithm::WPA3) {
            names.emplace_back("WPA3");
        }
        return names;
    }

    [[nodiscard]] inline auto get_cipher_algorithm_names(const CipherAlgorithm algorithm) noexcept
            -> std::vector<std::string> {
        std::vector<std::string> names {};
        if((algorithm & CipherAlgorithm::TKIP) == CipherAlgorithm::TKIP) {
            names.emplace_back("TKIP");
        }

        if((algorithm & CipherAlgorithm::CCMP) == CipherAlgorithm::CCMP) {
            names.emplace_back("CCMP");
        }

        if((algorithm & CipherAlgorithm::WEP40) == CipherAlgorithm::WEP40) {
            names.emplace_back("WEP40");
        }

        if((algorithm & CipherAlgorithm::WEP104) == CipherAlgorithm::WEP104) {
            names.emplace_back("WEP104");
        }
        return names;
    }

    /**
     * This function initializes a scan on the network interface, if the interface is wireless. After the scan, all
     * results are collected into the set of networks.
     *
     * @param interface The interface, with that should be scanned
     * @return          All found WiFi networks in a set
     */
    [[nodiscard]] auto enumerate_wlan_networks(const NetworkInterface& interface) noexcept
            -> Result<std::unordered_set<WifiNetwork>>;
}// namespace kstd::platform

KSTD_DEFAULT_HASH((kstd::platform::WifiBand), value._frequency, value._signal_strength_unspec, value._signal_strength)
KSTD_HASH((kstd::platform::WifiNetwork), [&]() {
    auto result = kstd::hash(value._ssid);
    kstd::combined_hash_into(result, kstd::hash_range(value._bands.cbegin(), value._bands.cend()));
    return result;
}())