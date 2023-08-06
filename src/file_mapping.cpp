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

#include "kstd/platform/file_mapping.hpp"
#include "kstd/platform/memory.hpp"

#ifdef PLATFORM_UNIX
#if defined(CPU_64_BIT) && !defined(PLATFORM_APPLE)
#define KSTD_MMAP ::mmap64
#else
#define KSTD_MMAP ::mmap
#endif
#endif

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

#ifdef PLATFORM_WINDOWS
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
#else
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
#ifndef PLATFORM_APPLE
            map_flags |= MAP_EXECUTABLE;
#endif
        }

        _address = KSTD_MMAP(nullptr, size, prot, map_flags, _file.get_handle(), 0);

        if(_address == nullptr) {
            throw std::runtime_error {
                    fmt::format("Could not map file {}: {}", _file.get_path().string(), get_last_error())};
        }
#endif
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
#ifdef PLATFORM_WINDOWS
            ::UnmapViewOfFile(_address);
            ::CloseHandle(_handle);
#else
            const auto size = _file.get_size().get_or(0);

            if(size == 0) {
                return;
            }

            ::munmap(_address, size);
#endif
        }
    }

    auto FileMapping::resize(usize size) noexcept -> Result<void> {
        return _file.resize(size);
    }

    auto FileMapping::sync() noexcept -> Result<void> {
#ifdef PLATFORM_WINDOWS
#else
        auto size_result = _file.get_size();

        if(!size_result) {
            return size_result.forward<void>();
        }

        if(::msync(_address, *size_result, MS_SYNC) != 0) {
            return Error {fmt::format("Could not sync mapping: {}", get_last_error())};
        }
#endif

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