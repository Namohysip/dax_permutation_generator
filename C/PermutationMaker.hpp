#include <string>
#ifndef PERMUTATIONMAKER_H
#define PERMUTATIONMAKER_H

struct GlobalSettings {
	igraph_t * original_graph;
	std::string fileBase;
	int maxGraphs;
	int minSize = 1;
	double timeLimit;
	bool mergeChainsBefore;
	bool mergeChainsAfter;
	int attemptCap;
};

/*used to preemptively store tasks for clustering
 all at once rather than one by one. */
struct taskBin {
	double totalRuntime;
	std::vector<std::string> ids;
	double lastIFAdded = -1;
	igraph_integer_t lastAdded = -1;
};

/*Used to keep track of the distance from a graph. */
struct taskDistance {
	int distance = 0;
	igraph_integer_t id = -1;
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
igraph_t * horizontalClustering(igraph_t * graph, int perLevel, bool noBinRestrictions);
igraph_t * impactFactorClustering(igraph_t * graph, int perLevel, bool noBinRestrictions);
igraph_t * distanceBalancedClustering(igraph_t * graph, int perLevel, bool noBinRestrictions);
std::vector<igraph_integer_t> * getGraphsAtLevel(igraph_t * graph, int level);
igraph_integer_t levelLabel(igraph_t * graph);
void test_with_small_hardcoded_graph();
igraph_integer_t findVertexID(igraph_t * graph, std::string id);
igraph_t * noOp(igraph_t * graph);
igraph_t * customClustering(igraph_t * graph, std::string idList);

std::map<igraph_integer_t,std::map<igraph_integer_t,int> * > * calculateDistance(igraph_t * graph, std::vector<igraph_integer_t> * tasks);
void calculateImpactFactors(igraph_t * graph, igraph_integer_t sink);




struct GlobalSettings * getGlobalSettings();

#endif
