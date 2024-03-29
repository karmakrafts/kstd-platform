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

#pragma once

#include "file.hpp"
#include <kstd/bitflags.hpp>
#include <kstd/defaults.hpp>
#include <kstd/result.hpp>
#include <kstd/types.hpp>

namespace kstd::platform::mm {
    enum class MappingType : u8 {
        FILE
    };

    KSTD_BITFLAGS(u8, MappingAccess, READ = 0x01U, WRITE = 0x02U, EXECUTE = 0x04U)// NOLINT

    [[nodiscard]] inline auto derive_file_mode(MappingAccess access) noexcept -> file::FileMode {
        const auto is_readable = (access & MappingAccess::READ) == MappingAccess::READ;
        const auto is_writable = (access & MappingAccess::WRITE) == MappingAccess::WRITE;

        if(is_readable && is_writable) {
            return file::FileMode::READ_WRITE;
        }
        else if(is_writable) {// NOLINT
            return file::FileMode::READ;
        }

        return file::FileMode::WRITE;
    }

    [[nodiscard]] inline auto derive_access(file::FileMode mode) noexcept -> MappingAccess {
        switch(mode) {
            case file::FileMode::WRITE: return MappingAccess::WRITE;
            case file::FileMode::READ_WRITE: return MappingAccess::READ | MappingAccess::WRITE;
            default: return MappingAccess::READ;
        }
    }

    struct MemoryMapping {
        [[nodiscard]] virtual auto resize(usize size) noexcept -> Result<void> = 0;

        [[nodiscard]] virtual auto sync() noexcept -> Result<void> = 0;

        [[nodiscard]] virtual auto get_type() const noexcept -> MappingType = 0;

        [[nodiscard]] virtual auto get_access() const noexcept -> MappingAccess = 0;

        [[nodiscard]] virtual auto get_address() const noexcept -> void* = 0;
    };
}// namespace kstd::platform::mm