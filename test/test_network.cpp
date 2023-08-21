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

#include "kstd/platform/network.hpp"
#include <gtest/gtest.h>
#include <iostream>

// Thanks microsoft for these definitions
#undef interface

TEST(kstd_platform_Network, test_enumerate_interfaces) {
    using namespace kstd::platform;
    
    const auto result = enumerate_interfaces(InterfaceInfoFlags::WIRELESS);
    ASSERT_NO_THROW(result.throw_if_error());// NOLINT

    for(const auto& interface : *result) {
        std::cout << interface.get_description() << '\n';
        std::cout << " - Path: " << interface.get_name() << '\n';
        std::cout << " - MTU: " << interface.get_mtu() << '\n';
        std::cout << " - MAC Address: " << interface.get_mac_address() << '\n';
        std::cout << " - Type: " << get_interface_type_name(interface.get_type()) << '\n';
        if(interface.get_link_speed().has_value()) {
            std::cout << " - Speed: " << *interface.get_link_speed() << '\n';
        }

        const auto wireless_information = interface.get_wireless_information();
        if (wireless_information.has_value()) {
            std::cout << " - Wireless information:\n";
            std::cout << "   - WLAN Network Name: " << wireless_information->get_network_name() << '\n';
        }

        if(!interface.get_addresses().empty()) {
            std::cout << " - Addresses:\n";
        }

        for(const auto& address : interface.get_addresses()) {
            std::cout << "   - ";
            if(address.get_address().has_value()) {
                std::cout << *address.get_address() << ' ';
            }
            std::cout << "(" << get_address_family_name(address.get_family()) << '/'
                      << get_routing_scheme_name(address.get_routing_scheme()) << ")\n";
        }
    }
}
