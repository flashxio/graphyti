#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>

#include "src/flash-graph/bindings/CGraph.h"
#include "src/utils/FileManager.h"
#include "src/flash-graph/utils/el2fg.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

namespace py = pybind11;

std::string get_home() {
    struct passwd *pw = getpwuid(getuid());
    return std::string(pw->pw_dir);
}

// Helpers for scripting tasks -- cleaner in python
static std::string get_basename(const std::string& s) {
    py::object os_path = py::module::import("os.path");

    return os_path.attr("basename")(s).cast<std::string>();
}

static std::vector<std::string> splitext(const std::string& s) {
    py::object os_path = py::module::import("os.path");

    return os_path.attr("splitext")(s).cast<std::vector<std::string> >();
}

static void delete_file(const std::string& s) {
    py::object os = py::module::import("os");
    os.attr("remove")(s);
}

class Format {
    private:
    std::string configs;

    public:
    Format(const std::string& configs): configs(configs) {
    }

    void set_configs(const std::string& configs) {
        this->configs = configs;
    }

    void edge2graphyti(std::vector<std::string> edgelists, std::string adj_fn,
            std::string index_fn, bool directed, int nthread) {
        fg::utils::el2fg(edgelists, adj_fn, index_fn, directed, nthread);
    }

    std::pair<std::string, std::string> load(
            std::string edgelist, bool directed, int nthread) {
        if (edgelist.empty())
            return std::pair<std::string, std::string>("", "");

        if (configs.empty())
            throw std::runtime_error("Configuration file must be set first");

        std::string bn = splitext(get_basename(edgelist))[0];
        std::string adj_fn = bn+std::string(".adj");
        std::string idx_fn = bn+std::string(".idx");

        std::vector<std::string> els { edgelist };

        fg::utils::el2fg(els, adj_fn, idx_fn, directed);

        fg::FileManager fm(configs);

        // Load the adjacency list and index file
        fm.to_ex_mem(adj_fn, adj_fn);
        fm.to_ex_mem(idx_fn, idx_fn);

        // Delete the temps
        delete_file(adj_fn);
        delete_file(idx_fn);

        // Delete the temps
        return std::pair<std::string, std::string>(adj_fn, idx_fn);
    }
};

