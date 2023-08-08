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
 * @author Alexander Hinze
 * @since 02/07/2023
 */

#ifdef PLATFORM_APPLE

#include "kstd/platform/file_mapping.hpp"
#include "kstd/platform/memory.hpp"

namespace kstd::platform::mm {
    FileMapping::FileMapping(const kstd::platform::mm::FileMapping& other) :
            FileMapping(other._file.get_path(), other._access) {
    }

    FileMapping::FileMapping(kstd::platform::mm::FileMapping&& other) noexcept :
            _file {std::move(other._file)},
            _type {other._type},
            _access {other._access},
            _address {other._address} {
        other._address = nullptr;
    }

    FileMapping::FileMapping() noexcept :
            _type {MappingType::FILE},
            _access {MappingAccess::NONE},
            _address {nullptr} {
    }

    FileMapping::FileMapping(std::filesystem::path path, MappingAccess access) :
            _file {file::File {std::move(path), derive_file_mode(access)}},
            _type {MappingType::FILE},
            _access {access},
            _address {nullptr} {
        const auto is_readable = (_access & MappingAccess::READ) == MappingAccess::READ;
        const auto is_writable = (_access & MappingAccess::WRITE) == MappingAccess::WRITE;
        const auto is_executable = (_access & MappingAccess::EXECUTE) == MappingAccess::EXECUTE;

        if(is_executable && !_file.is_executable()) {
            _file.set_executable().throw_if_error();
        }

        auto size = _file.get_size().get_or(0);

        if(size == 0) {
            _file.resize(1).throw_if_error();
            size = 1;// Make sure we map at least one byte of data
        }

        i32 prot = 0;
        i32 map_flags = MAP_SHARED | MAP_FILE;

        if(is_readable) {
            prot |= PROT_READ;
        }

        if(is_writable) {
            prot |= PROT_WRITE;
        }

        if(is_executable) {
            prot |= PROT_EXEC;
        }

        _address = ::mmap(nullptr, size, prot, map_flags, _file.get_handle(), 0);

        if(_address == nullptr) {
            throw std::runtime_error {
                    fmt::format("Could not map file {}: {}", _file.get_path().string(), get_last_error())};
        }
    }

    auto FileMapping::operator=(const kstd::platform::mm::FileMapping& other) -> FileMapping& {
        if(this == &other) {
            return *this;
        }
        *this = FileMapping {other._file.get_path(), other._access};
        return *this;
    }

    auto FileMapping::operator=(kstd::platform::mm::FileMapping&& other) noexcept -> FileMapping& {
        _file = std::move(other._file);
        _type = other._type;
        _access = other._access;
        _address = other._address;
        other._address = nullptr;
        return *this;
    }

    FileMapping::~FileMapping() noexcept {
        if(_address != nullptr) {
            const auto size = _file.get_size().get_or(0);

            if(size == 0) {
                return;
            }

            ::munmap(_address, size);
        }
    }

    auto FileMapping::resize(usize size) noexcept -> Result<void> {
        return _file.resize(size);
    }

    auto FileMapping::sync() noexcept -> Result<void> {
        auto size_result = _file.get_size();

        if(!size_result) {
            return size_result.forward<void>();
        }

        if(::msync(_address, *size_result, MS_SYNC) != 0) {
            return Error {fmt::format("Could not sync mapping: {}", get_last_error())};
        }

        return {};
    }

    auto FileMapping::get_type() const noexcept -> MappingType {
        return _type;
    }

    auto FileMapping::get_access() const noexcept -> MappingAccess {
        return _access;
    }

    auto FileMapping::get_address() const noexcept -> void* {
        return _address;
    }
}// namespace kstd::platform::mm

#endif// PLATFORM_APPLE