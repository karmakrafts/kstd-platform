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
#include <kstd/result.hpp>
#include <unordered_set>
#include <kstd/macros.hpp>

#ifdef PLATFORM_WINDOWS
#include <wlanapi.h>
#else
#include <linux/nl80211.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>

// Undefine __USE_MISC to prevent macro redefinition
#undef __USE_MISC
#include <net/if.h>
#endif

#include "kstd/platform/network.hpp"

namespace kstd::platform {
#ifdef PLATFORM_LINUX
    namespace {
        namespace nl {
            KSTD_DEFAULT_DELETER(SocketDeleter, nl_socket_free)
            KSTD_DEFAULT_DELETER(MessageDeleter, nlmsg_free)
            KSTD_DEFAULT_DELETER(CallbackDeleter, nl_cb_put)
        }
    }
#endif

    class WlanNetwork final {
        std::string _ssid;

        public:
        friend struct std::hash<WlanNetwork>;

        inline WlanNetwork(const std::string ssid) noexcept :
                _ssid {std::move(ssid)} {
        }

        KSTD_DEFAULT_MOVE_COPY(WlanNetwork, WlanNetwork, inline)
        ~WlanNetwork() noexcept = default;

        [[nodiscard]] inline auto get_ssid() const noexcept -> const std::string& {
            return _ssid;
        }
    };

    // clang-format off
    [[nodiscard]] auto enumerate_wlan_networks(const NetworkInterface& interface) noexcept -> Result<std::unordered_set<WlanNetwork>>;
    // clang-format on
}// namespace kstd::platform

KSTD_DEFAULT_HASH((kstd::platform::WlanNetwork), value._ssid)