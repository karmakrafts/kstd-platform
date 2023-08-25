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

#include "kstd/platform/wireless.hpp"
#include "kstd/platform/platform.hpp"

namespace kstd::platform {
    auto enumerate_wlan_networks(const NetworkInterface& interface) noexcept
            -> Result<std::unordered_set<WlanNetwork>> {
        using namespace std::string_literals;

        // Cancel the function if the type of the interface is not wireless
        if(interface.get_type() != InterfaceType::WIRELESS) {
            return Error {fmt::format("The interface type is not wireless ({})",
                                      get_interface_type_name(interface.get_type()))};
        }

        // Allocate socket
        auto socket = std::unique_ptr<nl_sock, nl::SocketDeleter<nl_sock>> {nl_socket_alloc()};
        nl_socket_set_buffer_size(socket.get(), 8192, 8192);

        // Connect to nl80211 control channel
        if(genl_connect(socket.get()) != 0) {
            return Error {"Unable to enumerate Wi-Fi networks: Unable to connect to nl80211"s};
        }

        // Receive id of nl80211 driver by socket
        const int nl80211_driver_id = genl_ctrl_resolve(socket.get(), "nl80211");
        if(nl80211_driver_id < 0) {
            return Error {"Unable to enumerate Wi-Fi networks: No nl80211 found"s};
        }

        // Generate SSIDs to scan
        auto ssids_to_scan = std::unique_ptr<nl_msg, nl::MessageDeleter<nl_msg>> {nlmsg_alloc()};
        if(ssids_to_scan == nullptr) {
            return Error {"Unable to enumerate Wi-Fi networks: Not enough memory to allocate message"s};
        }

        // Generate message for Wi-Fi network scan
        auto scan_message = std::unique_ptr<nl_msg, nl::MessageDeleter<nl_msg>> {nlmsg_alloc()};
        if(scan_message == nullptr) {
            return Error {"Unable to enumerate Wi-Fi networks: Not enough memory to allocate message"s};
        }

        genlmsg_put(scan_message.get(), 0, 0, nl80211_driver_id, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0);
        nla_put_u32(scan_message.get(), NL80211_ATTR_IFINDEX, if_nametoindex(interface.get_name().c_str()));
        nla_put(ssids_to_scan.get(), 1, 0, "");
        nla_put_nested(scan_message.get(), NL80211_ATTR_SCAN_SSIDS, ssids_to_scan.get());

        // Allocate callback and configure it
        auto callback = std::unique_ptr<nl_cb, nl::CallbackDeleter<nl_cb>> {nl_cb_alloc(NL_CB_DEFAULT)};
        // https://github.com/Robpol86/libnl/blob/master/example_c/scan_access_points.c#L233

        return {{}};
    }
}// namespace kstd::platform

#endif