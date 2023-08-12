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

#pragma once

#include "kstd/platform/platform.hpp"
#include <fmt/format.h>
#include <initializer_list>
#include <kstd/bitflags.hpp>
#include <kstd/result.hpp>
#include <regex>
#include <stdexcept>
#include <string>

#ifdef PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <WinDNS.h>
#include <Windows.h>
#include <kstd/option.hpp>
#else
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netinet/in.h>
#include <resolv.h>
#endif

namespace kstd::platform {

#if defined(PLATFORM_WINDOWS)
    enum class RecordType : kstd::u16 {
        A = DNS_TYPE_A,
        AAAA = DNS_TYPE_AAAA
    };
#elif defined(PLATFORM_LINUX)
    enum class RecordType : kstd::u16 {
        A = T_A,
        AAAA = T_AAAA
    };
#else
    enum class RecordType : kstd::u16 {
        A = __ns_type::ns_t_a,
        AAAA = __ns_type::ns_t_aaaa
    };
#endif

    class Resolver final {
        // TODO: Add documentation and comments to every function
#ifdef PLATFORM_WINDOWS
        kstd::Option<DNS_ADDR_ARRAY> _dns_addresses;
#else
        std::vector<std::string> _dns_addresses;
#endif

        public:
        Resolver(std::vector<std::string> dns_addresses);
        Resolver();
        ~Resolver();
        KSTD_DEFAULT_MOVE(Resolver, Resolver)
        KSTD_NO_COPY(Resolver, Resolver)

        [[nodiscard]] auto resolve(const std::string& address, RecordType type) noexcept -> kstd::Result<std::string>;
    };

    [[nodiscard]] auto enumerate_nameservers() noexcept -> kstd::Result<std::vector<std::string>>;

    inline auto is_ipv4_address(const std::string& address) noexcept -> bool {
        static auto s_pattern = std::regex(R"(([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3}))");
        return std::regex_match(address, s_pattern);
    }

    inline auto is_ipv6_address(const std::string& address) noexcept -> bool {
        static auto s_pattern = std::regex(
                "(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-"
                "F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){"
                "1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}"
                "|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,"
                "4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25["
                "0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]"
                ")\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))");
        return std::regex_match(address, s_pattern);
    }
}// namespace kstd::platform