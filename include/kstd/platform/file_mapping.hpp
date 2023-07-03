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

#include "memory_mapping.hpp"
#include <filesystem>

namespace kstd::platform {
    class FileMapping final : public MemoryMapping {
        File _file;

#ifdef PLATFORM_WINDOWS
        FileHandle _handle {};
#endif

        [[nodiscard]] auto soft_map() noexcept -> Result<void>;

        [[nodiscard]] auto soft_unmap() noexcept -> Result<void>;

        public:
        KSTD_NO_MOVE_COPY(FileMapping)

        FileMapping(std::filesystem::path path, MappingAccess access) noexcept;

        ~FileMapping() noexcept final = default;

        [[nodiscard]] auto map() noexcept -> Result<void> final;

        [[nodiscard]] auto unmap() noexcept -> Result<void> final;

        [[nodiscard]] auto resize(usize size) noexcept -> Result<void> final;

        [[nodiscard]] auto sync() noexcept -> Result<void> final;

        [[nodiscard]] inline auto get_file() const noexcept -> const File& {
            return _file;
        }

#ifdef PLATFORM_WINDOWS

        [[nodiscard]] inline auto get_handle() const noexcept -> FileHandle {
            return _handle;
        }

#endif
    };
}// namespace kstd::platform