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

#ifdef PLATFORM_WINDOWS

#include "kstd/platform/dns.hpp"
#include <WS2tcpip.h>
#include <stdexcept>

namespace kstd::platform {

    Resolver::Resolver() {
        // Initialize WSA and throw exception if failed
        WSADATA wsaData {};
        if(FAILED(WSAStartup(MAKEWORD(2, 2), &wsaData))) {
            throw std::runtime_error {fmt::format("Unable to startup WSA: {}", get_last_error())};
        }
    }

    Resolver::~Resolver() {
        // Cleanup WSA
        WSACleanup();
    }

    auto Resolver::resolve(const std::string& address, const RecordType type) noexcept -> kstd::Result<std::string> {

        // Send request over DnsQuery function
        PDNS_RECORD record = nullptr;
        if(FAILED(DnsQuery(address.data(), static_cast<kstd::u16>(type), DNS_QUERY_STANDARD | DNS_QUERY_BYPASS_CACHE,
                           nullptr, &record, nullptr))) {
            return kstd::Error {fmt::format("Error while resolving {}: {}", address, get_last_error())};
        }

        // Convert record content
        std::string result;
        switch(type) {
            case RecordType::A: {
                // Convert binary representation to literal representation of IPv4 address
                SOCKADDR_IN addr {};
                addr.sin_family = AF_INET;
                addr.sin_addr = *reinterpret_cast<in_addr*>(&record->Data.A.IpAddress);// NOLINT

                std::array<wchar_t, 128> address_buffer {};
                DWORD size = address_buffer.size();
                if(FAILED(WSAAddressToStringW(reinterpret_cast<LPSOCKADDR>(&addr), sizeof(addr), nullptr,// NOLINT
                                              address_buffer.data(), &size))) {
                    return kstd::Error {
                            fmt::format("Unable to format IPv4 Address into literal => {}", get_last_error())};
                }

                result = std::string {address_buffer.begin(), address_buffer.begin() + size - 1};
                break;
            }
            case RecordType::AAAA: {
                // Convert binary representation to literal representation of IPv6 address
                SOCKADDR_IN6 addr {};
                addr.sin6_family = AF_INET6;
                addr.sin6_addr = *reinterpret_cast<in_addr6*>(&record->Data.AAAA.Ip6Address);// NOLINT

                std::array<wchar_t, 128> address_buffer {};
                DWORD size = address_buffer.size();
                if(FAILED(WSAAddressToStringW(reinterpret_cast<LPSOCKADDR>(&addr), sizeof(addr), nullptr,// NOLINT
                                              address_buffer.data(), &size))) {
                    return kstd::Error {
                            fmt::format("Unable to format IPv4 Address into literal => {}", get_last_error())};
                }

                result = std::string {address_buffer.begin(), address_buffer.begin() + size - 1};
                break;
            }
        }
        DnsRecordListFree(record, DnsFreeRecordListDeep);
        return result;
    }

}// namespace kstd::platform
#endif