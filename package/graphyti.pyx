# distutils: language = c++
# cython: language_level=3

from libcpp.vector cimport vector
from libcpp.string cimport string
from libc.stdint cimport intptr_t
from libcpp.memory cimport unique_ptr
from libcpp.memory cimport shared_ptr
from libcpp cimport bool

from cython.operator cimport dereference as deref
import numpy as np
cimport numpy as np
import ctypes
import os, sys
PYTHON_VERSION = sys.version_info[0]

if PYTHON_VERSION == 2:
    from exceptions import NotImplementedError, RuntimeError

# from Exceptions.runtime import UnsupportedError

from os.path import abspath
from os.path import exists
from os.path import join as pjoin

# Metadata
__version__ = "0.0.1"
__author__ = "Disa Mhembere, Da Zheng"
__maintainer__ = "Disa Mhembere <disa@jhu.edu>"
__package__ = "graphyti"


cdef extern from "src/flash-graph/bindings/CGraph.h" namespace "fg":
    cdef cppclass CGraph:
        CGraph() except +
        CGraph(const string& graph_file, const string& index_file,
                const string& config_file) except +
        vector[size_t] coreness(const size_t kmax, const size_t kmin)

cdef class Graph:
    cdef CGraph cg

    def __cinit__(self, const string& graph_file, const string& index_file,
            const string& config_file):
        self.cg = CGraph(graph_file, index_file, config_file)

    def coreness(self, kmax=0, kmin=0):
        return self.cg.coreness(kmax, kmin)

    def const vertex_id_t vcount():
        return self.cg.vcount()
