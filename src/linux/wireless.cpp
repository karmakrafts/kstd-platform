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
#include <iostream>

namespace kstd::platform {
    struct ScanResult {
        bool done;
        bool aborted;
    };

    // NOLINTBEGIN
    auto compute_ssid(unsigned char* ie, int ielen) noexcept -> std::string {
        uint8_t len;
        uint8_t* data;
        std::string result {};

        while(ielen >= 2 && ielen >= ie[1]) {
            if(ie[0] == 0 && ie[1] >= 0 && ie[1] <= 32) {
                len = ie[1];
                data = reinterpret_cast<uint8_t*>(ie + 2);
                for(usize i = 0; i < len; ++i) {
                    if(isprint(data[i]) && data[i] != ' ' && data[i] != '\\') {
                        result += data[i];
                    }
                    else if(data[i] == ' ' && (i != 0 && i != len - 1)) {
                        result += ' ';
                    }
                    else {
                        result += fmt::format("{0:x}", data[i]);
                    }
                }
                break;
            }
            ielen -= ie[1] + 2;
            ie += ie[1] + 2;
        }

        return result;
    }
    // NOLINTEND

    inline auto parse_mac_addr(unsigned char* mac_addr) -> std::string {
        std::string mac_addr_string {};
        for(int i = 0; i < 6; ++i) {
            if(i == 0) {
                mac_addr_string.append(fmt::format("{:02X}", mac_addr[i]));// NOLINT
            }
            else {
                mac_addr_string.append(fmt::format(":{:02X}", mac_addr[i]));// NOLINT
            }
        }
        return mac_addr_string;
    }

    int error_handler([[maybe_unused]] sockaddr_nl* nl_addr, nlmsgerr* error, void* arg) {
        *static_cast<int*>(arg) = error->error;
        return NL_STOP;
    }

    int finish_handler([[maybe_unused]] nl_msg* message, void* arg) {
        *static_cast<int*>(arg) = 0;
        return NL_SKIP;
    }

    int ack_handler([[maybe_unused]] nl_msg* message, void* arg) {
        *static_cast<int*>(arg) = 1;
        return NL_STOP;
    }

    int no_seq_check([[maybe_unused]] nl_msg* message, [[maybe_unused]] void* arg) {
        return NL_OK;
    }

    int callback_handler(nl_msg* message, void* arg) {
        const auto* message_header = static_cast<genlmsghdr*>(nlmsg_data(nlmsg_hdr(message)));
        auto* result = static_cast<ScanResult*>(arg);

        if(message_header->cmd == NL80211_CMD_SCAN_ABORTED) {
            result->done = true;
            result->aborted = true;
        }
        else if(message_header->cmd == NL80211_CMD_NEW_SCAN_RESULTS) {
            result->done = true;
            result->aborted = false;
        }

        return NL_SKIP;
    }

