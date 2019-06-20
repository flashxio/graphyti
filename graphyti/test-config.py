#!/usr/bin/env python

# Copyright 2019 neurodata (http://neurodata.io/)
# Written by Disa Mhembere (disa@jhu.edu)
#
# This file is part of knor.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from Configuration import Configuration
import os

conffn = "/home/disa/TEST_CONFIGS"
datafn = "/home/disa/TEST_DATA_LOC"

# Create a conf
Configuration.create_default_configs(conffn, datafn)
# Verify it
Configuration.verify_configs(conffn)

# Read it back
c = Configuration.get_configs(conffn)
# Verify read back
Configuration.verify_configs(c)

assert(Configuration.get_config("RAID_mapping") == "RAID0")
assert(Configuration.get_config("prof_file") == "graph.prof")
assert(Configuration.get_config("merge_reqs") == "")
assert(Configuration.get_config("io_depth") == "64")

# Test get config
dummy_config_map = {}
for key in c:
    dummy_config_map[key] = Configuration.get_config(key)

assert(dummy_config_map == c)

# Update conf
Configuration.set_config("num_nodes", "1")
assert(Configuration.get_config("num_nodes") == "1")

new_threads = int(Configuration.get_config("threads")) + 2
Configuration.set_config("threads", new_threads)

assert(Configuration.get_config("threads") == str(new_threads))
Configuration.verify_configs()

Configuration.set_config("merge_reqs", False)

# Read it back explictly
c = Configuration.get_configs(conffn)
# Verify read back
Configuration.verify_configs(c)

# Verify updated
os.remove(conffn)
os.removedirs(datafn)

print("TEST configuration.py successful!\n")
