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

__configsfile__ = ""
__datalocationfile__ = ""

__default_config__ = {
        "RAID_mapping" : "RAID0",
        "io_depth" : "64",
        "cache_size" : "4G",
        "num_nodes" : "4",
        "root_conf" : "",
        "threads" : "4",
        "print_io_stat" : "",
        "prof_file" : "graph.prof",
        "merge_reqs" : ""
}

class Configuration:
    @staticmethod
    def __write_configs(configs):
        import os
        if not __configsfile__:
            raise RuntimeError("Configuration file not set!\n")

        with open(__configsfile__, "wb") as file:
            for config in configs:
                if (type(configs[config]) == bool):
                    if configs[config]:
                        s = config + "=\n"
                    else:
                        continue # Skip this config
                else:
                    s = config + "=" + str(configs[config]) + "\n"

                file.write(s.encode("UTF-8"))

    @staticmethod
    def set_configs(configs, filename=""):
        global __configsfile__

        if type(configs) == str:
            import os
            if not os.path.exists(configs):
                raise RuntimeError(
                    "Configuration file '{}' not found!\n ".format(configs))
            __configsfile__ = configs
        elif type(configs) == dict:
            if not filename:
                raise RuntimeError("Configuration file must have location\n")

            __configsfile__ = filename
            Configuration.__write_configs(configs)
        else:
            raise RuntimeError("Unknown configurations type: '{}'".format(
                type(configs)))

    @staticmethod
    def create_default_configs(configs_filename, data_location):
        global __configsfile__
        global __datalocationfile__

        import os
        if os.path.exists(configs_filename):
            raise RuntimeError("Configuration file '{}' "+\
                    "already exists!\n".format(configs_filename))

        # TODO Remove assumptions about the hardware
        # I.E: Memory size, # NUMA nodes, Cache Size
        configs = __default_config__.copy() # shallow copy
        if not os.path.exists(data_location):
            os.makedirs(data_location)

        configs["root_conf"] = data_location
        __datalocationfile__ = data_location

        __configsfile__ = configs_filename

        Configuration.__write_configs(configs)

    # Not yet fool proof
    @staticmethod
    def get_configs(configs_filename=__configsfile__):
        import os
        if not os.path.exists(configs_filename):
            raise RuntimeError("Configuration file '{}' "+\
                    "not found!\n".format(configs_filename))

        config_map = {}
        with open(configs_filename, "rb") as file:
            data = file.readlines()
            # for line in map((lambda x : x.encode("UTF-8")), data):
            for line in data:
                if line:
                    cleaned_line = "".join(line.strip().split())
                    params = cleaned_line.split("=")
                    if len(params):
                        config_map[params[0]] = \
                                "" if len(params) < 2 else params[1]
        return config_map

    # TODO: We can cache this and update as necessary
    @staticmethod
    def get_config(config):
        import os
        if not __configsfile__:
            raise RuntimeError("Configuration file not set!\n")
        if not os.path.exists(__configsfile__):
            raise RuntimeError("Configuration file not in local file system!\n")

        # Can throw KeyError
        return Configuration.get_configs(__configsfile__)[config]

    # TODO: We should do this iteratively
    @staticmethod
    def set_config(config, config_value):
        import os
        if not __configsfile__:
            raise RuntimeError("Configuration file not set!\n")
        if not os.path.exists(__configsfile__):
            raise RuntimeError("Configuration file not in local file system!\n")

        configs = Configuration.get_configs(__configsfile__)
        configs[config] = config_value
        Configuration.__write_configs(configs)

    @staticmethod
    def verify_configs(configs=None):
        if configs is None:
            config_map = Configuration.get_configs(__configsfile__)
        else:
            config_map = configs

        for key in config_map:
            if key in __default_config__: # first check if it exists
                if type(config_map[key]) != type(__default_config__[key]):
                    raise RuntimeError("Config '{}' incorrect value type! Got: '{}' expected '{}'\n".format(key,
                            type(config_map[key]),
                            type(__default_config__[key])))

                if type(config_map[key]) != type(__default_config__[key]):
                    raise RuntimeError("Config error incorrect value type for key '{}'\n".format(key))
