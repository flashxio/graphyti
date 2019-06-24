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
    return (os_path.attr("basename")(s)).cast<std::string>();
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

        std::string bn = get_basename(edgelist);
        std::string adj_fn = bn+std::string(".adj");
        std::string idx_fn = bn+std::string(".idx");

        std::vector<std::string> els { edgelist };

        //fg::utils::el2fg(els, adj_fn, idx_fn, directed,
            //nthread, NULL, false, true, false,
            //0, 0, "", ".");

        fg::utils::el2fg(els, adj_fn, idx_fn, directed);
        fg::FileManager fm(configs);

        //// Load the adjacency list and index file
        fm.to_ex_mem(adj_fn, adj_fn);
        fm.to_ex_mem(idx_fn, idx_fn);

        //// Delete the temps
        delete_file(adj_fn);
        delete_file(idx_fn);

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

        .def("vcount", &fg::CGraph::vcount, "The number of vertices in the graph")
        .def("ecount", &fg::CGraph::ecount, "The number of edges in the graph")
        .def("is_directed", &fg::CGraph::is_directed, "Is the graph directed")
        .def("is_in_mem", &fg::CGraph::is_in_mem, "Is the graph in memory")

        /*Algorithms*/
        .def("coreness", &fg::CGraph::coreness,
                "The coreness value / k-core decomposition of the graph.",
                py::arg("kmax")=0, py::arg("kmin")=0)
        .def("betweenness", &fg::CGraph::betweenness,
                "The betweenness centrality of the graph.",
                py::arg("ids")=std::vector<fg::vertex_id_t>())
        .def("degree", &fg::CGraph::degree,
                "The degree of vertices in the graph.",
                py::arg("edge_type")="both", py::return_value_policy::copy)
        .def("triangles", &fg::CGraph::triangles,
                "Triangle count of each vertex.",
                py::arg("cycles_only")=false)
        .def("local_scan", &fg::CGraph::local_scan,
                "Scan statistic.", \
                py::arg("num_hops")=1)
        .def("topk_scan", &fg::CGraph::topk_scan,
                "Top k vertices for scan statistic.")
        .def("diameter", &fg::CGraph::diameter,
                "Compute an estimate of the graph diameter",
                py::arg("num_para_bfs")=2, py::arg("directed")=true)
        .def("pagerank", &fg::CGraph::pagerank,
                "Compute the PageRank of vertices in the graph",
                py::arg("niters")=30, py::arg("damping_factor")=.85,
                py::arg("algo")="push")
        .def("weakly_connected_components",
                &fg::CGraph::weakly_connected_components,
                "Weakly connected components",
                py::arg("sync")=false)
        .def("connected_components",
                &fg::CGraph::connected_components,
                "Connected components")
        .def("strongly_connected_components",
                &fg::CGraph::strongly_connected_components,
                "Strongly Connected components")
        .def("bfs_vcount", &fg::CGraph::bfs_vcount,
                "Strongly Connected components",
                py::arg("start_vertex")=-1, py::arg("edge_type")="both")
        .def("__repr__", &fg::CGraph::to_str,
                "String representation of the graph")
        .def("get_min_vertex_id", &fg::CGraph::min_id,
                "Get the minimum vertex ID of the graph")
        .def("get_max_vertex_id", &fg::CGraph::max_id,
                "Get the maximum vertex ID of the graph");

    // File Manager
        py::class_<fg::FileManager>(m, "FileManager")
        .def(py::init<const std::string&>(), "Create a File Manager object"
                , py::return_value_policy::reference)
        .def("delete", &fg::FileManager::delete_file,
                "Delete a file from SAFS")
        .def("export", &fg::FileManager::from_ex_mem,
                "Export files from SAFS to the local file system",
                py::arg("local_fs_filename"), py::arg("safs_filename"))
        .def("load", &fg::FileManager::to_ex_mem,
                "Load files into SAFS",
                py::arg("safs_filename"), py::arg("local_fs_filename"))
        .def("rename", &fg::FileManager::rename,
                "Rename files in SAFS",
                py::arg("filename"), py::arg("new_filename"))
        .def("list", &fg::FileManager::list_files,
                "List all files in SAFS")
        .def("file_exists", &fg::FileManager::file_exists,
                "Determine if the file exists in SAFS")
        .def("file_size", &fg::FileManager::file_size,
                "Determine the size of the file in SAFS")
        .def("info", &fg::FileManager::info,
                "Get info on a file")
        .def("__repr__", &fg::FileManager::to_str,
                "String representation of the File Manager");

        py::class_<Format>(m, "Format")
            .def(py::init<const std::string&>(),
                "Create a Format converter object",
                py::arg("configs")="",
                py::return_value_policy::reference)
        .def("edge2graphyti", &Format::edge2graphyti,
                "Convert edge list(s) to graphyti format",
                py::arg("edgelists"), py::arg("adj_fn"),
                py::arg("index_fn"),
                py::arg("directed")=true, py::arg("nthread")=4)
        .def("load", &Format::load,
                "Convert edge list(s) to graphyti format and load into SAFS",
                py::arg("edgelist"),
                py::arg("directed")=true, py::arg("nthread")=4);

    // Versioning information
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
