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
#include <kstd/platform/file.hpp>

TEST(kstd_io_File, TestOpenClose) {
    kstd::platform::File file("./test/test_file.bin", kstd::platform::FileMode::READ_WRITE);

    ASSERT_TRUE(file.open().is_ok());
    ASSERT_TRUE(file.get_handle().is_valid());
    ASSERT_TRUE(file.close().is_ok());
}