import sys
PYTHON_VERSION = sys.version_info[0]

if PYTHON_VERSION == 2:
    from graphyti import Graph
    from graphyti import FileManager
    from Configuration import Configuration
    from graphyti import Format
    from graphyti import __version__
    from Exceptions.runtime import *
else:
    from .graphyti import Graph
    from .graphyti import FileManager
    from .Configuration import Configuration
    from .graphyti import Format
    from .graphyti import __version__
    from .Exceptions.runtime import *
