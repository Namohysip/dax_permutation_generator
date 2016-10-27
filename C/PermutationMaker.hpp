#pragma once

void combine(igraph_t * , igraph_integer_t, igraph_integer_t);
bool addWithoutDuplicates( std::vector<igraph_t *> * , igraph_t *);
std::vector<igraph_t *> * exhaustivePermStart(igraph_t * graph, bool timed , double seconds , int goal );
igraph_t * getImported();
void outputDAX(std::vector<igraph_t *> *, std::string);
std::vector<std::string> * exhaustivePermHashStart(igraph_t * graph,  std::string fileBase, double seconds, int goal);
std::vector<igraph_t *> * randomizedPerm(igraph_t * graph, double time, int max, std::string fileBase);
int RandomizedPermEvenSpread(igraph_t * graph, int maxPerLevel, std::string fileBase);

