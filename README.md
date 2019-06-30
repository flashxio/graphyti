# graphyti

The *Graphyti* library is a collection graph algorithms optimized for
multicore Non-Uniform Memory Access Architectures (NUMA) and Semi-External
Memory. Graphyti is built on the [FlashGraph](https://github.com/flashxio/FlashX)
graph engine and uses a userspace file system called SAFS to perform I/O for
(semi)-external memory usage.

## Language

`graphyti` is only supported for **Python 3**

## Tested OSes

- Ubuntu LTS 16.04 and 18.04

## Python Dependencies

- `numpy`
- `pybind11`

## System Dependencies

The following are required for Linux:
- `libaio-dev libatlas-base-dev zlib1g-dev`
- `libnuma-dev libhwloc-dev`

## Installation

```
pip install graphyti
```

## Graph Conversion

Edge lists in plain text format can be converted to the FlashGraph format which
creates an adjacency list file and an index file that can then be ingested into
SAFS or used directed from the local filesystem for in-memory computation.
One can do so as follows:

```
import graphyti as gt

formatter = gt.Format("location/of/configuration/file")
formatter.edge2graphyti("location/of/edgelist.txt",
    "location/of/adjacencylist.adj", "location/of/index.idx")
```

## Out of core (External Memory) configuration

Automated configuration is a way to get started, but will not provide the best
performance for your system. We recommend tuning the parameters according to the
documentation [here](https://github.com/flashxio/FlashX/wiki/SAFS-user-manual).

### Automated configuration

```
import graphyti as gt
c = gt.Configuration.create_default_configs()
```

## Docker

To build and run Graphyti using docker we provide a `Dockerfile`. Simply:

```
docker build -t graphyti .
docker run -i -t graphyi
```

## Usage

We provide use case examples in the `example` directory.
