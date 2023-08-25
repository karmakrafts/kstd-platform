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
    namespace { namespace nl {
        KSTD_DEFAULT_DELETER(SocketDeleter, nl_socket_free)
        KSTD_DEFAULT_DELETER(MessageDeleter, nlmsg_free)
        KSTD_DEFAULT_DELETER(CallbackDeleter, nl_cb_put)
    }}// namespace ::nl
#endif

    class WifiBand {
        std::string _mac_address;
        usize _frequency;
        usize _signal_strength;
        bool _signal_strength_unspec;

        public:
        friend struct std::hash<WifiBand>;

        inline WifiBand(const std::string mac_address, const usize frequency, const usize signal_strength,
                        const bool signal_strength_unspec) :// NOLINT
                _mac_address {std::move(mac_address)},
                _frequency {frequency},
                _signal_strength {signal_strength},
                _signal_strength_unspec {signal_strength_unspec} {
        }

        KSTD_DEFAULT_MOVE_COPY(WifiBand, WifiBand, inline)
        ~WifiBand() noexcept = default;

        [[nodiscard]] inline auto get_mac_address() const noexcept -> const std::string& {
            return _mac_address;
        }

        [[nodiscard]] inline auto get_frequency() const noexcept -> usize {
            return _frequency;
        }

        [[nodiscard]] inline auto get_signal_strength() const noexcept -> usize {
            return _signal_strength;
        }

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

        [[nodiscard]] inline auto get_ssid() const noexcept -> const Option<std::string>& {
            return _ssid;
        }

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

    // clang-format off
    [[nodiscard]] auto enumerate_wlan_networks(const NetworkInterface& interface) noexcept -> Result<std::unordered_set<WifiNetwork>>;
    // clang-format on
}// namespace kstd::platform

KSTD_DEFAULT_HASH((kstd::platform::WifiBand), value._frequency, value._signal_strength_unspec, value._signal_strength)
KSTD_HASH((kstd::platform::WifiNetwork), [&]() {
    auto result = kstd::hash(value._ssid);
    kstd::combined_hash_into(result, kstd::hash_range(value._bands.cbegin(), value._bands.cend()));
    return result;
}())