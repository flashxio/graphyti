#!/usr/bin/python3

import graphyti as gt

### Configure the I/O system (Only done once )
c = gt.Configuration.create_default_configs() # Accept defaults

### Load up a graph into the I/O system

fm = gt.FileManager(c["configs"]) # Pass the file manager the configuration file

### Convert the adjacency list file into a graphyti format and load into SAFS

f = gt.Format(c["configs"])
adj_fn, idx_fn = f.load("test-graph.txt") # Default file names

### One could also manually load the adjacency and index files
# Not run
# fm.load(adj_fn, adj_fn) # Assumes `test-d.adj` is in the current working dir
# fm.load(idx_fn, idx_fn) # Assumes `test-d.idx` is in the current working dir

### Verify the `test-u.adj` is loaded into I/O system
print(fm.list())

### Create a graph object (either in memory or in semi-external memory)
g = gt.Graph(adj_fn, idx_fn, c["configs"]) # Graphyti searches SAFS first, then the local filesystem for the file

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
print("Example successfully completes!")
