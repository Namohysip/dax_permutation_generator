#include <string>

#ifndef PERMUTATIONMAKER_H
#define PERMUTATIONMAKER_H

struct GlobalOptions {
	std::string fileBase;
	int maxGraphs;
	double timeLimit;
	bool mergeChains;
};

void combine(igraph_t * , igraph_integer_t, igraph_integer_t);
bool addWithoutDuplicates( std::vector<igraph_t *> * , igraph_t *);
std::vector<igraph_t *> * exhaustivePermStart(igraph_t * graph);
igraph_t * getImported();
std::vector<std::string> * exhaustivePermHashStart(igraph_t * graph);
std::vector<igraph_t *> * randomizedPerm(igraph_t * graph);
int RandomizedPermEvenSpread(igraph_t * graph, int maxPerLevel, int depthLimit, int attempt_cap);
bool mergeAChain(igraph_t * graph);
igraph_t * horizontalClustering(igraph_t * graph, int perLevel, int method);
std::vector<igraph_integer_t> * getGraphsAtLevel(igraph_t * graph, int level);
igraph_integer_t levelLabel(igraph_t * graph);

void calculateImpactFactors(igraph_t * graph, igraph_integer_t sink);




struct GlobalOptions * getConfig();

#endif
