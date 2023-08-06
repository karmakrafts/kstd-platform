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

namespace kstd::platform::mm {
    class FileMapping final : public MemoryMapping {
        file::File _file;
        MappingType _type;
        MappingAccess _access;
        void* _address;

#ifdef PLATFORM_WINDOWS
        file::FileHandle _handle {};
#endif

        public:
        FileMapping(const FileMapping& other);
        FileMapping(FileMapping&& other) noexcept;
        FileMapping() noexcept;

        FileMapping(std::filesystem::path path, MappingAccess access);

        ~FileMapping() noexcept;

        auto operator=(const FileMapping& other) -> FileMapping&;
        auto operator=(FileMapping&& other) noexcept -> FileMapping&;

        [[nodiscard]] auto resize(usize size) noexcept -> Result<void> final;

        [[nodiscard]] auto sync() noexcept -> Result<void> final;

        [[nodiscard]] auto get_type() const noexcept -> MappingType final;

        [[nodiscard]] auto get_access() const noexcept -> MappingAccess final;

        [[nodiscard]] auto get_address() const noexcept -> void* final;

        [[nodiscard]] inline auto get_file() const noexcept -> const file::File& {
            return _file;
        }

#ifdef PLATFORM_WINDOWS

        [[nodiscard]] inline auto get_handle() const noexcept -> file::FileHandle {
            return _handle;
        }

#endif
    };
}// namespace kstd::platform::mm