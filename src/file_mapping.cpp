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

namespace kstd::platform {
    FileMapping::FileMapping(std::filesystem::path path, MappingAccess access) noexcept :
            Mapping(MappingType::FILE, access),
            _file(File(std::move(path), derive_file_mode(access))) {
    }

    auto FileMapping::soft_map() noexcept -> Result<void> {
        if(is_mapped()) {
            return make_error<void>(
                    std::string_view(fmt::format("Cannot map file {}: already mapped", _file.get_path().string())));
        }

        const auto is_readable = !(_access & MappingAccess::READ);
        const auto is_writable = !(_access & MappingAccess::WRITE);
        const auto is_executable = !(_access & MappingAccess::EXECUTE);

        if(is_executable && !_file.is_executable()) {
            if(const auto result = _file.set_executable(); !result) {
                return result.forward_error<void>();
            }
        }

        auto size = _file.get_size().unwrap_or(0);

        if(size == 0) {
            if(const auto result = _file.resize(1); !result) {
                return result.forward_error<void>();
            }

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

        _handle = FileHandle(
                ::CreateFileMappingW(_file.get_handle(), &_file.get_security_attribs(), map_prot, 0, 0, nullptr));

        if(!_handle.is_valid()) {
            return make_error<void>(std::string_view(fmt::format("Could not open shared memory handle for {}: {}",
                                                                 _file.get_path().string(), get_last_error())));
        }

        _address = ::MapViewOfFileEx(_handle, map_access, 0, 0, 0, nullptr);

        if(_address == nullptr) {
            return make_error<void>(std::string_view(fmt::format("Could not map shared memory for {}: {}",
                                                                 _file.get_path().string(), get_last_error())));
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
            return make_error<void>(std::string_view(
                    fmt::format("Could not map file {}: {}", _file.get_path().string(), get_last_error())));
        }
#endif

        return {};
    }

    auto FileMapping::soft_unmap() noexcept -> Result<void> {
        if(!is_mapped()) {
            return make_error<void>(
                    std::string_view(fmt::format("Cannot unmap file {}: not mapped", _file.get_path().string())));
        }

#ifdef PLATFORM_WINDOWS
        if(::UnmapViewOfFile(_address) == 0 || ::CloseHandle(_handle) == 0) {
            return make_error<void>(std::string_view(
                    fmt::format("Cannot unmap shared memory {}: {}", _file.get_path().string(), get_last_error())));
        }

        _handle = FileHandle();
#else
        const auto size = _file.get_size().unwrap_or(0);

        if(size == 0) {
            return make_error<void>(
                    std::string_view(fmt::format("Cannot unmap zero-size file {}", _file.get_path().string())));
        }

        if(::munmap(_address, size) == -1) {
            return make_error<void>(std::string_view(
                    fmt::format("Cannot unmap file {}: {}", _file.get_path().string(), get_last_error())));
        }
#endif

        _address = nullptr;
        return {};
    }

    auto FileMapping::map() noexcept -> Result<void> {
        if(const auto result = _file.open(); !result) {
            return result.forward_error<void>();
        }

        return soft_map();
    }

    auto FileMapping::unmap() noexcept -> Result<void> {
        if(const auto result = soft_unmap(); !result) {
            return result.forward_error<void>();
        }

        return _file.close();
    }

    auto FileMapping::resize(usize size) noexcept -> Result<void> {
        const auto mapped = is_mapped();

        if(mapped) {
            if(const auto result = soft_unmap(); !result) {
                return result.forward_error<void>();
            }
        }

        if(const auto result = _file.resize(size); !result) {
            return result.forward_error<void>();
        }

        if(mapped) {
            if(const auto result = soft_map(); !result) {
                return result.forward_error<void>();
            }
        }

        return {};
    }

    auto FileMapping::sync() noexcept -> Result<void> {
        if(!is_mapped()) {
            return make_error<void>(std::string_view(
                    fmt::format("Cannot synchronize memory to file {}: not mapped", _file.get_path().string())));
        }

#ifdef PLATFORM_WINDOWS
#else
        auto size_result = _file.get_size();

        if(!size_result) {
            return size_result.forward_error<void>();
        }

        if(::msync(_address, size_result.unwrap(), MS_SYNC) != 0) {
            return make_error<void>(std::string_view(fmt::format("Could not sync mapping: {}", get_last_error())));
        }
#endif

        return {};
    }
}// namespace kstd::platform