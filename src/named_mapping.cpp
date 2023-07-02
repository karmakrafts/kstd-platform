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

#include "kstd/platform/named_mapping.hpp"
#include "kstd/platform/memory.hpp"

namespace kstd::platform {
    NamedMapping::NamedMapping(std::string name, MappingAccess access, usize size) noexcept :
            Mapping(MappingType::NAMED, access),
            _name(std::move(name)),
            _size(size) {
    }

    auto NamedMapping::map() noexcept -> Result<void> {
        if(is_mapped()) {
            return make_error<void>(
                    std::string_view(fmt::format("Cannot map shared memory {}: already mapped", _name)));
        }

        const auto is_readable = !(_access & MappingAccess::READ);
        const auto is_writable = !(_access & MappingAccess::WRITE);
        const auto is_executable = !(_access & MappingAccess::EXECUTE);

#ifdef PLATFORM_WINDOWS
        auto* sec_desc = new SECURITY_DESCRIPTOR();// Heap-allocate new security descriptor

        if(::InitializeSecurityDescriptor(sec_desc, SECURITY_DESCRIPTOR_REVISION) == 0) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not allocate security descriptor for {}: {}", _name, get_last_error())));
        }

        _security_attribs.nLength = sizeof(SECURITY_ATTRIBUTES);
        _security_attribs.lpSecurityDescriptor = sec_desc;
        _security_attribs.bInheritHandle = true;// Make sure child-processes can inherit the handle of this file

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

        const auto wide_name = util::to_utf16(_name);
        _handle =
                FileHandle(::CreateFileMapping2(INVALID_HANDLE_VALUE, &_security_attribs, map_access, map_prot,
                                                SEC_COMMIT, static_cast<ULONG64>(_size), wide_name.data(), nullptr, 0));

        if(!_handle.is_valid()) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not open shared memory handle for {}: {}", _name, get_last_error())));
        }

        _address = ::MapViewOfFileEx(_handle, map_access, 0, 0, 0, nullptr);

        if(_address == nullptr) {
            return make_error<void>(
                    std::string_view(fmt::format("Could not map shared memory for {}: {}", _name, get_last_error())));
        }
#else
        i32 access = O_CREAT | O_EXCL;
        u32 security = 0;
        i32 map_prot = 0;
        i32 map_flags = MAP_SHARED;

        if((is_readable && is_writable) || is_writable) {
            access |= O_RDWR;
        }
        else if(is_readable) {
            access |= O_RDONLY;
        }

        if(is_readable) {
            security |= S_IRUSR;
            map_prot |= PROT_READ;
        }

        if(is_writable) {
            security |= S_IWUSR;
            map_prot |= PROT_WRITE;
        }

        if(is_executable) {
            security |= S_IXUSR;
            map_prot |= PROT_EXEC;
#ifndef PLATFORM_APPLE
            map_flags |= MAP_EXECUTABLE;
#endif
        }

        _handle = FileHandle(shm_open(_name.c_str(), access, security));

        if(!_handle.is_valid()) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not open shared memory handle for {}: {}", _name, get_last_error())));
        }

        _address = KSTD_MMAP(nullptr, _size, map_prot, map_flags, _handle, 0);

        if(_address == nullptr) {
            return make_error<void>(
                    std::string_view(fmt::format("Could not map shared memory for {}: {}", _name, get_last_error())));
        }
#endif

        return {};
    }

    auto NamedMapping::unmap() noexcept -> Result<void> {
        if(!is_mapped()) {
            return make_error<void>(std::string_view(fmt::format("Cannot unmap shared memory {}: not mapped", _name)));
        }

#ifdef PLATFORM_WINDOWS
        if(::UnmapViewOfFile(_address) == 0 || ::CloseHandle(_handle) == 0) {
            return make_error<void>(
                    std::string_view(fmt::format("Cannot unmap shared memory {}: {}", _name, get_last_error())));
        }

        delete reinterpret_cast<SECURITY_DESCRIPTOR*>(_security_attribs.lpSecurityDescriptor);
#else
        if(::munmap(_address, _size) == -1 || ::close(_handle) == -1 || ::shm_unlink(_name.c_str()) == -1) {
            return make_error<void>(
                    std::string_view(fmt::format("Cannot unmap shared memory {}: {}", _name, get_last_error())));
        }
#endif

        _handle = FileHandle();
        _address = nullptr;
        return {};
    }

    auto NamedMapping::resize(usize size) noexcept -> Result<void> {
        const auto mapped = is_mapped();

        if(mapped) {
            if(const auto result = unmap(); !result) {
                return result.forward_error<void>();
            }
        }

        _size = size;// Just update the size

        if(mapped) {
            if(const auto result = map(); !result) {
                return result.forward_error<void>();
            }
        }

        return {};
    }

    auto NamedMapping::sync() noexcept -> Result<void> {
        if(!is_mapped()) {
            return make_error<void>(std::string_view(fmt::format("Cannot unmap shared memory {}: not mapped", _name)));
        }

#ifdef PLATFORM_WINDOWS
#else
        if(::msync(_address, _size, MS_SYNC) != 0) {
            return make_error<void>(std::string_view(fmt::format("Could not sync mapping: {}", get_last_error())));
        }
#endif

        return {};
    }
}// namespace kstd::platform