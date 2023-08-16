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

namespace kstd::platform {
    Resolver::Resolver(const std::vector<std::string> dns_addresses) :
            _dns_addresses {dns_addresses} {
        if(dns_addresses.size() == 0) {
            throw std::runtime_error("Unable to initialize list of DNS servers: No DNS server specified");
        }
    }

    Resolver::Resolver() :
            _dns_addresses {} {
    }

    Resolver::~Resolver() {
    }

    auto Resolver::resolve(const std::string& address, const RecordType type) noexcept -> kstd::Result<std::string> {
        if(address == "localhost") {
            switch(type) {
                case RecordType::A: return kstd::Result<std::string> {"127.0.0.1"};
                case RecordType::AAAA: return kstd::Result<std::string> {"::1"};
            }
        }

        // Init resolv API
        res_init();

        // Generate custom nameserver list if needed
        if(_dns_addresses.size() > 0) {
            _res.nscount = _dns_addresses.size();
            for(int i = 0; i < _res.nscount; ++i) {
                if(is_ipv4_address(_dns_addresses[i])) {
                    _res.nsaddr_list[i].sin_family = AF_INET;
                }
                else if(is_ipv6_address(_dns_addresses[i])) {
                    _res.nsaddr_list[i].sin_family = AF_INET6;
                }
                else {
                    return kstd::Error {fmt::format("Unable to resolve address of {}: Illegal DNS server address {}",
                                                    address, _dns_addresses[i])};
                }
                _res.nsaddr_list[i].sin_addr.s_addr = ::inet_addr(_dns_addresses[i].c_str());
                _res.nsaddr_list[i].sin_port = htons(53);
            }
        }

        // Send DNS request
        std::array<u_char, 4096> response_buffer {'\0'};
        kstd::isize response_length = ::res_query(address.c_str(), __ns_class::ns_c_in, static_cast<int>(type),
                                                  response_buffer.data(), sizeof(response_buffer));

        if(response_length < 0) {
            return kstd::Error {fmt::format("Unable to resolve address {}: {}", address, get_last_error())};
        }

        if(response_length == 0) {
            return kstd::Error {fmt::format("Unable to resolve address {}: There is no response", address)};
        }
        return std::string {response_buffer.cbegin(), response_buffer.cbegin() + response_length + 1};
    }
}// namespace kstd::platform

#endif