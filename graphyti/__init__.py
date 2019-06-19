import sys
PYTHON_VERSION = sys.version_info[0]

# TODO: Import Exceptions
# TODO: Import utils -- from .graphyti import Util

if PYTHON_VERSION == 2:
    from graphyti import Graph
    from Configuration import Configuration
else:
    from .graphyti import Graph
    from .Configuration import Configuration
