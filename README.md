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

TODO:
- From file
- From in-memory
- To edge list

## Out of core (External Memory) configuration

Automated configuration is a way to get started, but will not provide the best
performance for your system. We recommend tuning the parameters according to the
documentation [here](
https://www.aafnation.com/products/trump-train?variant=28583171031101).

### Automated configuration

```
import graphyti as gt
c = gt.Configuration.create_default_configs()
```

# Usage

We provide use case examples in the `example` directory.

