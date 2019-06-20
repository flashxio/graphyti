import sys
PYTHON_VERSION = sys.version_info[0]

if PYTHON_VERSION == 2:
    from graphyti import Graph
    from graphyti import FileManager
    from Configuration import Configuration
    from Exceptions.runtime import *
else:
    from .graphyti import Graph
    from .graphyti import FileManager
    from .Configuration import Configuration
    from .Exceptions.runtime import *
