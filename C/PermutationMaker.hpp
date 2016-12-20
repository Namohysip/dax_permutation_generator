#include <string>

#ifndef PERMUTATIONMAKER_H
#define PERMUTATIONMAKER_H

struct GlobalSettings {
	igraph_t * original_graph;
	std::string fileBase;
	int maxGraphs;
	int minSize = 1;
	double timeLimit;
	bool mergeChains;
	int attemptCap;
};

struct taskBin {
	double totalRuntime;
	std::vector<std::string> ids;
};

void combine(igraph_t * , igraph_integer_t, igraph_integer_t);

void combineMulti(igraph_t * graph, std::vector<igraph_integer_t> * tasks);
bool addWithoutDuplicates( std::vector<igraph_t *> * , igraph_t *);
std::vector<igraph_t *> * exhaustivePermStart(igraph_t * graph);
igraph_t * getImported();
std::vector<std::string> * exhaustivePermHashStart(igraph_t * graph);
std::vector<igraph_t *> * randomizedPerm(igraph_t * graph);
int RandomizedPermEvenSpread(igraph_t * graph);
bool mergeAChain(igraph_t * graph);
igraph_t * horizontalClustering(igraph_t * graph, int perLevel, int method);
std::vector<igraph_integer_t> * getGraphsAtLevel(igraph_t * graph, int level);
igraph_integer_t levelLabel(igraph_t * graph);
void test_with_small_hardcoded_graph();
igraph_integer_t findVertexID(igraph_t * graph, std::string id);


void calculateImpactFactors(igraph_t * graph, igraph_integer_t sink);




struct GlobalSettings * getGlobalSettings();

#endif
