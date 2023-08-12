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

#ifdef PLATFORM_APPLE

#include "kstd/platform/dns.hpp"
#include <array>

namespace kstd::platform {
    Resolver::Resolver() {
    }

    Resolver::~Resolver() {
    }

    auto Resolver::resolve(const std::string& address, const RecordType type) noexcept -> kstd::Result<std::string> {
        // TODO: Is this really necessary?
        if (address == "localhost") {
            switch(type) {
                case RecordType::A: return kstd::Result<std::string> {"127.0.0.1"};
                case RecordType::AAAA: return kstd::Result<std::string> {"::1"};
            }
        }

        std::array<u_char, 256> response {};
        kstd::usize response_length = res_query(address.c_str(), C_IN, static_cast<int>(type), response.data(),
                                                response.size() * sizeof(char));
        if(response_length < 0) {
            return kstd::Error {fmt::format("Unable to resolve address {}: {}", address, get_last_error())};
        }
        return std::string {response.begin(), response.begin()};
    }
}// namespace kstd::platform

#endif