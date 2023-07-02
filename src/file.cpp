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

#include "kstd/platform/file.hpp"

#include <kstd/utils.hpp>

#ifdef PLATFORM_UNIX

#include <sys/stat.h>

#if defined(CPU_64_BIT) && !defined(PLATFORM_APPLE)
#define KSTD_FSTAT ::fstat64
#define KSTD_FTRUNCATE ::ftruncate64
#define KSTD_FILE_STAT struct stat64
#else
#define KSTD_FSTAT ::fstat
#define KSTD_FTRUNCATE ::ftruncate
#define KSTD_FILE_STAT struct stat
#endif

#endif

namespace kstd::platform {
    File::File(std::filesystem::path path, FileMode mode) noexcept :
            _path(std::move(path)),
            _mode(mode),
            _handle() {
    }

    auto File::open() noexcept -> Result<void> {
        if(is_open()) {
            return make_error<void>(
                    std::string_view(fmt::format("Could not open file {}: already opened", _path.string())));
        }

        const auto exists = std::filesystem::exists(_path);

        if(!exists && _path.has_parent_path()) {
            const auto parent_path = _path.parent_path();

            if(!std::filesystem::exists(parent_path)) {
                std::filesystem::create_directories(parent_path);
            }
        }

#ifdef PLATFORM_WINDOWS
        auto* sec_desc = new SECURITY_DESCRIPTOR();// Heap-allocate new security descriptor

        if(::InitializeSecurityDescriptor(sec_desc, SECURITY_DESCRIPTOR_REVISION) == 0) {
            return make_error<void>(std::string_view(fmt::format("Could not allocate security descriptor for {}: {}",
                                                                 _path.string(), platform::get_last_error())));
        }

        _security_attribs.nLength = sizeof(SECURITY_ATTRIBUTES);
        _security_attribs.lpSecurityDescriptor = sec_desc;
        _security_attribs.bInheritHandle = true;// Make sure child-processes can inherit the handle of this file

        const auto wide_path = utils::to_utf16(_path.string());
        DWORD access = 0;

        switch(_mode) {
            case FileMode::READ: access = GENERIC_READ; break;
            case FileMode::WRITE: access = GENERIC_WRITE; break;
            case FileMode::READ_WRITE: access = GENERIC_READ | GENERIC_WRITE; break;
        }

        const auto disposition = exists ? OPEN_EXISTING : CREATE_NEW;
        _handle = platform::FileHandle(::CreateFileW(wide_path.data(), access, 0, &_security_attribs, disposition,
                                                     FILE_ATTRIBUTE_NORMAL, nullptr));
#else
        i32 access = 0;
        u32 security = 0;

        switch(_mode) {
            case FileMode::READ:
                access = O_RDONLY;
                security = S_IRUSR | S_IRGRP | S_IROTH;
                break;
            case FileMode::WRITE:
                access = O_RDWR;
                security = S_IWUSR | S_IWGRP | S_IWOTH;
                break;
            case FileMode::READ_WRITE:
                access = O_RDWR;
                security = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH;
                break;
        }

        if(!exists) {
            access |= O_CREAT;
        }

        _handle = platform::FileHandle(::open(_path.c_str(), access, security));// NOLINT
#endif

        if(!_handle.is_valid()) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not open file {}: {}", _path.string(), platform::get_last_error())));
        }

        return {};
    }

    auto File::close() noexcept -> Result<void> {
        if(!is_open()) {
            return make_error<void>(
                    std::string_view(fmt::format("Could not close file {}: not opened", _path.string())));
        }

#ifdef PLATFORM_WINDOWS
        if(!::CloseHandle(_handle)) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not close file {}: {}", _path.string(), platform::get_last_error())));
        }

        delete reinterpret_cast<SECURITY_DESCRIPTOR*>(_security_attribs.lpSecurityDescriptor);
#else
        if(::close(_handle) != 0) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not close file {}: {}", _path.string(), platform::get_last_error())));
        }
