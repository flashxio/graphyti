#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>

#include "src/flash-graph/bindings/CGraph.h"

namespace py = pybind11;

PYBIND11_MODULE(graphyti, m) {
    m.doc() = R"pbdoc(
         Python graphyti API
         ---------------------------

         .. currentmodule:: graphyti

         .. autosummary::
            :toctree: _generate

     )pbdoc";

        // GMap
        py::class_<fg::CGraph>(m, "Graph")
        .def(py::init(), "Create a Graph object")
        .def(py::init<std::string, std::string, std::string>(),
            "Create a Graph object given adj list file, index file and "\
            "configuration file")
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
                py::arg("edge_type")="both")
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
        ;

    // Versioning information
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
