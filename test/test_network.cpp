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

    for (const auto& interface : result.get()) {
        std::cout << interface.name << " - " << interface.description << '\n';
        for (const auto family : interface.address_families) {
            switch (family) {
                case kstd::platform::AddressFamily::IPv4: std::cout << " - IPv4\n"; break;
                case kstd::platform::AddressFamily::IPv6: std::cout << " - IPv6\n"; break;
                case kstd::platform::AddressFamily::UNIX: std::cout << " - UNIX\n"; break;
                case kstd::platform::AddressFamily::IPX: std::cout << " - IPX\n"; break;
                case kstd::platform::AddressFamily::APPLE_TALK: std::cout << " - Apple Talk\n"; break;
            }
        }
    }
}
