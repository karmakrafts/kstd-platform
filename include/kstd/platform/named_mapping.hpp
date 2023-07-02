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

namespace kstd::platform {
    class NamedMapping final : public Mapping {
        std::string _name;
        usize _size;
        FileHandle _handle;

#ifdef PLATFORM_WINDOWS
        SECURITY_ATTRIBUTES _security_attribs {};
#endif

        public:
        NamedMapping(std::string name, MappingAccess access, usize size) noexcept;

        ~NamedMapping() noexcept final = default;

        [[nodiscard]] auto map() noexcept -> Result<void> final;

        [[nodiscard]] auto unmap() noexcept -> Result<void> final;

        [[nodiscard]] auto resize(usize size) noexcept -> Result<void> final;

        [[nodiscard]] auto sync() noexcept -> Result<void> final;

        [[nodiscard]] inline auto get_name() const noexcept -> const std::string& {
            return _name;
        }

        [[nodiscard]] inline auto get_size() const noexcept -> usize {
            return _size;
        }

        [[nodiscard]] inline auto get_handle() const noexcept -> FileHandle {
            return _handle;
        }

#ifdef PLATFORM_WINDOWS

        [[nodiscard]] inline auto get_security_attribs() noexcept -> SECURITY_ATTRIBUTES& {
            return _security_attribs;
        }

#endif
    };
}// namespace kstd::platform