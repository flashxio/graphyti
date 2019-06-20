#!/usr/bin/python3

from graphyti import graphyti as gt

### Configure the I/O system
c = Configuration.create_default_configs() # Accept defaults

### Load up a graph into the I/O system

fm = gt.FileManager(c["configs"]) # Pass the file manager the configuration file

adj_fn = "test-u.adj"
idx_fn = "test-u.idx"

fm.load(adj_fn, adj_fn) # Assumes `test-u.adj` is in the current working dir
fm.load(idx_fn, idx_fn) # Assumes `test-u.idx` is in the current working dir

### Verify the `test-u.adj` is loaded into I/O system
print(fm.list())

### Create a graph object (either in memory or in semi-external memory)
g = gt.Graph(adj_fn, test-u.idx, c["configs"]) # Graphyti searches SAFS first, then the local filesystem for the file

### String representation of the graph
print(g)

### Compute the degree of each node
deg = g.degree()
print("The degrees of nodes in the graph: {}".format(deg))

### Compute the betweenness centrality
import numpy as np

start_vertex = np.random.randint(g.get_min_vertex_id(), g.get_max_vertex_id())
bc = g.betweenness()
print("The betweenness centrality of nodes in the graph: {}".format(bc))
