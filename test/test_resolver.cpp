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
 * @since 12/08/2023
 */

#include "kstd/platform/dns.hpp"
#include <gtest/gtest.h>
#include <iostream>

TEST(kstd_platform_Resolver, test_resolve_local_addresses) {
    auto resolver = kstd::platform::Resolver {};
    ASSERT_EQ(resolver.resolve("localhost", kstd::platform::RecordType::A).get_or_throw(), "127.0.0.1");
    ASSERT_EQ(resolver.resolve("localhost", kstd::platform::RecordType::AAAA).get_or_throw(), "::1");
}

TEST(kstd_platform_Resolver, test_resolve_remote_addresses) {
    auto resolver = kstd::platform::Resolver {};
    resolver.resolve("karmakrafts.dev", kstd::platform::RecordType::A).throw_if_error();
    resolver.resolve("karmakrafts.dev", kstd::platform::RecordType::AAAA).throw_if_error();
}

TEST(kstd_platform_Resolver, test_resolve_with_invalid_nameservers) {
    auto resolver = kstd::platform::Resolver {{"1.2.3.4", "1.2.3.5", "1.2.3.6"}};
    ASSERT_TRUE(resolver.resolve("karmakrafts.dev", kstd::platform::RecordType::A).is_error());
}

TEST(kstd_platform_Resolver, test_enumerate_nameservers) {
    auto result = kstd::platform::enumerate_nameservers();
    ASSERT_TRUE(result.is_ok());
    for (auto address : result.get()) {
        std::cout << fmt::format("Nameserver: {}", address) << '\n';
    }
}