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
        .def(py::init(), "Create a Graph Object")
        .def(py::init<std::string, std::string, std::string>(),
                "Create a Graph Object given adj list file, index file and configurat    ion file")
        .def("vcount", &fg::CGraph::vcount, "The number of vertices in the graph")
        .def("ecount", &fg::CGraph::ecount, "The number of edges in the graph")
        .def("is_directed", &fg::CGraph::is_directed, "Is the graph directed")
        .def("is_in_mem", &fg::CGraph::is_in_mem, "Is the graph in memory");

    // Versioning information
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
