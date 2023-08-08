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

#ifdef PLATFORM_WINDOWS

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

        DWORD map_prot = 0;
        DWORD map_access = 0;

        if(is_writable) {
            map_prot = (is_executable ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE);
        }
        else if(is_readable) {
            map_prot = (is_executable ? PAGE_EXECUTE_READ : PAGE_READONLY);
        }

        if(is_writable && is_readable) {
            map_access = FILE_MAP_ALL_ACCESS;
        }
        else if(is_writable) {
            map_access = FILE_MAP_WRITE;
        }
        else if(is_readable) {
            map_access = FILE_MAP_READ;
        }

        if(is_executable) {
            map_access |= FILE_MAP_EXECUTE;
        }

        _handle = ::CreateFileMappingW(_file.get_handle(), &_file.get_security_attribs(), map_prot, 0, 0, nullptr);

        if(!_handle.is_valid()) {
            throw std::runtime_error {fmt::format("Could not open shared memory handle for {}: {}",
                                                  _file.get_path().string(), get_last_error())};
        }

        _address = ::MapViewOfFileEx(_handle, map_access, 0, 0, 0, nullptr);

        if(_address == nullptr) {
            throw std::runtime_error {
                    fmt::format("Could not map shared memory for {}: {}", _file.get_path().string(), get_last_error())};
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
            ::UnmapViewOfFile(_address);
            ::CloseHandle(_handle);
        }
    }

    auto FileMapping::resize(usize size) noexcept -> Result<void> {
        return _file.resize(size);
    }

    auto FileMapping::sync() noexcept -> Result<void> {
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

#endif// PLATFORM_WINDOWS