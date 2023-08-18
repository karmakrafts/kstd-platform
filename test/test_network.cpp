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
    const auto result = kstd::platform::enumerate_interfaces();
    ASSERT_NO_THROW(result.throw_if_error());// NOLINT

    for(const auto& interface : *result) {
        std::cout << interface.name << " - " << interface.description;
        if(interface.link_speed.has_value()) {
            std::cout << " (" << *interface.link_speed << ')';
        }

        std::cout << " (" << kstd::platform::get_interface_type_name(interface.type) << ")\n";
        for(const auto& address : interface.addresses) {
            std::cout << " - ";
            if (address.address.has_value()) {
                std::cout << *address.address << ' ';
            }
            std::cout << "(" << kstd::platform::get_address_family_name(address.family)
                      << '/' << kstd::platform::get_routing_scheme_name(address.routing_scheme) << ")\n";
        }
    }
}