#endif

        _handle = platform::FileHandle();// Set handle to be invalid after this
        return {};
    }

    auto File::set_executable(bool is_executable) const noexcept -> Result<void> {
#ifdef PLATFORM_UNIX
        KSTD_FILE_STAT stats {};

        if(KSTD_FSTAT(_handle, &stats) != 0) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not stat file {}: {}", _path.string(), platform::get_last_error())));
        }

        auto mode = stats.st_mode;

        if(is_executable) {
            mode |= S_IXUSR | S_IXGRP | S_IXOTH;
        }
        else {
            mode &= ~(S_IXUSR | S_IXGRP | S_IXOTH);
        }

        if(::fchmod(_handle, mode) != 0) {
            return make_error<void>(std::string_view(fmt::format("Could not set executable bit for {}: {}",
                                                                 _path.string(), platform::get_last_error())));
        }
#endif

        return {};
    }

    auto File::is_executable() const noexcept -> Result<bool> {
#ifdef PLATFORM_WINDOWS
        const auto wide_path = utils::to_utf16(_path.string());
        DWORD type = 0;
        return make_ok<bool>(::GetBinaryTypeW(wide_path.data(), &type));
#else
        if(!is_open()) {
            return make_error<bool>(
                    std::string_view(fmt::format("Could not stat file {}: not opened", _path.string())));
        }

        KSTD_FILE_STAT stats {};

        if(KSTD_FSTAT(_handle, &stats) != 0) {
            return make_error<bool>(std::string_view(
                    fmt::format("Could not stat file {}: {}", _path.string(), platform::get_last_error())));
        }

        return make_ok<bool>(((stats.st_mode & S_IXUSR) == S_IXUSR) | ((stats.st_mode & S_IXGRP) == S_IXGRP) |
                             ((stats.st_mode & S_IXOTH) == S_IXOTH));// NOLINT
#endif
    }

    auto File::get_size() const noexcept -> Result<usize> {
        if(!is_open()) {
            return make_error<usize>(
                    std::string_view(fmt::format("Could not retrieve file size for {}: not opened", _path.string())));
        }

#ifdef PLATFORM_WINDOWS
        LARGE_INTEGER size {};

        if(!::GetFileSizeEx(_handle, &size)) {
            return make_error<usize>(std::string_view(fmt::format("Could not retrieve file size for {}: {}",
                                                                  _path.string(), platform::get_last_error())));
        }

        return make_ok<usize>(static_cast<usize>(size.QuadPart));
#else
        KSTD_FILE_STAT stats {};

        if(KSTD_FSTAT(_handle, &stats) != 0) {
            return make_error<usize>(std::string_view(fmt::format("Could not retrieve file size for {}: {}",
                                                                  _path.string(), platform::get_last_error())));
        }

        return make_ok<usize>(static_cast<usize>(stats.st_size));
#endif
    }

    auto File::resize(usize size) const noexcept -> Result<void> {
        if(!is_open()) {
            return make_error<void>(
                    std::string_view(fmt::format("Could not resize file {}: not opened", _path.string())));
        }

#ifdef PLATFORM_WINDOWS
        LARGE_INTEGER distance {};
        distance.QuadPart = static_cast<LONGLONG>(size);

        if(!::SetFilePointerEx(_handle, distance, nullptr, FILE_BEGIN)) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not set file pointer for {}: {}", _path.string(), platform::get_last_error())));
        }

        if(!::SetEndOfFile(_handle)) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not truncate file {}: {}", _path.string(), platform::get_last_error())));
        }

        distance.QuadPart = 0;

        if(!::SetFilePointerEx(_handle, distance, nullptr, FILE_BEGIN)) {
            return make_error<void>(std::string_view(fmt::format("Could not reset file pointer for {}: {}",
                                                                 _path.string(), platform::get_last_error())));
        }
#else
        if(KSTD_FTRUNCATE(_handle, static_cast<KSTD_OFFSET_TYPE>(size)) == -1) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not set file pointer for {}: {}", _path.string(), platform::get_last_error())));
        }
#endif

        return {};
    }

}// namespace kstd::platform