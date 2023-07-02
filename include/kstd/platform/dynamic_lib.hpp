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
 * @since 06/05/2023
 */

#pragma once

#include <kstd/defaults.hpp>
#include <kstd/result.hpp>
#include <string>

#include "platform.hpp"

namespace kstd::platform {
    class DynamicLib final {
        std::string _name;
        KSTD_LIBRARY_HANDLE_TYPE _handle;

        [[nodiscard]] auto get_function_address(std::string_view name) noexcept -> Result<void*>;

        public:
        KSTD_NO_MOVE_COPY(DynamicLib)

        explicit DynamicLib(std::string name) noexcept;

        ~DynamicLib() noexcept = default;

        [[nodiscard]] auto load() noexcept -> Result<void>;

        [[nodiscard]] auto unload() noexcept -> Result<void>;

        template<typename R, typename... ARGS>
        [[nodiscard]] inline auto get_function(std::string_view name) noexcept -> Result<R (*)(ARGS...)> {
            auto address_result = get_function_address(name);

            if(!address_result) {
                return address_result.forward_error<R (*)(ARGS...)>();
            }

            return {reinterpret_cast<R (*)(ARGS...)>(*address_result)};// NOLINT
        }

        [[nodiscard]] inline auto get_name() const noexcept -> const std::string& {
            return _name;
        }

        [[nodiscard]] inline auto is_loaded() const noexcept -> bool {
            return _handle != nullptr;
        }

        [[nodiscard]] inline auto get_handle() const noexcept -> KSTD_LIBRARY_HANDLE_TYPE {
            return _handle;
        }
    };
}// namespace kstd::platform