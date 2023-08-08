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

#ifdef PLATFORM_APPLE

#include "kstd/platform/process.hpp"

#include <array>
#include <fmt/format.h>
#include <kstd/safe_alloc.hpp>
#include <libproc.h>

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
            _pid {invalid_process_id},
            _handle {invalid_process_handle} {
    }

    Process::Process(const NativeProcessId pid) :
            _pid {pid},
            _handle {pid} {
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

    Process::~Process() noexcept = default;

    auto Process::get_current() noexcept -> Result<Process> {
        return try_construct<Process>(::getpid());
    }

    auto Process::get_path() const noexcept -> Result<std::filesystem::path> {
        std::array<char, max_path> buffer {};
        if(::proc_pidpath(_pid, buffer.data(), max_path) < 0) {
            return Error {get_last_error()};
        }
        return std::filesystem::path {buffer.data()};
    }
}// namespace kstd::platform

#endif// PLATFORM_APPLE