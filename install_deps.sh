#!/bin/bash
#
# Copyright 2023 Karma Krafts & associates
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

sudo apt update -y
sudo apt upgrade -y
sudo apt install -y libnl-3-200 libnl-3-dev libnl-genl-3-200 libnl-genl-3-dev libnl-nf-3-200 libnl-nf-3-dev libnl-route-3-200 libnl-route-3-dev