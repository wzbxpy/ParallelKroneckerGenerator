# Kronecker graph generator
Support generate dense graph. 
We notice that the generator provided by graph 500 do not check the collide edges. When graph is dense, there may exist massive collide edges.
However, straightforward resample the collide edges would suffer from lower possibility to hit the unsampled edges.
Thus, we reimplement the kronecker graph generator to support dense graphs following the idea of [**PSKG**](http://snap.stanford.edu/class/cs224w-2012/projects/cs224w-035-final.v01.pdf). 

## Implementation details 
Rather than generating the edges one by one, we first generate the degree of each vertex using poisson distribution, then generate the neighbors of each vertex.
### Degree generation
Since the generated degree for some vertices may large than the total number of vertices, we redistribute the overflow neighbors to other vertices by another round of poisson process. We repate it until all neighbors have been distributed.
### Neighbor generation 
We consider following situation and design corresponding algorithm
1. Full neighbors, i.e., degree = number of vertices: This means that all neighbors exist, and we directly generate all neighbors.
2. Dense neighbors, i.e., degree > number of vertices / constant value: Since the neighbor is dense, using reject sampling facing lower hit possibility. So we first sample some neighbors, then re-calculate the possibility of rest neighbors and sample with the new possibility.
3. Sparse neighbors, i.e., degree < number of vertices / constant value: directly sample the neighbors, when facing sampled neighbors, reinsert it.

## usage
build:
    g++ Generator.cpp -O3 -o Generator

run:
    ./Generator #path #logVertexNum #averageDegree #threadNums #seed
    ./Generator ./example/ 2 2 1 0

