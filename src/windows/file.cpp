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

#include "kstd/platform/file.hpp"

#include <kstd/utils.hpp>

namespace kstd::platform::file {
    File::File(const File& other) :
            File(other._path, other._mode) {
    }

    File::File(File&& other) noexcept :
            _path {std::move(other._path)},
            _mode {other._mode},
            _handle {other._handle} {
        other._handle = invalid_file_handle;
    }

    File::File() noexcept :
            _mode {file::FileMode::READ},
            _handle {invalid_file_handle} {
    }

    File::File(std::filesystem::path path, FileMode mode) :
            _path {std::move(path)},
            _mode {mode} {
        const auto exists = std::filesystem::exists(_path);

        if(!exists && _path.has_parent_path()) {
            const auto parent_path = _path.parent_path();

            if(!std::filesystem::exists(parent_path)) {
                std::filesystem::create_directories(parent_path);
            }
        }

        auto* sec_desc = new SECURITY_DESCRIPTOR();// Heap-allocate new security descriptor

        if(::InitializeSecurityDescriptor(sec_desc, SECURITY_DESCRIPTOR_REVISION) == 0) {
            throw std::runtime_error {
                    fmt::format("Could not allocate security descriptor for {}: {}", _path.string(), get_last_error())};
        }

        _security_attribs.nLength = sizeof(SECURITY_ATTRIBUTES);
        _security_attribs.lpSecurityDescriptor = sec_desc;
        _security_attribs.bInheritHandle = true;// Make sure child-processes can inherit the handle of this file

        const auto wide_path = utils::to_wcs(_path.string());
        DWORD access = 0;

        switch(_mode) {
            case FileMode::READ: access = GENERIC_READ; break;
            case FileMode::WRITE: access = GENERIC_WRITE; break;
            case FileMode::READ_WRITE: access = GENERIC_READ | GENERIC_WRITE; break;
        }

        const auto disposition = exists ? OPEN_EXISTING : CREATE_NEW;
        _handle = ::CreateFileW(wide_path.data(), access, 0, &_security_attribs, disposition, FILE_ATTRIBUTE_NORMAL,
                                nullptr);

        if(!_handle.is_valid()) {
            throw std::runtime_error {fmt::format("Could not open file {}: {}", _path.string(), get_last_error())};
        }
    }

    File::~File() noexcept {
        if(_handle.is_valid()) {
            ::CloseHandle(_handle);
            delete reinterpret_cast<SECURITY_DESCRIPTOR*>(_security_attribs.lpSecurityDescriptor);
        }
    }

    auto File::operator=(const kstd::platform::file::File& other) -> File& {
        if(this == &other) {
            return *this;
        }
        *this = File {other._path, other._mode};
        return *this;
    }

    auto File::operator=(kstd::platform::file::File&& other) noexcept -> File& {
        _path = std::move(other._path);
        _mode = other._mode;
        _handle = other._handle;
        other._handle = invalid_file_handle;
        return *this;
    }

    auto File::set_executable(bool is_executable) const noexcept -> Result<void> {
        return {};
    }

    auto File::is_executable() const noexcept -> Result<bool> {
        const auto wide_path = utils::to_wcs(_path.string());
        DWORD type = 0;
        return ::GetBinaryTypeW(wide_path.data(), &type);
    }

    auto File::get_size() const noexcept -> Result<usize> {
        LARGE_INTEGER size {};

        if(!::GetFileSizeEx(_handle, &size)) {
            return Error {fmt::format("Could not retrieve file size for {}: {}", _path.string(), get_last_error())};
        }

        return static_cast<usize>(size.QuadPart);
    }

    auto File::resize(usize size) const noexcept -> Result<void> {
        LARGE_INTEGER distance {};
        distance.QuadPart = static_cast<LONGLONG>(size);

        if(!::SetFilePointerEx(_handle, distance, nullptr, FILE_BEGIN)) {
            return Error {fmt::format("Could not set file pointer for {}: {}", _path.string(), get_last_error())};
        }

        if(!::SetEndOfFile(_handle)) {
            return Error {fmt::format("Could not truncate file {}: {}", _path.string(), get_last_error())};
        }

        distance.QuadPart = 0;

        if(!::SetFilePointerEx(_handle, distance, nullptr, FILE_BEGIN)) {
            return Error {fmt::format("Could not reset file pointer for {}: {}", _path.string(), get_last_error())};
        }

        return {};
    }

}// namespace kstd::platform::file

#endif// PLATFORM_WINDOWS