PYBIND11_MODULE(graphyti, m) {
    m.doc() = R"pbdoc(
         Python graphyti API
         ---------------------------

         .. currentmodule:: graphyti

         .. autosummary::
            :toctree: _generate

     )pbdoc";
        // Graph
        py::class_<fg::CGraph>(m, "Graph")
        .def(py::init(), "Create a Graph object"
                , py::return_value_policy::reference)

        .def(py::init<std::string, std::string, std::string>(),
            "Create a Graph object given adj list file, index file and "
            "configuration file",  py::return_value_policy::reference)
        /*Algorithms*/
        /* Coreness */
        .def("coreness", &fg::CGraph::coreness,
                R"pbdoc(
        Compute the k-core/coreness of a graph. The algorithm will
        determine which vertices are between core `kmin` and `kmax` --
        all other vertices will be assigned to core 0.

        Optional arguments:
        -------------------
        kmin:
            - The minimum core value to be computed.
        kmax:
            - (Optional) The kmax value. If omitted then all cores are
            computed i.e., coreness. *This is not recommended for very large
            graphs.*

        Returns:
        --------
         A list containing the core of each vertex between `kmin`
         and `kmax`. All other vertices are assigned to core 0.
                )pbdoc",
                py::arg("kmax")=0, py::arg("kmin")=0)

        /* Betweenness centrality */
        .def("betweenness", &fg::CGraph::betweenness,
        R"pbdoc(
        The betweenness centrality of the graph.
        This measure of centrality is based on the shortest paths within which a
        vertex participates.

        Optional arguments:
        -------------------
        ids:
            - The vertex IDs for which BC should be computed

        Returns:
        --------
        A list with an entry for each vertex in the graph's
          betweenness centrality value.
        )pbdoc",
                py::arg("ids")=std::vector<fg::vertex_id_t>())

        /* Closeness centrality */
        .def("closeness", &fg::CGraph::closeness,
        R"pbdoc(
        The closeness centrality of a graph is a measure of a nodes average
        farness (in terms of links traversed, that may be weighted) of all
        other nodes in the graph.

        ids:
            - The vertex IDs for which closeness centrality should be computed
        edge_type:
            - The edge type: "in", "out" or "both"

        Returns:
        --------
        A list with an entry for each vertex in the graph's
          closeness centrality value.
        )pbdoc",
                py::arg("ids")=std::vector<fg::vertex_id_t>(),
                py::arg("edge_type")="both")

        /* Diversity */
        .def("diversity", &fg::CGraph::diversity,
        R"pbdoc(
        The diversity of a vertex is defined as the (scaled)
        Shannon entropy of the weights of its incident edges.

        Optional arguments:
        -------------------
        edge_type:
            - The edge type: "in", "out" or "both"
        memopt:
            - Optimize for minimal memory rather than performance.

        Returns:
        --------
        A list with an entry for each vertex in the graph's diversity value.
        )pbdoc",
                py::arg("edge_type")="out", py::arg("memopt")=false)

        /* Topological Sort */
        .def("toposort", &fg::CGraph::topo_sort,
        R"pbdoc(
        Compute one (of possibly many) topological sortings of the graph.
        Given a directed acyclic graph (DAG) there is one sorting without
        ambiguity. Non-DAG Graphs will produce a valid sorting
        non-deterministically.

        Optional arguments:
        -------------------
        approx:
            - Compute a (faster) sorting

        Returns:
        --------
        A list of topologically sorted vertex IDs
        )pbdoc",
        py::arg("approx")=true)

        /* Louvain Modularity */
        .def("louvain", &fg::CGraph::louvain,
        R"pbdoc(
        Compute louvain agglomerative hiearchical clustering algorithm.
        At each `level` clusters merge to those with the highest degree of
        modularity change

        Optional arguments:
        -------------------
        nlevels:
            - The number of hierarchical levels of louvain to perform

        Returns:
        --------
        A numeric list with the cluster assignments for each vertex
        )pbdoc",
                py::arg("nlevels")=4)

        /* Degree */
        .def("degree", &fg::CGraph::degree,
        R"pbdoc(
        The degree of vertices in the graph is the number edges incident to
        a vertex.

        Optional arguments:
        -------------------
        edge_type:
            - The edge type: "in", "out" or "both"
        memopt:
            - Optimize for minimal memory rather than performance.

        Returns:
        --------
        A list with an entry for each vertex in the graph's diversity value.
        )pbdoc",
                py::arg("edge_type")="both", py::return_value_policy::copy)

        /* Triangles */
        .def("triangles", &fg::CGraph::triangles,
        R"pbdoc(
        Triangle count of each vertex. Counts the number 3-cliques that exist
        within the graph. Used as a measure of importance for vertices that have
        large numbers of triangles with which they are associated.

        CYCLE triangles are defined for directed graphs and
                depend on the direction of each edge. All edges must be
                head to tail connections.
            E.g A -----> B
                ^     /
                |   /
                | v
                C

        ALL triangles. Edge direction is disregarded.
            E.g A ----- B
                |     /
                |   /
                | /
                C

        Optional arguments:
        -------------------
        cycles_only:
            - Count only cycle triangles

        Returns:
        --------
        A list containing the triangle count of each vertex in the graph
        )pbdoc",
                py::arg("cycles_only")=false)

        /* Local Scan */
        .def("local_scan", &fg::CGraph::local_scan,
        R"pbdoc(
        Scan statistic is a measure of non-homogeniety is the nodes of a graph.
        Nodes with larger scan statistic appear more different than others.

        Optional arguments:
        -------------------
        num_hops:
            - The neighborhood around a vertex within which one must look

        Returns:
        --------
        A list containing the scan statistic of each vertex in the graph
        )pbdoc",
                py::arg("num_hops")=1)

        /* Top k Scan */
        .def("topk_scan", &fg::CGraph::topk_scan,
        R"pbdoc(
        Compute the top `k` scan statistics valued vertices. See help for
        `local_scan` for more details.

        Positional arguments:
        -------------------
        k:
            - The top number of vertices

        Returns:
        --------
        A list of 2-tuples with <vertex-ID, scan-value> of the top k scan
        statistic values
        )pbdoc",
        py::arg("k"))

        /* Diameter */
        .def("diameter", &fg::CGraph::diameter,
        R"pbdoc(
        Compute an estimate of the graph diameter. Diameter is a measure of the
        extent of a graph from one peripheral vertex to another

        Optional arguments:
        -------------------
        num_para_bfs:
            - The number of concurrent parallel sweeps to perform
        directed:
            - Are the sweep directed?
        Returns:
        --------
        The estimated graph diameter
        )pbdoc",
                py::arg("num_para_bfs")=2, py::arg("directed")=true)

        /* PageRank*/
        .def("pagerank", &fg::CGraph::pagerank,
        R"pbdoc(
        Compute the PageRank of vertices in the graph. Pagerank is a measure of
        importance of a vertex based upon the number of high ranking vertices
        with which it shares edges.
        See: http://ilpubs.stanford.edu:8090/422/1/1999-66.pdf

        Optional arguments:
        -------------------
        niters:
            - The number of iterations to perform
        damping_factor:
            - Used as probability of a user continuing to browse and jumping to
            a random page
        algo:
            - Either "push" or "pull" determines whether nodes broadcast their
            PageRank values (push) or nodes request PageRank values of their
            neighbors (pull). Default is "push" and is faster

        Returns:
        --------
        A list containing the PageRank of each vertex
        )pbdoc",
                py::arg("niters")=30, py::arg("damping_factor")=.85,
                py::arg("algo")="push")

        /* Weakly connected components*/
        .def("weakly_connected_components",
                &fg::CGraph::weakly_connected_components,
        R"pbdoc(
        Weakly connected components is the maximal subgraph of a directed graph
        such that for every pair of vertices  u, v in the subgraph, there is an
        undirected path from u to v and a directed path from v to u.

        Optional arguments:
        -------------------
        sync:
            - Perform algorithm synchronously or asynchronously (faster)

        Returns:
        --------
        A list of vertex IDs comprising the weakly connected components
        )pbdoc",
                py::arg("sync")=false)

        /* Connected components */
        .def("connected_components", &fg::CGraph::connected_components,
        R"pbdoc(
        The connected components are formed by subgraphs in which any two
        vertices are connected to each other by a path. Non path connected
        vertices are placed in other connected components.

        Returns:
        --------
        A list containing the component ID of each vertex
        )pbdoc")

        /*  Strongly connected components*/
        .def("strongly_connected_components",
                &fg::CGraph::strongly_connected_components,
        R"pbdoc(
        The strongly connected components of a directed graph form partitions
        of subgraphs that are connected via either in or out edges.

        Returns:
        --------
        A list of vertex IDs comprising the strongly connected component
        )pbdoc")

        /* BFS */
        .def("bfs_vcount", &fg::CGraph::bfs_vcount,
        R"pbdoc(
        Perform Breadth First traversal and count the number of vertices
        visited in the sweep.

        Positional arguments:
        --------------------
        start_vertex:
            - The vertex from which to begin the BFS sweep

        Optional arguments:
        -------------------
        edge_type:
            - The edge type: "in", "out" or "both"

        Returns:
        --------
        The BFS count of vertices visited
        )pbdoc",
                py::arg("start_vertex"), py::arg("edge_type")="both")

        .def("__repr__", &fg::CGraph::to_str,
        R"pbdoc(
        String representation of the graph
        )pbdoc")
        .def("get_min_vertex_id", &fg::CGraph::min_id,
        R"pbdoc(
        Get the minimum vertex ID of the graph
        )pbdoc")
        .def("get_max_vertex_id", &fg::CGraph::max_id,
        R"pbdoc(
        Get the maximum vertex ID of the graph
        )pbdoc")
        .def("vcount", &fg::CGraph::vcount,
        R"pbdoc(
        The number of vertices in the graph
        )pbdoc")
        .def("ecount", &fg::CGraph::ecount,
        R"pbdoc(
        The number of edges in the graph
        )pbdoc")
        .def("vcount", &fg::CGraph::vcount,
        R"pbdoc(
        The number of vertices in the graph
        )pbdoc")
        .def("is_directed", &fg::CGraph::is_directed,
        R"pbdoc(
        Determine if the graph directed
        )pbdoc")
        .def("is_in_mem", &fg::CGraph::is_in_mem,
        R"pbdoc(
        Determine if the graph is in memory
        )pbdoc");

    // File Manager
        py::class_<fg::FileManager>(m, "FileManager")
        .def(py::init<const std::string&>(),
        R"pbdoc(
        Create a File Manager object

        Positional arguments:
        --------------------
        config_file:
            - The location of the configuration file on disk
        )pbdoc",
                py::arg("config_file")="",
                py::return_value_policy::reference)
        .def("delete", &fg::FileManager::delete_file,
        R"pbdoc(
        Delete a file from SAFS

        Positional arguments:
        --------------------
        filename:
            - The name of the file in SAFS
        )pbdoc",
        py::arg("filename"))
        .def("export", &fg::FileManager::from_ex_mem,
        R"pbdoc(
        Export files from SAFS to the local file system

        Positional arguments:
        --------------------
        local_fs_filename:
            - The path and file name requested in the local file system
        safs_fs_filename:
            - The filename in SAFS
        )pbdoc",
                py::arg("local_fs_filename"), py::arg("safs_filename"))
        .def("load", &fg::FileManager::to_ex_mem,
        R"pbdoc(
        Load files into SAFS"

        Positional arguments:
        --------------------
        safs_fs_filename:
            - The filename to be created in SAFS
        local_fs_filename:
            - The path and file name of the file in the local file system
        )pbdoc",
                py::arg("safs_filename"), py::arg("local_fs_filename"))
        .def("rename", &fg::FileManager::rename,
        R"pbdoc(
        Rename files in SAFS

        Positional arguments:
        --------------------
        filename:
            - The current file name within the SAFS
        new_filename:
            - The new file name within the SAFS
        )pbdoc",
                py::arg("filename"), py::arg("new_filename"))
        .def("list", &fg::FileManager::list_files,
        R"pbdoc(
        List all files in SAFS
        )pbdoc")
        .def("file_exists", &fg::FileManager::file_exists,
        R"pbdoc(
        Determine if the file exists in SAFS

        Positional arguments:
        --------------------
        filename:
            - The name of the file being queried
        )pbdoc",
                py::arg("filename")
        )
        .def("file_size", &fg::FileManager::file_size,
        R"pbdoc(
        Determine the size of the file in SAFS

        Positional arguments:
        --------------------
        filename:
            - The name of the file being queried
        )pbdoc",
                py::arg("filename")
        )
        .def("info", &fg::FileManager::info,
        R"pbdoc(
        Get info on a file

        Positional arguments:
        --------------------
        filename:
            - The name of the file being queried
        )pbdoc",
                py::arg("filename")
        )
        .def("__repr__", &fg::FileManager::to_str,
        R"pbdoc(
        String representation of the File Manager
        )pbdoc");

        py::class_<Format>(m, "Format")
            .def(py::init<const std::string&>(),
        R"pbdoc(
        Create a Format converter object

        Positional arguments:
        --------------------
        configs:
            - Configuration file name
        )pbdoc",
                py::arg("configs")="",
                py::return_value_policy::reference)
        .def("edge2graphyti", &Format::edge2graphyti,
        R"pbdoc(
        Convert edge list(s) to graphyti format

        Positional arguments:
        --------------------
        edgelists:
            - A list of files that contains the edge lists for the graph
                in plain text
        adj_fn:
            - The requested adjacency list file name in
        index_fn:
            - The requested index list file name
        )pbdoc",
                py::arg("edgelists"), py::arg("adj_fn"),
                py::arg("index_fn"),
                py::arg("directed")=true, py::arg("nthread")=4)
        .def("load", &Format::load,
        R"pbdoc(
        Convert edge list(s) to graphyti format and load into SAFS

        Positional arguments:
        --------------------
        edgelist:
            - An edge list file for a graph in plain text
        adj_fn:
            - The requested adjacency list file name in
        index_fn:
            - The requested index list file name
        )pbdoc",
                py::arg("edgelist"),
                py::arg("directed")=true, py::arg("nthread")=4);

    // Versioning information
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
