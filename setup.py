import os, sys, re

from glob import glob
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.build_clib import build_clib
import setuptools

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

################################ End VarDecl ###################################

class get_pybind_include(object):
    """Helper class to determine the pybind11 include path

    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)

# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        try:
            compiler.compile([f.name], extra_postargs=[flagname])
        except setuptools.distutils.errors.CompileError:
            return False
    return True


def cpp_flag(compiler):
    """Return the -std=c++[11/14] compiler flag.

    The c++14 is prefered over c++11 (when it is available).
    """
    if has_flag(compiler, '-std=c++14'):
        return '-std=c++14'
    elif has_flag(compiler, '-std=c++11'):
        return '-std=c++11'
    else:
        raise RuntimeError('Unsupported compiler -- at least C++11 support '
                           'is needed!')
class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        'unix': [],
    }

    if sys.platform == 'darwin':
        c_opts['unix'] += ['-stdlib=libc++', '-mmacosx-version-min=10.7']

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])

        # Add Specific build dependencies
        # opts.append("-I/usr/include/boost")

        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
            opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, '-fvisibility=hidden'):
                opts.append('-fvisibility=hidden')
        elif ct == 'msvc':
            opts.append('/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version())
        for ext in self.extensions:
            ext.extra_compile_args = opts
        build_ext.build_extensions(self)

# For C++ libraries
libsafs = ("safs",
        {"sources": glob(os.path.join("graphyti", "src", "libsafs", "*.cpp"))})
libgraph = ("graph",
        {"sources": glob(os.path.join("graphyti", "src", "flash-graph", "*.cpp"))})
libgraph_algs = ("graph-algs",
        {"sources": glob(os.path.join("graphyti", "src", "flash-graph",
        "libgraph-algs", "*.cpp"))})

libraries = [libsafs, libgraph, libgraph_algs]

sources = [os.path.join("graphyti", "graphyti.cpp")]

_include_dirs = [".",
        "graphyti/src/flash-graph/bindings",
        "graphyti/src/flash-graph/libgraph-algs",
        "graphyti/src/flash-graph/utils",
        "graphyti/src/utils",
        "graphyti/src/flash-graph",
        "graphyti/src/flash-graph/graphlab",
        "graphyti/src/libsafs",
        ]

extra_compile_args = ["-std=c++11", "-fPIC", "-Wno-attributes",
        "-Wno-unused-variable", "-Wno-unused-function"
        ]

extra_compile_args.extend(map((lambda i : "-I"+i) , _include_dirs))
extra_compile_args.extend(["-fopenmp",
    "-DUSE_LIBAIO", "-DUSE_NUMA"])

extra_link_args = [
        "-Lgraphyti/src/flash-graph/libgraph-algs", "-lgraph-algs",
        "-Lgraphyti/src/flash-graph", "-lgraph",
        "-Lgraphyti/src/libsafs", "-lsafs", "-lrt", "-lz", "-lhwloc", "-laio",
         "-lnuma", "-lpthread", "-rdynamic", "-mavx", "-fopenmp"]

ext_modules = [
        Extension(
            "graphyti.graphyti",                        # the extension name
            sources=sources,
            language="c++",
            include_dirs=[
            # Path to pybind11 headers
            get_pybind_include(),
            get_pybind_include(user=True),
        ],

        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args),
]

ext_modules[0].include_dirs.extend(_include_dirs)

# Build libraries
class graphyti_clib(build_clib, object):
    def initialize_options(self):
        super(graphyti_clib, self).initialize_options()
        self.include_dirs = [
                os.path.join("graphyti", "src", "libsafs"),
                os.path.join("graphyti", "src", "flash-graph"),
                os.path.join("graphyti", "src", "flash-graph", "libgraph-algs"),
                os.path.join("graphyti", "src", "flash-graph", "bindings"),
                os.path.join("graphyti", "src", "flash-graph", "utils"),
                ]
        self.define = [ ("USE_LIBAIO", None),
                ("USE_NUMA", None) ]

    def build_libraries(self, libraries):
        for (lib_name, build_info) in libraries:
            sources = build_info.get("sources")
            if sources is None or not isinstance(sources, (list, tuple)):
                raise RuntimeError(("in \"libraries\" option " +
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
    version="0.0.1a",
    description="A parallel and scalable graph library built on FlashGraph",
    long_description="Graphyti scales graph operations beyond" +\
            "memory through out-of-core processing with SSDs using FlashGraph",
    url="https://github.com/flashxio/graphyti",
    author="Disa Mhembere, Da Zheng",
    author_email="disa@cs.jhu.edu",
    license="Apache License, Version 2.0",
    keywords="graph parallel scalable machine-learning",
    install_requires=[
        ],
    package_dir = {"graphyti": "graphyti"},
    packages=["graphyti", "graphyti.Exceptions"],
    libraries =libraries,
    cmdclass = {'build_clib': graphyti_clib, 'build_ext': BuildExt},
    ext_modules = ext_modules,
)
