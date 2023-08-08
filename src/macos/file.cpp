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

#include "kstd/platform/file.hpp"

#include "kstd/utils.hpp"
#include <sys/stat.h>

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

        _handle = ::open(_path.c_str(), access, security);// NOLINT

        if(!_handle.is_valid()) {
            throw std::runtime_error {fmt::format("Could not open file {}: {}", _path.string(), get_last_error())};
        }
    }

    File::~File() noexcept {
        if(_handle.is_valid()) {
            ::close(_handle);
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
        struct stat stats {};

        if(::fstat(_handle, &stats) != 0) {
            return Error {fmt::format("Could not stat file {}: {}", _path.string(), get_last_error())};
        }

        auto mode = stats.st_mode;

        if(is_executable) {
            mode |= S_IXUSR | S_IXGRP | S_IXOTH;
        }
        else {
            mode &= ~(S_IXUSR | S_IXGRP | S_IXOTH);
        }

        if(::fchmod(_handle, mode) != 0) {
            return Error {fmt::format("Could not set executable bit for {}: {}", _path.string(), get_last_error())};
        }

        return {};
    }

    auto File::is_executable() const noexcept -> Result<bool> {
        struct stat stats {};

        if(::fstat(_handle, &stats) != 0) {
            return Error {fmt::format("Could not stat file {}: {}", _path.string(), get_last_error())};
        }

        return ((stats.st_mode & S_IXUSR) == S_IXUSR) | ((stats.st_mode & S_IXGRP) == S_IXGRP) |// NOLINT
               ((stats.st_mode & S_IXOTH) == S_IXOTH);                                          // NOLINT
    }

    auto File::get_size() const noexcept -> Result<usize> {
        struct stat stats {};

        if(::fstat(_handle, &stats) != 0) {
            return Error {fmt::format("Could not retrieve file size for {}: {}", _path.string(), get_last_error())};
        }

        return static_cast<usize>(stats.st_size);
    }

    auto File::resize(usize size) const noexcept -> Result<void> {
        if(::ftruncate(_handle, static_cast<NativeOffset>(size)) == -1) {
            return Error {fmt::format("Could not set file pointer for {}: {}", _path.string(), get_last_error())};
        }

        return {};
    }

}// namespace kstd::platform::file

#endif// PLATFORM_APPLE