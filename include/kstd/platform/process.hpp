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

#pragma once

#include "platform.hpp"
#include "process_handle.hpp"
#include <filesystem>
#include <kstd/defaults.hpp>
#include <kstd/result.hpp>

namespace kstd::platform {
    class Process final {
        NativeProcessId _id;

        inline Process(NativeProcessId id) noexcept :
                _id(id) {
        }

        public:
        KSTD_DEFAULT_MOVE_COPY(Process)

        ~Process() noexcept = default;

        [[nodiscard]] static auto get_current() noexcept -> Process;

        [[nodiscard]] auto get_path() const noexcept -> Result<std::filesystem::path>;

        [[nodiscard]] auto open_handle() noexcept -> ProcessHandle;

        [[nodiscard]] constexpr auto get_id() const noexcept -> NativeProcessId {
            return _id;
        }
    };
}// namespace kstd::platform