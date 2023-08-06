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
#include <kstd/safe_alloc.hpp>

#if defined(PLATFORM_WINDOWS)
#include <Psapi.h>
#include <processthreadsapi.h>
#elif defined(PLATFORM_APPLE)
#include <libproc.h>
#endif

namespace kstd::platform {
    Process::Process(const Process& other) :
            Process(other._pid) {
    }

    Process::Process(Process&& other) noexcept :
            _pid {other._pid},
            _handle {other._handle} {
        other._handle = invalid_process_handle;
    }

    Process::Process() noexcept :
            _pid {-1},
            _handle {invalid_process_handle} {
    }

    Process::Process(const NativeProcessId pid) :
            _pid {pid},
#ifdef PLATFORM_WINDOWS
            _handle {::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid)}
#else
            _handle {pid}
#endif
    {
        if(_handle == invalid_process_handle) {
            throw std::runtime_error(fmt::format("Could not open process handle: {}", get_last_error()));
        }
    }

    auto Process::operator=(const Process& other) -> Process& {
        *this = Process {other._pid};
        return *this;
    }

    auto Process::operator=(Process&& other) noexcept -> Process& {
        _pid = other._pid;
        _handle = other._handle;
        other._handle = invalid_process_handle;
        return *this;
    }

    Process::~Process() noexcept {
#ifdef PLATFORM_WINDOWS
        if(_handle != invalid_process_handle) {
            ::CloseHandle(_handle);
        }
#endif
    }

    auto Process::get_current() noexcept -> Result<Process> {
#ifdef PLATFORM_WINDOWS
        return try_construct<Process>(::GetCurrentProcessId());
#else
        return try_construct<Process>(::getpid());
#endif
    }

    auto Process::get_path() const noexcept -> Result<std::filesystem::path> {
#if defined(PLATFORM_WINDOWS)
        std::array<wchar_t, max_path> buffer {};
        if(::GetModuleFileNameExW(_handle, ::GetModuleHandleW(nullptr), buffer.data(), MAX_PATH) == 0) {
            return Error {fmt::format("Could not retrieve process path: {}", get_last_error())};
        }
#elif defined(PLATFORM_APPLE)
        std::array<char, max_path> buffer {};
        if(::proc_pidpath(_pid, buffer.data(), max_path) < 0) {
            return Error {get_last_error()};
        }
#else
        std::array<char, max_path> buffer {};
        auto exe_path = fmt::format("/proc/{}/exe", _pid);
        if(::readlink(exe_path.c_str(), buffer.data(), max_path) == -1) {
            return Error {get_last_error()};
        }
#endif
        return std::filesystem::path {buffer.data()};
    }
}// namespace kstd::platform