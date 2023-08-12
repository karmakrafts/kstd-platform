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
#include <kstd/bitflags.hpp>
#include <kstd/result.hpp>
#include <string>

#ifdef PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <WinDNS.h>
#include <Windows.h>
#else
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

        public:
        Resolver();
        ~Resolver();
        KSTD_DEFAULT_MOVE(Resolver, Resolver)
        KSTD_NO_COPY(Resolver, Resolver)

        [[nodiscard]] auto resolve(const std::string& address, RecordType type) noexcept -> kstd::Result<std::string>;
    };
}// namespace kstd::platform