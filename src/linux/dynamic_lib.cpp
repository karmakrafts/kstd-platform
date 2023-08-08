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

#ifdef PLATFORM_LINUX

#include "kstd/platform/dynamic_lib.hpp"

#include <dlfcn.h>
#include <fmt/format.h>
#include <kstd/utils.hpp>

namespace kstd::platform {
    DynamicLib::DynamicLib(std::string name) :
            _name {std::move(name)},
            _handle {::dlopen(_name.c_str(), RTLD_LAZY)} {
        if(_handle == invalid_module_handle) {
            throw std::runtime_error {fmt::format("Could not open shared object {}: {}", _name, get_last_error())};
        }
    }

    DynamicLib::DynamicLib() noexcept :
            _handle {invalid_module_handle} {
    }

    DynamicLib::~DynamicLib() noexcept {
        if(_handle != invalid_module_handle) {
            ::dlclose(_handle);
        }
    }

    DynamicLib::DynamicLib(const kstd::platform::DynamicLib& other) :
            DynamicLib(other._name) {
    }

    DynamicLib::DynamicLib(kstd::platform::DynamicLib&& other) noexcept :
            _name {std::move(other._name)},
            _handle {other._handle} {
        other._handle = invalid_module_handle;
    }

    auto DynamicLib::operator=(const kstd::platform::DynamicLib& other) -> DynamicLib& {
        if(this == &other) {
            return *this;
        }
        *this = DynamicLib {other._name};
        return *this;
    }

    auto DynamicLib::operator=(kstd::platform::DynamicLib&& other) noexcept -> DynamicLib& {
        _name = std::move(other._name);
        _handle = other._handle;
        other._handle = invalid_module_handle;
        return *this;
    }

    auto DynamicLib::get_function_address(const std::string& name) noexcept -> Result<void*> {
        auto* address = ::dlsym(_handle, name.data());

        if(address == nullptr) {
            return Error {fmt::format("Could not resolve function {} in {}: {}", name, _name, get_last_error())};
        }

        return reinterpret_cast<void*>(address);// NOLINT
    }
}// namespace kstd::platform

#endif// PLATFORM_LINUX