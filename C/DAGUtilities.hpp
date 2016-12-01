#ifndef DAGUTILITIES_H
#define DAGUTILITIES_H

void printEdges(igraph_t*);
void printNodes(igraph_t*);
igraph_t * getImported();
void printNodesWithLevelsAndIF(igraph_t * graph);
#endif
