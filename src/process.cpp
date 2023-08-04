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
#include <kstd/utils.hpp>

#ifdef PLATFORM_WINDOWS
#include <Psapi.h>
#include <processthreadsapi.h>
#include <regex>
#define KSTD_MAX_PATH MAX_PATH
#else
#define KSTD_MAX_PATH PATH_MAX
#endif

#ifdef PLATFORM_APPLE
#include <libproc.h>
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
#if defined(PLATFORM_WINDOWS)
        std::array<wchar_t, KSTD_MAX_PATH> buffer {};
        auto handle_result = open_handle();// This handle will be automatically disposed
        if(!handle_result) {
            return handle_result.forward<std::filesystem::path>();
        }
        ::GetModuleFileNameExW(*handle_result, 0, buffer.data(), MAX_PATH);
#elif defined(PLATFORM_APPLE)
        std::array<char, KSTD_MAX_PATH> buffer {};
        if(::proc_pidpath(_id, buffer.data(), KSTD_MAX_PATH) < 0) {
            return Error {get_last_error()};
        }
#else
        std::array<char, KSTD_MAX_PATH> buffer {};
        auto exe_path = fmt::format("/proc/{}/exe", _id);
        if(::readlink(exe_path.c_str(), buffer.data(), KSTD_MAX_PATH) == -1) {
            return Error {get_last_error()};
        }
#endif
        return std::filesystem::path {buffer.data()};
    }

    auto Process::open_handle() const noexcept -> Result<ProcessHandle> {
#ifdef PLATFORM_WINDOWS
        using namespace std::string_literals;
        HANDLE handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, _id);
        if(handle == nullptr) {
            return Error {"Could not open process handle"s};
        }
        return ProcessHandle {handle};
#else
        return ProcessHandle {_id};
#endif
    }
}// namespace kstd::platform