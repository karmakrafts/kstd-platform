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

#include <gtest/gtest.h>
#include <kstd/platform/file_mapping.hpp>

TEST(kstd_platform_FileMapping, test_map_unmap) {
    using namespace kstd::platform;

    const auto access = mm::MappingAccess::READ | mm::MappingAccess::WRITE;
    mm::FileMapping mapping("./test/test_file_2.bin", access);

    ASSERT_TRUE(mapping.map().is_ok());

#ifdef PLATFORM_WINDOWS
    ASSERT_TRUE(mapping.get_handle().is_valid());
#endif

    ASSERT_TRUE(mapping.unmap().is_ok());
}