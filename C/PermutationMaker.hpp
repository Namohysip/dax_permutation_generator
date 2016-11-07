#ifndef PERMUTATIONMAKER_H
#define PERMUTATIONMAKER_H

void combine(igraph_t * , igraph_integer_t, igraph_integer_t);
bool addWithoutDuplicates( std::vector<igraph_t *> * , igraph_t *);
std::vector<igraph_t *> * exhaustivePermStart(igraph_t * graph, std::string filebase, bool timed = false, double seconds = 0 , int goal = 0);
igraph_t * getImported();
std::vector<std::string> * exhaustivePermHashStart(igraph_t * graph,  std::string fileBase, double seconds, int goal);
std::vector<igraph_t *> * randomizedPerm(igraph_t * graph, double time, int max, std::string fileBase);
int RandomizedPermEvenSpread(igraph_t * graph, int maxPerLevel, int depthLimit, int attempt_cap, std::string fileBase);

#endif
