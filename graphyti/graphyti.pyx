# distutils: language = c++
# cython: language_level=2

from libcpp.vector cimport vector
from libcpp.pair cimport pair
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

from Exceptions.runtime import UnsupportedError

from os.path import abspath
from os.path import exists
from os.path import join as pjoin
import psutil

# Metadata
__version__ = "0.0.1"
__author__ = "Disa Mhembere, Da Zheng"
__maintainer__ = "Disa Mhembere <disa@cs.jhu.edu>"
__package__ = "graphyti"

ctypedef unsigned vertex_id_t
ctypedef pair[vertex_id_t, size_t] tk_t

cdef extern from "src/flash-graph/bindings/CGraph.h" namespace "fg":
    cdef cppclass CGraph:
        CGraph() except +
        # TODO: Get this to work for files in SAFS
        CGraph(string graph_file, string index_file,
                string config_file) except +
        vector[size_t] coreness(const size_t kmax, const size_t kmin)
        vector[float] betweenness(vector[vertex_id_t]& ids)
        vector[vertex_id_t] degree(const string& etype)
        vector[size_t] triangles(bool cycles_only)
        vector[size_t] local_scan(const short num_hops)
        vector[tk_t] topk_scan(const size_t k)
        vector[float] pagerank(const int num_iters,
                const float damping_factor, const string& algo)
        vector[vertex_id_t] weakly_connected_components(const bool sync)
        vector[vertex_id_t] connected_components()
        vector[vertex_id_t] strongly_connected_components()
        size_t bfs_vcount(vertex_id_t start_vertex, const string edge_type_str)
        size_t diameter(const unsigned num_para_bfs, bool directed)

        # Utils
        const vertex_id_t vcount()
        const size_t ecount() const
        const bool is_directed() const
        const bool is_in_mem() const

# cdef extern from "src/flash-graph/utils/el2fg.h" namespace "fg::utils":
    # cpdef el2fg(vector[string] extfns, string safs_adj_fn,
            # string safs_index_fn, bool directed, int nthread)

cdef class Util:
    @staticmethod
    def edge2graphyti(edgeloc, outadj, outidx, directed=True,
            nthread=psutil.cpu_count(), to_safs=False):

        if not isinstance (edgeloc, list):
            edgeloc = [edgeloc]

        for fn in edgeloc:
            if not os.path.exists(edgeloc):
                raise RuntimeError("Cannot find input file: '{}'\n".format(fn))

        # FIXME: Make run
        # el2fg(list(map(string, edgeloc)), outadj, outidx, directed, nthread)

        if to_safs:
            pass # TODO: Move and ingest

# TODO: Wrap SAFS-util for ingestion
# TODO: Interactive configuration file creation for safs & storage of it

def ba(s: str) -> bytearray:
    return bytearray(s, "utf8")

cdef class Graph:
    cdef CGraph cg

    def __cinit__(self, graph_file, index_file, config_file):
        self.cg = CGraph(ba(graph_file), ba(index_file), ba(config_file))

    def coreness(self, kmax=0, kmin=0):
        return self.cg.coreness(kmax, kmin)

    def betweenness(self, ids=[]):
        return self.cg.betweenness(ids)

    def degree(self, edge_type="both"):
        return self.cg.degree(ba(edge_type))

    def triangles(self, cycles_only=False):
        return self.cg.triangles(cycles_only)

    def local_scan(self, num_hops=1):
        return self.cg.local_scan(num_hops)

    def topk_scan(self, k):
        return self.cg.topk_scan(k)

    def diameter(self, num_para_bfs=2, directed=True):
        return self.cg.diameter(num_para_bfs, directed)

    def weakly_connected_components(self, sync=False):
        return self.cg.weakly_connected_components(sync)

    def connected_components(self):
        return self.cg.connected_components()

    def bfs_vcount(self, start_vertex=None, edge_type_str="both"):
        return self.cg.bfs_vcount(start_vertex, edge_type_str)

    def vcount(self):
        return self.cg.vcount()

    def ecount(self):
        return self.cg.ecount()

    def is_in_mem(self):
        return self.cg.is_in_mem()

    def is_directed(self):
        return self.cg.is_directed()

    def __str__(self):
        return "Graphyti Graph:\n {}, {}\nv: {}, e: {}\n".format(
                ("Directed" if self.directed() else "Undirected"),
                    ("In-mem" if self.is_in_mem() else "On-Disk"),
                    self.vcount(), self.ecount())
