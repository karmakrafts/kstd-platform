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

#include <fmt/format.h>
#include <kstd/result.hpp>
#include <unordered_set>
#include <kstd/option.hpp>

#ifdef PLATFORM_WINDOWS
#include <Winsock2.h>
#include <iphlpapi.h>
#include <kstd/types.hpp>
#else
#include <sys/socket.h>
#endif

namespace kstd::platform {

    enum class AddressFamily : u8 {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX,
        IPX = AF_IPX,
        APPLE_TALK = AF_APPLETALK
    };

    struct NetworkInterface final {
        std::string name;
        std::string description;
        std::unordered_set<AddressFamily> address_families;
        Option<kstd::usize> link_speed;
    };

    [[nodiscard]] auto enumerate_interfaces() noexcept -> Result<std::vector<NetworkInterface>>;

}// namespace kstd::platform