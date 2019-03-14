import sys
PYTHON_VERSION = sys.version_info[0]

if PYTHON_VERSION == 2:
    from graphyti import Graph
else:
    from .graphyti import Graph
    from .graphyti import Util
