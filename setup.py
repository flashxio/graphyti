import os, sys, re

from glob import glob
from distutils.command.build_clib import build_clib
from distutils.errors import DistutilsSetupError
from Cython.Distutils import build_ext
from distutils.core import setup, Extension
from Cython.Build import cythonize
#from utils import find_header_loc
dir_path = os.path.dirname(os.path.realpath(__file__))

_REPO_ISSUES_ = "https://github.com/flashxio/graphyti/issues"
_OS_SUPPORTED_ = {"linux":"linux", "mac":"darwin"}

patts = []
for opsys in _OS_SUPPORTED_.values():
    patts.append(re.compile("(.*)("+opsys+")(.*)"))

raw_os = sys.platform.lower()

OS = None
for patt in patts:
    res = re.match(patt, raw_os)
    if res is not None:
        OS = res.groups()[1]
        break

if OS not in list(_OS_SUPPORTED_.values()):
    raise RuntimeError("Operating system {}\n." +\
            "Please post an issue at {}\n".format(raw_os, _REPO_ISSUES_))

# Hack to stop -Wstrict-prototypes warning on linux
if OS == _OS_SUPPORTED_["linux"]:
    from distutils import sysconfig

    sysconfig._config_vars["OPT"] = \
            sysconfig._config_vars["OPT"].replace("-Wstrict-prototypes", "")
    sysconfig._config_vars["OPT"] = \
            sysconfig._config_vars["OPT"].replace("-O2", "-O3")
    (os.environ["OPT"],) = sysconfig.get_config_vars("OPT")


# For C++ libraries
libsafs = ("safs",
        {"sources": glob(os.path.join("graphyti", "src", "libsafs", "*.cpp"))})
libgraph = ("graph",
        {"sources": glob(os.path.join("graphyti", "src", "flash-graph", "*.cpp"))})
libgraph_algs = ("graph-algs",
        {"sources": glob(os.path.join("graphyti", "src", "flash-graph",
        "libgraph-algs", "*.cpp"))})

libraries = [libsafs, libgraph, libgraph_algs]
sources = [os.path.join("graphyti", "graphyti.pyx")]

extra_compile_args = ["-std=c++11", "-O3", "-fPIC", "-Wno-attributes",
        "-Wno-unused-variable", "-Wno-unused-function", "-fopenmp",
        "-I.", "-Igraphyti",
        "-Igraphyti/src/libsafs", "-Igraphyti/src/flash-graph",
        "-Igraphyti/src/flash-graph/libgraph-algs",
        "-Igraphyti/src/flash-graph/bindings", "-DUSE_NUMA"] # -DNDEBUG

extra_link_args =  [
        "-Lgraphyti/src/flash-graph/libgraph-algs", "-lgraph-algs",
        "-Lgraphyti/src/flash-graph", "-lgraph",
            "-Lgraphyti/src/libsafs", "-lsafs",
            "-lpthread", "-lnuma", "-lcblas",
            "-rdynamic", "-lrt", "-mavx", "-fopenmp"]


# Build cython modules
ext_modules = cythonize(Extension(
    "graphyti.graphyti",                                # the extension name
    sources=sources,
    language="c++",
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args))

class graphyti_clib(build_clib, object):
    def initialize_options(self):
        super(graphyti_clib, self).initialize_options()
        self.include_dirs = [
                os.path.join("graphyti", "src", "libsafs"),
                os.path.join("graphyti", "src", "flash-graph"),
                os.path.join("graphyti", "src", "flash-graph", "libgraph-algs"),
                os.path.join("graphyti", "src", "flash-graph", "bindings") ]
        #self.include_dirs.append(find_header_loc("numpy"))
        self.define = [ ("USE_NUMA", None) ]

    def build_libraries(self, libraries):
        for (lib_name, build_info) in libraries:
            sources = build_info.get("sources")
            if sources is None or not isinstance(sources, (list, tuple)):
                raise DistutilsSetupError(("in \"libraries\" option " +
                    "(library \"%s\"), \"sources\" must be present and must" +
                    "be a list of source filenames") % lib_name)
            sources = list(sources)

            print("building \"%s\" library" % lib_name)

            macros = build_info.get("macros")
            include_dirs = build_info.get("include_dirs")

            # pass flags to compiler
            extra_preargs = ["-std=c++11", "-Wno-unused-function"]
            extra_preargs.append("-fopenmp")

            objects = self.compiler.compile(sources,
                    output_dir=self.build_temp,
                    macros=macros,
                    include_dirs=include_dirs,
                    debug=self.debug,
                    extra_preargs=extra_preargs)

            self.compiler.create_static_lib(objects, lib_name,
                    output_dir=self.build_clib,
                    debug=self.debug)

setup(
    name="graphyti",
    version="0.0.1",
    description="A parallel and scalable graph library built on FlashGraph",
    long_description="FlashGraph Graphyti scales graph operations beyond" +\
            "memory through out-of-core processing with SSDs",
    url="https://github.com/flashxio/graphyti",
    author="Disa Mhembere, Da Zheng",
    author_email="disa@cs.jhu.edu",
    license="Apache License, Version 2.0",
    keywords="graph parallel scalable machine-learning",
    install_requires=[
        "numpy",
        "Cython>=0.26",
        "cython>=0.26",
        ],
    package_dir = {"graphyti": "graphyti"},
    packages=["graphyti", "graphyti.Exceptions"],
    libraries =libraries,
    cmdclass = {'build_clib': graphyti_clib, 'build_ext': build_ext},
    ext_modules = ext_modules,
)
