#!/usr/bin/env python

# Copyright 2017 neurodata (http://neurodata.io/)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0 #
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Created by Disa Mhembere on 2017-07-17.
# Email: disa@jhu.edu

import sys
import os
from os.path import abspath
from subprocess import check_output

PYTHON_VERSION = sys.version_info[0]


def dir_contains_ext(dirname, filext):
    for fn in os.listdir(dirname):
        split = os.path.splitext(fn)
        if len(split) > 1 and split[1] in filext:
            return True
    return False

def find_header_loc(library, filext=[".h", ".hpp", ".hxx"]):
    for name in sys.path:
        if os.path.isdir(name):
            # print "Checking dir {}".format(name)
            cmd = "find {} -name {}".format(
                    abspath(os.path.join(name, "*")), library)
            out = check_output(cmd, shell=True)

            if PYTHON_VERSION == 2:
                dirs = map((lambda s : s.strip()), out.split("\n"))
            else:
                dirs = list(map((lambda s : s.strip()),
                    out.decode().split("\n")))

            for dirname in dirs:
                if dirname and dir_contains_ext(dirname, filext):
                    return os.path.dirname(os.path.abspath(dirname))

    return ""