    int dump_callback(nl_msg* message, void* arg) {
        const auto* message_header = static_cast<genlmsghdr*>(nlmsg_data(nlmsg_hdr(message)));
        std::array<nla_policy, NL80211_BSS_MAX + 1> bss_policies {};
        bss_policies[NL80211_BSS_TSF] = {.type = NLA_U64};
        bss_policies[NL80211_BSS_FREQUENCY] = {.type = NLA_U32}, bss_policies[NL80211_BSS_BSSID] = {};
        bss_policies[NL80211_BSS_BEACON_INTERVAL] = {.type = NLA_U16};
        bss_policies[NL80211_BSS_CAPABILITY] = {.type = NLA_U16};
        bss_policies[NL80211_BSS_INFORMATION_ELEMENTS] = {};
        bss_policies[NL80211_BSS_SIGNAL_MBM] = {.type = NLA_U32};
        bss_policies[NL80211_BSS_SIGNAL_UNSPEC] = {.type = NLA_U8};
        bss_policies[NL80211_BSS_STATUS] = {.type = NLA_U32};
        bss_policies[NL80211_BSS_SEEN_MS_AGO] = {.type = NLA_U32};
        bss_policies[NL80211_BSS_BEACON_IES] = {};

        // Parse attribute indicies and BSS with error checks. If an error occurs, the network will be skipped
        std::array<nlattr*, NL80211_ATTR_MAX + 1> netlink_attribute_indicies {};
        int err = nla_parse(netlink_attribute_indicies.data(), NL80211_ATTR_MAX, genlmsg_attrdata(message_header, 0),
                            genlmsg_attrlen(message_header, 0), nullptr);
        if(err < 0) {
            return NL_SKIP;
        }

        if(netlink_attribute_indicies[NL80211_ATTR_BSS] == nullptr) {
            return NL_SKIP;
        }

        std::array<nlattr*, NL80211_ATTR_MAX + 1> bss {};
        err = nla_parse_nested(bss.data(), NL80211_BSS_MAX, netlink_attribute_indicies[NL80211_ATTR_BSS],
                               bss_policies.data());
        if(err < 0) {
            return NL_SKIP;
        }

        // BSSID or IE is missing, we can't parse
        if(bss[NL80211_BSS_BSSID] == nullptr || bss[NL80211_BSS_INFORMATION_ELEMENTS] == nullptr) {
            return NL_SKIP;
        }

        // Get network SSID
        Option<std::string> ssid = compute_ssid(static_cast<u_char*>(nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS])),
                                                nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]));
        if(ssid->empty()) {
            ssid = {};
        }

        // Get signal strength
        bool signal_strength_unspec = false;
        u8 signal_strength = 0;
        if(bss[NL80211_BSS_SIGNAL_MBM] != nullptr) {
            signal_strength = nla_get_u8(bss[NL80211_BSS_SIGNAL_MBM]);
            signal_strength_unspec = false;
        }
        else if(bss[NL80211_BSS_SIGNAL_UNSPEC] != nullptr) {
            signal_strength = nla_get_u8(bss[NL80211_BSS_SIGNAL_UNSPEC]);
            signal_strength_unspec = true;
        }

        auto* interfaces = static_cast<std::unordered_set<WifiNetwork>*>(arg);
        interfaces->insert(
                WifiNetwork {ssid,
                             {{parse_mac_addr(static_cast<u_char*>(nla_data(bss[NL80211_BSS_BSSID]))),
                               nla_get_u32(bss[NL80211_BSS_FREQUENCY]), signal_strength, signal_strength_unspec}}});
        return NL_SKIP;
    }


    auto enumerate_wlan_networks(const NetworkInterface& interface) noexcept
            -> Result<std::unordered_set<WifiNetwork>> {
        using namespace std::string_literals;

        ScanResult result {.done = false, .aborted = false};

        // Cancel the function if the type of the interface is not wireless
        if(interface.get_type() != InterfaceType::WIRELESS) {
            return Error {fmt::format("The interface type is not wireless ({})",
                                      get_interface_type_name(interface.get_type()))};
        }

        // Allocate socket
        const auto socket = std::unique_ptr<nl_sock, nl::SocketDeleter> {nl_socket_alloc()};

        // Connect to nl80211 control channel
        if(genl_connect(socket.get()) != 0) {
            return Error {"Unable to enumerate Wi-Fi networks: Unable to connect to nl80211"s};
        }

        // Receive family id by socket
        const int family_id = genl_ctrl_resolve(socket.get(), "nl80211");
        if(family_id < 0) {
            return Error {fmt::format("Unable to enumerate Wi-Fi networks: {} (Unable to get family id)",
                                      nl_geterror(family_id))};
        }

        // Set Socket membership
        int scan_group_id;// NOLINT
        if((scan_group_id = genl_ctrl_resolve_grp(socket.get(), "nl80211", "scan")) < 0) {
            return Error {fmt::format("Unable to enumerate Wi-Fi networks: {} (Unable to resolve scan group id)",
                                      nl_geterror(family_id))};
        }

        if(nl_socket_add_membership(socket.get(), scan_group_id) < 0) {
            return Error {fmt::format("Unable to enumerate Wi-Fi networks: {} (Unable to join scan group)",
                                      nl_geterror(family_id))};
        }

        // Generate SSIDs to scan
        const auto ssids_to_scan = std::unique_ptr<nl_msg, nl::MessageDeleter> {nlmsg_alloc()};
        if(ssids_to_scan == nullptr) {
            return Error {"Unable to enumerate Wi-Fi networks: Not enough memory to allocate message"s};
        }

        // Get index of interface
        const auto interface_index = interface.get_index();

        // Generate message for Wi-Fi network scan
        const auto scan_message = std::unique_ptr<nl_msg, nl::MessageDeleter> {nlmsg_alloc()};
        if(scan_message == nullptr) {
            return Error {"Unable to enumerate Wi-Fi networks: Not enough memory to allocate message"s};
        }

        genlmsg_put(scan_message.get(), NL_AUTO_PORT, NL_AUTO_SEQ, family_id, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0);
        nla_put_u32(scan_message.get(), NL80211_ATTR_IFINDEX, interface_index);
        nla_put(ssids_to_scan.get(), 1, 0, "");
        nla_put_nested(scan_message.get(), NL80211_ATTR_SCAN_SSIDS, ssids_to_scan.get());

        // Allocate callback, result and error and configure the callback
        int error = 0;
        result.aborted = true;
        auto callback = std::unique_ptr<nl_cb, nl::CallbackDeleter> {nl_cb_alloc(NL_CB_DEFAULT)};
        if(nl_cb_set(callback.get(), NL_CB_VALID, NL_CB_CUSTOM, callback_handler, &result) < 0) {
            return Error {"Unable to enumerate Wi-Fi networks: Unable to set message callback valid"s};
        }

        // Set error handler
        if(nl_cb_err(callback.get(), NL_CB_CUSTOM, error_handler, &error) < 0) {
            return Error {"Unable to enumerate Wi-Fi networks: Unable to set error handler callback"s};
        }

        // Set finish handler
        if(nl_cb_set(callback.get(), NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &error) < 0) {
            return Error {"Unable to enumerate Wi-Fi networks: Unable to set finish handler callback"s};
        }

        // Set ACK handler
        int got_ack = 0;
        if(nl_cb_set(callback.get(), NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &got_ack) < 0) {
            return Error {"Unable to enumerate Wi-Fi networks: Unable to set ACK handler callback"s};
        }

        if(nl_cb_set(callback.get(), NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, nullptr) < 0) {
            return Error {"Unable to enumerate Wi-Fi networks: Unable to set No Sequence callback"s};
        }

        // Send message to Kernel
        error = 1;
        if(nl_send_auto(socket.get(), scan_message.get()) < 0) {
            return Error {"Unable to enumerate Wi-Fi networks: No bytes are sent to kernel"s};
        }

        int return_code;// NOLINT
        while(got_ack != 1) {
            if((return_code = nl_recvmsgs(socket.get(), callback.get())) < 0) {
                return Error {fmt::format("Unable to enumerate Wi-Fi networks: {} (Unable to receive message)",
                                          nl_geterror(return_code))};
            }
        }

        if(error < 0) {
            return Error {fmt::format("Unable to enumerate Wi-Fi networks: {}", nl_geterror(-error))};
        }

        while(!result.done) {
            nl_recvmsgs(socket.get(), callback.get());
        }

        if(result.aborted) {
            return Error {fmt::format("Unable to enumerate Wi-Fi networks: Kernel aborted scan")};
        }

        // Drop socket membership
        nl_socket_drop_membership(socket.get(), scan_group_id);

        std::unordered_set<WifiNetwork> raw_networks {};
        const auto get_scan_message = std::unique_ptr<nl_msg, nl::MessageDeleter> {nlmsg_alloc()};
        genlmsg_put(get_scan_message.get(), 0, 0, family_id, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);
        nla_put_u32(get_scan_message.get(), NL80211_ATTR_IFINDEX, interface_index);
        nl_socket_modify_cb(socket.get(), NL_CB_VALID, NL_CB_CUSTOM, dump_callback, &raw_networks);
        if(nl_send_auto(socket.get(), get_scan_message.get()) == 0) {
            return Error {"Unable to enumerate Wi-Fi networks: No bytes are sent to kernel"s};
        }

        return_code = nl_recvmsgs_default(socket.get());
        if(return_code < 0) {
            return Error {fmt::format("Unable to enumerate Wi-Fi networks: {}", nl_geterror(-return_code))};
        }

        // Deduplicate network list
        // TODO: Edge-Case => If there is not SSID, the network will deleted from the list of available networks
        std::vector<WifiNetwork> networks {};
        for(const auto& network : raw_networks) {
            auto real_network = streams::stream(networks).find_first([&](auto& value) {
                if(network.get_ssid()) {
                    if(!value.get_ssid()) {
                        return false;
                    }

                    return *network.get_ssid() == *value.get_ssid();
                }
                return false;
            });

            if(real_network) {
                real_network->_bands.push_back(network.get_bands()[0]);
                continue;
            }

            networks.push_back(network);
        }

        return {{networks.cbegin(), networks.cend()}};
    }
}// namespace kstd::platform

#endif