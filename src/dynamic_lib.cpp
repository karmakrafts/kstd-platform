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

#include "kstd/platform/dynamic_lib.hpp"

#ifdef PLATFORM_WINDOWS
#include <libloaderapi.h>
#else

#include <dlfcn.h>

#endif

#include <fmt/format.h>

namespace kstd::platform {
    DynamicLib::DynamicLib(std::string name) noexcept :
            _name(std::move(name)),
            _handle(nullptr) {
    }

    auto DynamicLib::load() noexcept -> Result<void> {
#ifdef PLATFORM_WINDOWS
        const auto wide_name = util::to_utf16(_name);
        _handle = ::LoadLibraryW(wide_name.data());

        if(_handle == nullptr) {
            return {std::unexpected(fmt::format("Could not open DLL {}: {}", _name, platform::get_last_error()))};
        }
#else
        _handle = ::dlopen(_name.c_str(), RTLD_LAZY);

        if(_handle == nullptr) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not open shared object {}: {}", _name, platform::get_last_error())));
        }
#endif

        return {};
    }

    auto DynamicLib::unload() noexcept -> Result<void> {
#ifdef PLATFORM_WINDOWS
        if(!::FreeLibrary(_handle)) {
            return {std::unexpected(fmt::format("Could not close DLL {}: {}", _name, platform::get_last_error()))};
        }
#else
        if(::dlclose(_handle) != 0) {
            return make_error<void>(std::string_view(
                    fmt::format("Could not close shared object {}: {}", _name, platform::get_last_error())));
        }
#endif

        return {};
    }

    auto DynamicLib::get_function_address(const std::string_view& name) noexcept -> Result<void*> {
#ifdef PLATFORM_WINDOWS
        auto* address = ::GetProcAddress(_handle, name.data());
#else
        auto* address = ::dlsym(_handle, name.data());
#endif

        if(address == nullptr) {
            return make_error<void*>(std::string_view(
                    fmt::format("Could not resolve function {} in {}: {}", name, _name, platform::get_last_error())));
        }

        return {reinterpret_cast<void*>(address)};// NOLINT
    }
}// namespace kstd::platform