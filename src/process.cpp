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
 * @since 03/07/2023
 */

#include "kstd/platform/process.hpp"

#include <array>
#include <fmt/format.h>
#include <kstd/libc.hpp>
#include <kstd/utils.hpp>

#ifdef PLATFORM_WINDOWS
#include <regex>
#define KSTD_MAX_PATH MAX_PATH
#else
#define KSTD_MAX_PATH PATH_MAX
#endif

namespace kstd::platform {
    auto Process::get_current() noexcept -> Process {
#ifdef PLATFORM_WINDOWS
        return {::GetCurrentProcessId()};
#else
        return {::getpid()};
#endif
    }

    auto Process::get_path() const noexcept -> Result<std::filesystem::path> {
        using namespace std::string_view_literals;
        std::array<char, KSTD_MAX_PATH> buffer {};

#ifdef PLATFORM_WINDOWS
        auto proc = ::OpenProcess(PROCESS_ALL_ACCESS, false, _handle);

        if(proc == nullptr) {
            return make_error<std::filesystem::path>("Could not open process handle"sv);
        }

        ::GetProcessImageFileNameA(proc, buffer.data(), MAX_PATH);
        ::CloseHandle(proc);
        std::string path(buffer.data());

        static std::regex pattern("\\\\\\\\Device\\\\\\\\HarddiskVolume[0-9]+\\\\\\\\");
        std::smatch match;

        if(!std::regex_search(path, match, pattern)) {
            return make_error<std::filesystem::path>("Could not find device match for kernel path"sv);
        }

        auto device_path = match[0].str();
        // TODO: ...
        return make_ok(std::filesystem::path(path));
#else
        auto exe_path = fmt::format("/proc/{}/exe", _id);
        if(::readlink(exe_path.c_str(), buffer.data(), KSTD_MAX_PATH) == -1) {
            return make_error<std::filesystem::path>(std::string_view(get_last_error()));
        }
        return make_ok(std::filesystem::path(buffer.data()));
#endif
    }

    auto Process::open_handle() noexcept -> ProcessHandle {
#ifdef PLATFORM_WINDOWS
        return {::OpenProcess(PROCESS_ALL_ACCESS, false, _id)};
#else
        return {_id};
#endif
    }
}// namespace kstd::platform