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

#ifdef PLATFORM_WINDOWS
#include <wlanapi.h>
#else
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

namespace kstd::platform {
#ifdef PLATFORM_LINUX
    namespace { namespace nl {
        KSTD_DEFAULT_DELETER(SocketDeleter, nl_socket_free)
        KSTD_DEFAULT_DELETER(MessageDeleter, nlmsg_free)
        KSTD_DEFAULT_DELETER(CallbackDeleter, nl_cb_put)
    }}// namespace ::nl
#endif

    class WlanNetwork final {
        std::string _mac_address;
        Option<std::string> _ssid;
        usize _frequency;
        u8 _signal_strength;
        bool _signal_strength_unspec;

        public:
        friend struct std::hash<WlanNetwork>;

        inline WlanNetwork(const std::string mac_address, const Option<std::string> ssid, const usize frequency,
                           const u8 signal_strength, const bool signal_strength_unspec) noexcept :
                _mac_address {mac_address},
                _ssid {std::move(ssid)},
                _frequency {frequency},
                _signal_strength {signal_strength},
                _signal_strength_unspec {signal_strength_unspec} {
        }

        KSTD_DEFAULT_MOVE_COPY(WlanNetwork, WlanNetwork, inline)
        ~WlanNetwork() noexcept = default;

        [[nodiscard]] inline auto get_mac_address() const noexcept -> const std::string& {
            return _mac_address;
        }

        [[nodiscard]] inline auto get_ssid() const noexcept -> const Option<std::string>& {
            return _ssid;
        }

        [[nodiscard]] inline auto get_frequency() const noexcept -> usize {
            return _frequency;
        }

        [[nodiscard]] inline auto get_signal_strength() const noexcept -> u8 {
            return _signal_strength;
        }

        [[nodiscard]] inline auto is_signal_strength_unit_unspecified() const noexcept -> bool {
            return _signal_strength_unspec;
        }

        [[nodiscard]] inline auto operator==(const WlanNetwork& other) const noexcept -> bool {
            return _ssid == other._ssid;
        }

        [[nodiscard]] inline auto operator!=(const WlanNetwork& other) const noexcept -> bool {
            return !(*this == other);
        }
    };

    // clang-format off
    [[nodiscard]] auto enumerate_wlan_networks(const NetworkInterface& interface) noexcept -> Result<std::unordered_set<WlanNetwork>>;
    // clang-format on
}// namespace kstd::platform

KSTD_DEFAULT_HASH((kstd::platform::WlanNetwork), value._ssid)