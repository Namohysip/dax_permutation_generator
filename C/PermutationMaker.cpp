#include <igraph.h>
#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <fstream>
#include <ctime>
#include <queue>
#include <climits>
#include<map>
#include<sstream>

#include "sha256.hpp"
#include "DAGUtilities.hpp"
#include "PermutationMaker.hpp"

int permCount = 0;
struct GlobalSettings config;




/*Combines the two given nodes, such that node2 is combined into node2.
Runtimes are added together, and the ID of node1 is kept. */
void combine(igraph_t * G, igraph_integer_t node1, igraph_integer_t node2){
	
	//set up the new combined node's components to be both nodes' components put together
	std::string oldComponents1 = VAS(G,"components",node1);
	std::string oldComponents2 = VAS(G,"components",node2);
	std::string newComponents = oldComponents1 + "," + oldComponents2;
	
	//sort components so the resulting DAX file is deterministic (for hashing purposes).
	//keeps the same graph from being hashed differently because components are in a different order.
	std::vector<std::string> subgraphComponents;
	split( newComponents, ',', &subgraphComponents);
	std::sort(subgraphComponents.begin(), subgraphComponents.end());
	newComponents = subgraphComponents.at(0);
	
	for(int i = 1; i < subgraphComponents.size(); i++){
		newComponents += "," + subgraphComponents.at(i);
	}
	int procResult;
	double runtimeResult;
	getMultiprocRuntime(&subgraphComponents, &procResult, &runtimeResult);
	//node1 retains its original ID
	
	igraph_vs_t out2;
	igraph_vs_t in2;
	igraph_vit_t outIter2;
	igraph_vit_t inIter2;
	igraph_vs_adj(&out2, node2, IGRAPH_OUT); //get all successors of node2
	igraph_vs_adj(&in2, node2, IGRAPH_IN); //get all predecessors of node2
	igraph_vit_create(G, out2, &outIter2);
	igraph_vit_create(G, in2, &inIter2);
	
	//This loop gets all out-edges of nose2 and brings it to node1
	while (! IGRAPH_VIT_END(outIter2)){
		igraph_integer_t child = IGRAPH_VIT_GET(outIter2);
		/*Delete the edge (node2, child) and then add edge (node1, child) */
		igraph_es_t edel;
		igraph_integer_t eid;
		igraph_get_eid(G, &eid, node2, child, (igraph_bool_t) true, (igraph_bool_t) false);
		igraph_es_1(&edel, eid);
		igraph_delete_edges(G, edel);
		if (! ( (int) node1 == (int) child)){ //ignore self-loops
		igraph_integer_t exists;
		igraph_get_eid(G, &exists, node1, child, (igraph_bool_t) true, (igraph_bool_t) false);
			if ((int) exists < 0) { //make sure the edge doesn't already exist
				igraph_add_edge(G, node1, child); 
			}
		}
		IGRAPH_VIT_NEXT(outIter2); //go to next out-vertex
		igraph_es_destroy(&edel);
	}
	
	//this loop gets all in-edges of node2 and routes them to node1 instead
	while (! IGRAPH_VIT_END(inIter2)){
		igraph_integer_t parent = IGRAPH_VIT_GET(inIter2);
		/*Delete the edge (parent, node2) and make it (parent, node1) */
		igraph_es_t edel;
		igraph_integer_t eid;
		igraph_get_eid(G, &eid, parent, node2, (igraph_bool_t) true, (igraph_bool_t) false);
		igraph_es_1(&edel, eid);
		igraph_delete_edges(G, edel);
		
		if (! ( (int) node1 == (int) parent)){ //ignore self-loops
		igraph_integer_t exists;
		igraph_get_eid(G, &exists, parent, node1, (igraph_bool_t) true, (igraph_bool_t) false);
			if ((int) exists < 0) { //make sure the edge doesn't already exist.
				igraph_add_edge(G, parent, node1); 
			}
		}
		IGRAPH_VIT_NEXT(inIter2); //go to next in-vertex
		igraph_es_destroy(&edel);
	}
	
	//Sets node1 to the new runtime as calculated above 
	SETVAN(G, "runtime", node1, runtimeResult);
	SETVAS(G,"components",node1, newComponents.c_str());
	SETVAN(G,"procs",node1,procResult);
	igraph_vs_t del;
	igraph_vs_1(&del, node2);
	igraph_delete_vertices(G, del);
	igraph_vs_destroy(&del);
	igraph_vit_destroy(&outIter2);
	igraph_vit_destroy(&inIter2);
	igraph_vs_destroy(&out2);
	igraph_vs_destroy(&in2);
}
/*The same as the other combine, but takse the strings that identify the tasks
 by their "id" value rather than their actual vertex id.*/
void combine ( igraph_t * graph, std::string node1, std::string node2){
	igraph_integer_t realNode1 = findVertexID(graph, node1);
	igraph_integer_t realNode2 = findVertexID(graph, node2);
	combine(graph, realNode1, realNode2);
}

/*Combines all tasks in the vector into the a single task,
 retaining the ID of the first task in the vector */
void combineMulti(igraph_t * graph, std::vector<igraph_integer_t> * tasks){
	if (tasks->size() < 2){
		return;
	}
	igraph_integer_t first = tasks->at(0);
	tasks->erase(tasks->begin());
	
	while(tasks->size() > 0){
		igraph_integer_t next = tasks->at(0);
		combine(graph, first, next);
		tasks->erase(tasks->begin());
		if((int) first > (int) next){
			first = (int) first - 1;
		}
	
		for(int i = 0; i < tasks->size(); i++){
			if((int) tasks->at(i) > (int) next){
			
				tasks->at(i) = tasks->at(i) - 1;
			}
		}
	}
	
	
}
/*Same as the other combineMulti, but takes a list of strings that correspond
  to the "id" values of the vertices to be combined. Converts the list to real ids,
  and then calls the other combineMulti */
void combineMulti(igraph_t * graph, std::vector<std::string> * tasks){
	if (tasks -> size() < 2) {
		return;
	}
	std::vector<igraph_integer_t> * realTasks = new std::vector<igraph_integer_t>;
	for(int i = 0; i < tasks->size(); i++){
		realTasks->push_back(findVertexID(graph, tasks->at(i)));
	}
	combineMulti(graph, realTasks);
	delete(realTasks);
}


/*Determines whether the two given nodes in the two different graphs are the same, in that
they have the same runtime and id attribute values. */
bool isSameJob (igraph_integer_t node1, igraph_integer_t node2, igraph_t * graph1, igraph_t * graph2){
	if ( VAN(graph1, "runtime", node1) != VAN(graph2, "runtime", node2)) {
		return false;
	}
	if ( strcmp(VAS(graph1, "id", node1), VAS(graph2, "id", node2)) != 0) {
		return false;
	}
	return true;
}

/*This function checks if the two graphs have identical jobs based on the edges.
 If every edge matches exactly, then it's the same graph, because all of the jobs
 have the same ids and runtimes for every endpoint, and are arranged the same way. */
bool checkAllNodes (igraph_t * graph1, igraph_t * graph2) {
	if (igraph_vcount(graph1) != igraph_vcount(graph2)) {
		return false;
	}
	if (igraph_ecount(graph1) != igraph_ecount(graph2)) {
		return false;
	}
	igraph_es_t edges1;
	igraph_es_t edges2;
	igraph_eit_t iter1;
	igraph_eit_t iter2;
	
	igraph_es_all(&edges1, IGRAPH_EDGEORDER_FROM);
	igraph_es_all(&edges2, IGRAPH_EDGEORDER_FROM);
	igraph_eit_create(graph1, edges1, &iter1);
	igraph_eit_create(graph2, edges2, &iter2);
	while( ! IGRAPH_EIT_END(iter1) ) {
		bool found = false;
		IGRAPH_EIT_RESET(iter2);
		while (! IGRAPH_EIT_END(iter2) ) {
			igraph_integer_t to1, from1, to2, from2;
			igraph_edge(graph1, IGRAPH_EIT_GET(iter1), &from1, &to1);
			igraph_edge(graph2, IGRAPH_EIT_GET(iter2), &from2, &to2);
			if( isSameJob(from1, from2, graph1, graph2)) {
				if (isSameJob(to1, to2, graph1, graph2)) {
					found = true;
					break;
				}
			}
			IGRAPH_EIT_NEXT(iter2);
		}
		IGRAPH_EIT_NEXT(iter1);
		if (!found) {
			igraph_es_destroy(&edges1);
			igraph_es_destroy(&edges2);
			igraph_eit_destroy(&iter1);
			igraph_eit_destroy(&iter2);
			return false;
		}
	}
	
	igraph_es_destroy(&edges1);
	igraph_es_destroy(&edges2);
	igraph_eit_destroy(&iter1);
	igraph_eit_destroy(&iter2);
	return true;
}

/*Adds an igraph to the UN-HASHED list of igraphs used. Without adding duplicate graphs.*/
bool addWithoutDuplicates( std::vector<igraph_t *> * graphs, igraph_t * toAdd) {
		for(std::vector<igraph_t*>::iterator it = graphs->begin(); it != graphs->end(); ++it){
			if (checkAllNodes( toAdd, *it)){
				return false;
			}
		}
	graphs->push_back(toAdd);
	return true;
}


/*Converts the given graph into a DAX .xml format and then outputs the result. */
void dagToDAX(igraph_t * graph){
	std::string name = config.fileBase + "_p" + std::to_string(permCount) + ".dax";
	permCount++;
	std::ofstream permFile(name);
	if(permFile.is_open()){
		//preamble to all files
		permFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n";
		permFile << "<adag xmlns=\"http://pegasus.isi.edu/schema/DAX\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://pegasus.isi.edu/schema/DAX http://pegasus.isi.edu/schema/dax-2.1.xsd\" version=\"2.1\" count=\"1\" index=\"0\" name=\"NAME\" jobCount=\"";
		permFile << std::to_string(igraph_vcount(graph));
		permFile << "\" fileCount=\"0\" childCount=\"";
		permFile << std::to_string(igraph_vcount(graph) - 1);
		permFile << "\">\n\n";
		for(int i = 0; i < igraph_vcount(graph); i++){
			permFile << "<job id =\"";
			permFile << VAS(graph, "id", i);
			permFile << "\" namespace=\"NAMESPACE\" name=\"Task\" version=\"1.0\" runtime=\"";
			permFile << std::to_string(VAN(graph, "runtime", i));
			permFile << "\" processors=\"";
			permFile << std::to_string(VAN(graph,"procs", i));
			permFile << "\" components=\"";
			permFile << (VAS(graph,"components",i));
			permFile << "\">\n</job>\n";
		}
		permFile << "\n";
		for (int i = 0; i < igraph_vcount(graph); i++) {
			igraph_vs_t parents;
			igraph_vit_t parIter;
		
			igraph_integer_t node = i;
			igraph_vs_adj(&parents, node, IGRAPH_IN);
			igraph_vit_create(graph, parents, &parIter);
			
			if(! IGRAPH_VIT_END(parIter)){
				permFile << "<child ref=\"";
				permFile << VAS(graph, "id", i);
				permFile << "\">";
				while(! IGRAPH_VIT_END(parIter)){
					permFile << "\n  <parent ref=\"";
					permFile << VAS(graph, "id", IGRAPH_VIT_GET(parIter));
					permFile << "\" />";
					
					IGRAPH_VIT_NEXT(parIter);
				}
				permFile << "\n</child>\n";
			}
			igraph_vs_destroy(&parents);
			igraph_vit_destroy(&parIter);
		}
		permFile << "\n</adag>";
		permFile.close();
	}
	else std::cerr << "Error writing file";
}

/*Exhaustively permutes all possible clusterings of the igraph. 
 Limited by both a time limit or cap on how many 
 graphs to permute. Once either limit is reached, no more permutations will be made,
 and the recursive function will go back to the top level and terminate. 
 if the time limit or max number of graphs is <= 0, it is assumed to not have a limit*/
bool exhaustivePerm(std::vector<igraph_t *> * graphs, igraph_t * graph, clock_t start){
	if(igraph_vcount(graph)<= config.minSize){
		return true; //base case -- only one node left
	}
	if(config.timeLimit > 0){
		if (config.timeLimit < double(clock() - start) / CLOCKS_PER_SEC) //time-up case -- return and wrap things up	return false;
		return false;
	}
	if (config.maxGraphs > 0){
		 if(graphs->size() >= config.maxGraphs)
			 return false;
	}
	
	if(graphs->size() % 1000 == 0){
		std::cout << "Made these many permutations: " << graphs->size() << "\n";
	}
	int nodes = igraph_vcount(graph);
	for(int i = 0; i < nodes - 1; i++){
		for(int j = i+1; j < nodes; j++){
			igraph_t * newGraph = new igraph_t;
			igraph_copy(newGraph, graph);
			igraph_integer_t gi = i;
			igraph_integer_t gj = j;
			combine(newGraph, gi, gj);
			igraph_bool_t legal;
			igraph_is_dag(newGraph, &legal);
			if((bool) legal){
				if(config.mergeChainsBefore || config.mergeChainsAfter){ //auto-combine chains
					while(mergeAChain(newGraph));
				}
				if(addWithoutDuplicates(graphs, newGraph)){
					dagToDAX(newGraph);
					if(! exhaustivePerm(graphs, newGraph, start)){
						return false; //ran out of time / hit graph limit
					}
				}
				else{
					igraph_destroy(newGraph);
				}
			}
			else {
				igraph_destroy(newGraph);
			}
		}
	}
	return true;
}


/*This is what should be called when starting any exhaustive permutation that does NOT use hashing. */
std::vector<igraph_t *> * exhaustivePermStart(igraph_t * graph){
	std::vector<igraph_t *> * graphs = new std::vector<igraph_t *>;
	graphs->push_back(graph);
	dagToDAX(graph);
	clock_t start = clock();
	if(config.mergeChainsBefore || config.mergeChainsAfter){
		igraph_t * newGraph = new igraph_t;
		igraph_copy(newGraph, graph);
		while(mergeAChain(newGraph));
		if(addWithoutDuplicates(graphs,newGraph)){
			exhaustivePerm(graphs,newGraph,start);
		}
		else {
			exhaustivePerm(graphs,graph,start);
		}
	}
	else {
		exhaustivePerm(graphs, graph, start);
	}
	return graphs;
}

/*Generates and returns what the given graph will look like in its DAX .xml format. */
std::string dagToDAXStr(igraph_t * graph){
	std::string result = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n<adag xmlns=\"http://pegasus.isi.edu/schema/DAX\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://pegasus.isi.edu/schema/DAX http://pegasus.isi.edu/schema/dax-2.1.xsd\" version=\"2.1\" count=\"1\" index=\"0\" name=\"NAME\" jobCount=\"";
		result += std::to_string(igraph_vcount(graph));
		result += std::string("\" fileCount=\"0\" childCount=\"");
		result += std::to_string(igraph_vcount(graph) - 1) + "\">\n\n";
		
	for(int i = 0; i < igraph_vcount(graph); i++){
		result += "<job id =\"";
		result += std::string(VAS(graph, "id", i));
		result += "\" namespace=\"NAMESPACE\" name=\"Task\" version=\"1.0\" runtime=\"" + std::to_string(VAN(graph, "runtime", i));
		result += "\" processors=\"" + std::to_string(VAN(graph,"procs", i)) + "\" components=\"" + VAS(graph,"components",i);
		result += 	"\"/>\n\n";
		
	}
	result += "\n";
	for (int i = 0; i < igraph_vcount(graph); i++) {
			igraph_vs_t parents;
			igraph_vit_t parIter;
		
			igraph_integer_t node = i;
			igraph_vs_adj(&parents, node, IGRAPH_IN);
			igraph_vit_create(graph, parents, &parIter);
			
			if(! IGRAPH_VIT_END(parIter)){
				result += "<child ref=\"" + std::string(VAS(graph, "id", i)) + "\">";
				while(! IGRAPH_VIT_END(parIter)){
					result += "\n  <parent ref=\"" + std::string(VAS(graph, "id", IGRAPH_VIT_GET(parIter))) + "\" />";
					IGRAPH_VIT_NEXT(parIter);
				}
				result += "\n</child>\n";
			}
			igraph_vs_destroy(&parents);
			igraph_vit_destroy(&parIter);
		}
		result += "\n</adag>";
		return result;
}


/*Outputs all of the graphs in the given list of igraphs, using the file base for the names.
 Currently not used now that all functions output graphs as they are created and validated. */
void outputDAX(std::vector<igraph_t *> * graphs){
	int i = 0;
	std::cout << "Writing out ";
	std::cout << graphs->size();
	std::cout << " files.\n";
	for(std::vector<igraph_t*>::iterator it = graphs->begin(); it != graphs->end(); ++it){
		dagToDAX(*it);
		i++;
		if(i % 1000 == 0){
			std::cout << "Wrote out " << i << " files.\n";
		}
	}
}

/*Same as outputDAX, but directly using the string given.*/
void outputDAXStr(std::string graph){
	std::string name = config.fileBase + "_p" + std::to_string(permCount) + ".dax";
	std::ofstream permFile(name);
	if(permFile.is_open()) {
		permCount++;
		permFile << graph;
	}
	else {
		std::cout << "error writing file";
	}
	permFile.close();
}

/*The hash equivalent of "Add without duplicates" and "dagToDAX" put together.
 First it computes the DAX format of the graph and hashes it, and then checks if there are
 duplicates in the given list. If there aren't, it outputs that file and returns true. Otherwise,
 no file is output, nothing is added to the list, and it returns false. */
bool addHashAndOutput(std::vector<std::string> *graphs, igraph_t * newGraph){
	std::string daxString = dagToDAXStr(newGraph);
	std::string hash = sha256(daxString);
	for(std::vector<std::string>::iterator it = graphs->begin(); it != graphs->end(); ++it){
		if (strcmp(hash.c_str(), it->c_str()) == 0){
			return false;
		}
	}
	graphs->push_back(hash);
	outputDAXStr(daxString);
	return true;
}

/*The hash equivalent of timedExhaustivePerm. */
bool exhaustivePermHash(std::vector<std::string> * graphs, igraph_t * graph, clock_t start){
	if(igraph_vcount(graph)<= config.minSize){
		return true; //base case -- smaller than the minimum allowed size
	}
	if(config.timeLimit > 0){
		if (config.timeLimit < double(clock() - start) / CLOCKS_PER_SEC) //time-up case -- return and wrap things up	return false;
		return false;
	}
	if (config.maxGraphs > 0){
		 if(graphs->size() >= config.maxGraphs)
			 return false;
	}
	
	if(graphs->size() % 1000 == 0){
		std::cout << "Made these many permutations: " << graphs->size() << "\n";
	}
	int nodes = igraph_vcount(graph);
	for(int i = 0; i < nodes - 1; i++){
		for(int j = i+1; j < nodes; j++){
			igraph_t * newGraph = new igraph_t;
			igraph_copy(newGraph, graph);
			igraph_integer_t gi = i;
			igraph_integer_t gj = j;
			combine(newGraph, gi, gj);
			igraph_bool_t legal;
			igraph_is_dag(newGraph, &legal);
			if((bool) legal){
				if(config.mergeChainsBefore || config.mergeChainsAfter){ //auto-combine chains
					while(mergeAChain(newGraph));
				}
				if(addHashAndOutput(graphs, newGraph)){
					if(! exhaustivePermHash(graphs, newGraph, start)){
						igraph_destroy(newGraph);
						delete(newGraph);
						return false; //ran out of time
					}
				}
			}
			igraph_destroy(newGraph);
			delete(newGraph);
		}
	}
	return true;
}

/*The hash equivalent of TimedExhaustivePermStart. checks config.fileBase name for output, 
since the output is done for each permutation after it is created, rather than after the entire
function terminates. */
std::vector<std::string> * exhaustivePermHashStart(igraph_t * graph){
	std::vector<std::string> * graphs = new std::vector<std::string>;
	addHashAndOutput(graphs, graph);
	clock_t start = clock();
	if(config.mergeChainsBefore || config.mergeChainsAfter){
		igraph_t * newGraph = new igraph_t;
		igraph_copy(newGraph, graph);
		while(mergeAChain(newGraph));
		if(addHashAndOutput(graphs,newGraph)){
			exhaustivePermHash(graphs,newGraph,start);
		}
		else {
			exhaustivePermHash(graphs,graph,start);
		}
	}
	else {
		exhaustivePermHash(graphs, graph, start);
	}
	return graphs;
}


bool makeRandomCombination(std::vector<igraph_t *> * graphs, igraph_t * source){
	int node1 = rand() % igraph_vcount(source);
	int node2 = rand() % igraph_vcount(source);
	if(node1 != node2){
		//copy, combine two random nodes, see if it's legal and not already done
		igraph_t * newGraph = new igraph_t; 
		igraph_copy(newGraph, source);
		igraph_integer_t node1_g = node1;
		igraph_integer_t node2_g = node2;
		if (node1 < node2){
			combine(newGraph, node1_g, node2_g);
		}
		else{
			combine(newGraph, node2_g, node1_g);
		}
		igraph_bool_t legal;
		igraph_is_dag(newGraph, &legal);
		if((bool) legal){
			if(! addWithoutDuplicates(graphs, newGraph)){
				igraph_destroy(newGraph);
				delete(newGraph);
				return false;
			}
			else{
				dagToDAX(newGraph);
			}
		}
		else {
			igraph_destroy(newGraph);
			delete(newGraph);
			return false;
		}
	}
	else{
		return false;
	}
	return true;
}
/*Naive implementation of a randomized permutation.
Picks a random graph, picks two random nodes from that graph,
and combines them. If it works, it's added to the list.*/
std::vector<igraph_t *> * randomizedPerm(igraph_t * graph){
	srand((double) clock());
	int failures = 0; //failure ciunt for the same nodes being picked, or an illegal graph is made
	int attempts = 0; //total attempt count
	clock_t start = clock(); //timer
	std::vector<igraph_t *> * graphs = new std::vector<igraph_t*>;
	graphs->push_back(graph);
	dagToDAX(graph);
	while((graphs->size() < config.maxGraphs || config.maxGraphs < 1)&& config.timeLimit > double(clock() - start) / CLOCKS_PER_SEC){ 
	//when time runs out or the goal permutation count is met, whichever comes first.
		
		igraph_t * sourceGraph = graphs->at(rand() % graphs->size()); //get a random graph already permuted
		if(igraph_vcount(sourceGraph) >= config.minSize) {
			attempts++;
			if(makeRandomCombination(graphs, sourceGraph)){
				if(graphs->size() % 1000 == 0){
					std::cout << "Made these many permutations: " << graphs->size() << "\n";
				}
			}
			failures++;
		}
	}
	std::cout << "Attempted making a graph this many times: " << attempts << "\n";
	std::cout << "Made these many illegal graphs: " << failures << "\n";
	return graphs;
}

/** Alternative version of randomized permutation that, instead of totally random permutations,
   makes a set number of randomized graphs of each size. In other words, if a graph is of size 10,
   and you want to make 5 graphs per size, then there will be 5 graphs of size 9, 5 of size 8,
   5 of size 7, and so on, unless it cannot create enough graphs after the specified number of
   attempts, configurable through the command prompt
*/
int RandomizedPermEvenSpread(igraph_t * graph){
	srand((double) clock());
	std::vector<igraph_t *> * open = new std::vector<igraph_t *>;
	dagToDAX(graph);
	int i;
	for(i = 0; i < config.maxGraphs; i++){ //create the original batch of permutations from the original graph.
		int attempt = 0;
		while(!makeRandomCombination(open,graph) && attempt < config.attemptCap){
			attempt++;
		}
		if(attempt < config.attemptCap){
			if((i+1) % 1000 == 0){
					std::cout << "Made these many permutations: " << i << "\n";
			}
		}
	}
	//For each if the original batch, make a new permutation by combining two of its nodes.
	while(open->size() > 0){
		igraph_t * iteration = open->front(); //treats this as a breadth-first search.
		int attempt = 0;
		if(igraph_vcount(iteration) > config.minSize){
			while(!makeRandomCombination(open, iteration) && attempt < config.attemptCap){
				attempt++;
			}
			if(attempt < config.attemptCap){
				i++;
				if(i % 1000 == 0){
						std::cout << "Made these many permutations: " << i << "\n";
				}
			}
		}
		//Only has to check for duplicates at the same level,
		//since all other permutations would have a different task count.
		open->erase(open->begin());
		igraph_destroy(iteration);	
		delete(iteration);
	}
	delete(open);
	return i;
}

/**
Gets all nodes that have no parents, and gives them a parent "head" node with no weight.
*/
igraph_integer_t makeHead(igraph_t * graph){
	igraph_integer_t node = 0;
	igraph_integer_t size = 0;
	igraph_add_vertices(graph,1,0);
	igraph_integer_t head = igraph_vcount(graph) - 1;
	SETVAS(graph,"id",head,"TEMP_HEAD");
	SETVAN(graph,"runtime",head,0);
	int count = 0;
	for(int i = 0; i < igraph_vcount(graph); i++){
		igraph_vs_t in;
		node = i;
		igraph_vs_adj(&in, node, IGRAPH_IN); //get all predecessors of the node
		igraph_vs_size(graph, &in, &size);
		if((int) size == 0 && (int) node != (int) head){
			count++;
			igraph_add_edge(graph, head, node);
		}
	}
	
	return head;
}

/**
Gets all nodes that have no children, and gives them a child "sink" node with no weight.
*/
igraph_integer_t makeSink(igraph_t * graph){
	igraph_integer_t node = 0;
	igraph_integer_t size = 0;
	igraph_add_vertices(graph,1,0);
	igraph_integer_t sink = igraph_vcount(graph) - 1;
	SETVAS(graph,"id",sink,"TEMP_SINK");
	SETVAN(graph,"runtime",sink,0);
	int count = 0;
	for(int i = 0; i < igraph_vcount(graph); i++){
		igraph_vs_t out;
		node = i;
		igraph_vs_adj(&out, node, IGRAPH_OUT); //get all successors of the node
		igraph_vs_size(graph, &out, &size);
		if((int) size == 0 && (int) node != (int) sink){
			count++;
			igraph_add_edge(graph,node,sink);
		}
	}
	
	return sink;
}

/*Returns the vertex id of TEMP_HEAD. The id of TEMP_SINK is the vertex id of TEMP_HEAD + 1.
*Labels the levels of every node. NOTE: adds TWO NEW NODES to the input graph, TEMP_HEAD and TEMP_SINK.
*These nodes should be deleted before outputing the DAX file of the graph, but they are not deleted
*here in case they are needed later after the labeling is complete.
*/
igraph_integer_t levelLabel(igraph_t * graph){
	igraph_integer_t head = makeHead(graph); //create head node.
	igraph_integer_t sink = makeSink(graph);
	for(int i = 0; i < igraph_vcount(graph); i++){
		SETVAN(graph,"level",i,0);
	}
	std::queue<igraph_integer_t> que;
	que.push(head); //start at the head
	while(!que.empty()){
		igraph_integer_t nextNode = que.front();
		que.pop();
		int level = VAN(graph,"level",nextNode) + 1;
		igraph_vs_t children;
		igraph_vit_t childIter;
		igraph_vs_adj(&children,nextNode,IGRAPH_OUT);
		igraph_vit_create(graph,children,&childIter);
		while(! IGRAPH_VIT_END(childIter)){
			igraph_integer_t child = IGRAPH_VIT_GET(childIter);
			if(VAN(graph,"level",child) < level){ 
			//if the child's level is less than it should be.
			//Otherwise, its level is higher because of a deeper parent than this one
				SETVAN(graph,"level",child,level);
				que.push(child);
			}
			IGRAPH_VIT_NEXT(childIter);
		}
		igraph_vs_destroy(&children);
		igraph_vit_destroy(&childIter);
	}
	
	return head;
}

/**
Returns the struct that holds all global configuratio variables
*/
struct GlobalSettings * getGlobalSettings(){
	return &config;
}

/**
This method will try to find a node that has only one child, and that child has
only that node as its parent. If it finds a graph like this, it conbines them and returns true.
If it cannot find such a pair, it does nothing and returns false.
*/
bool mergeAChain(igraph_t * graph){
	for (int i = 0; i < igraph_vcount(graph); i++) {
		igraph_integer_t parent = i;
		igraph_integer_t size;
		igraph_vs_t children;
		igraph_vs_adj(&children,parent,IGRAPH_OUT);
		igraph_vs_size(graph, &children, &size);
		if((int) size == 1){//there is only one child to this vertex
			igraph_vit_t childIter;
			igraph_vit_create(graph,children,&childIter);
			igraph_integer_t onlyChild = IGRAPH_VIT_GET(childIter);
			igraph_vs_t parents;
			igraph_vs_adj(&parents,onlyChild,IGRAPH_IN);
			igraph_vs_size(graph,&parents,&size);
			if((int) size == 1) { //there is only one parent to this vertex
				combine(graph,parent,onlyChild);
				igraph_vs_destroy(&children);
				igraph_vs_destroy(&parents);
				igraph_vit_destroy(&childIter);
				return true;
			}
			igraph_vs_destroy(&parents);
			igraph_vit_destroy(&childIter);
		}
		igraph_vs_destroy(&children);
	}
	return false;
}


/**
This method clusters tasks given in tasksAtLEvel by runtime, balancing the workflow at each level by runtime.
*/
std::vector<struct taskBin *> * clusterByRuntime(igraph_t * newGraph, std::vector<igraph_integer_t> * tasksAtLevel, int perLevel, bool noBinRestrictions){
	std::vector<struct taskBin *> * taskBins = new std::vector<struct taskBin *>;
	
	struct taskBin * smallestBin = NULL;
	int maxBinSize = tasksAtLevel->size() / perLevel;
	
	for(int i = 0; i < perLevel; i++){
		taskBins->push_back(new struct taskBin);
	}
	smallestBin = taskBins->at(0);
	//Keep merging tasks until nothing is left.
	while(tasksAtLevel->size() > 0){
		
		double maxRuntime = VAN(newGraph,"runtime",tasksAtLevel->at(0));
		int indexOfMax = 0; //needed for updating the vector list later
		igraph_integer_t maxTask = tasksAtLevel->at(0);
		
		//find the largest task by runtime
		for(int i = 0; i < tasksAtLevel->size(); i++){
			double nextRuntime = VAN(newGraph,"runtime",tasksAtLevel->at(i));
			if(nextRuntime > maxRuntime){
				maxRuntime = nextRuntime;
				indexOfMax = i;
				maxTask = tasksAtLevel->at(i);
			}
		}
		//place the largest task in the smallest bin
		smallestBin->totalRuntime += maxRuntime;
		smallestBin->ids.push_back(VAS(newGraph,"id",maxTask));
		tasksAtLevel->erase(tasksAtLevel->begin() + indexOfMax);
		//find new smallest bin
		smallestBin = NULL;
		while(smallestBin == NULL) {
			for(int i = 0; i < taskBins->size(); i++){
				if(smallestBin != NULL) {
					if(smallestBin->totalRuntime > taskBins->at(i)->totalRuntime && (taskBins->at(i)->ids.size() < maxBinSize || noBinRestrictions)){
					smallestBin = taskBins->at(i);
					}
				}
				else{
					if(taskBins->at(i)->ids.size() < maxBinSize){
						smallestBin = taskBins->at(i);
					}
				}
			}
			if(smallestBin == NULL){
				maxBinSize++; //if all bins are full, increment the bin size in order to put in the leftovers
			}
		}	
	}
	
	return taskBins;
}

/*Begins by creating a copy of the source graph.
  All work is done on the new graph.
  Labels all levels.
  Combines into different bins per level to then combine in the graph,
  Number of bins per level is determined by perLevel parameter.
  noBinRestrictions, if true, does the above. If false, then only 
  (tasks per level / perLevel) tasks may be placed into any bin, regardless of balancing.
  If there are leftover tasks, only one more may be placed into any bin
  after all other bins have been filled.
  */
igraph_t * horizontalClustering(igraph_t * graph, int perLevel, bool noBinRestrictions){
	//Make new graph and label their levels
	igraph_t * newGraph = new igraph_t;
	igraph_copy(newGraph, graph);
	
	
	if(config.mergeChainsBefore){ //auto-combine chains before any other changes, if desired
		while(mergeAChain(newGraph));
	}
	
	igraph_integer_t head = levelLabel(newGraph);
	igraph_integer_t sink = (int) head + 1;
	int depth = VAN(newGraph,"level",sink);
	
	/*Delete the head and sink nodes, as they are no longer needed*/
	igraph_vs_t delSink;
	igraph_vs_1(&delSink, sink);
	igraph_delete_vertices(newGraph, delSink);
	igraph_vs_destroy(&delSink);
	
	igraph_vs_t del;
	igraph_vs_1(&del, head);
	igraph_delete_vertices(newGraph, del);
	igraph_vs_destroy(&del);
	//For each level, combine tasks such that for each given bin, the largest task is put into the 
	//most empty bin, and so on, until all tasks have been placed in a bin.
	for(int level = 1; level < depth; level++){
		std::vector<igraph_integer_t> * tasksAtLevel = getGraphsAtLevel(newGraph, level);
		
		std::vector<struct taskBin *> * taskBins = clusterByRuntime(newGraph, tasksAtLevel, perLevel, noBinRestrictions);
		
		//after the bins have all been properly clustered at that level...
		for(int i = 0; i < taskBins->size(); i++){
			combineMulti(newGraph, &(taskBins->at(i)->ids));
		}
		
		//memory cleanup
		for(int i = 0; i < taskBins->size(); i++){
			delete(taskBins->at(i));
		}
		delete(tasksAtLevel);
		delete(taskBins);
		std::cout << "Finished clustering level " << level << "\n";
	}
	
	if(config.mergeChainsAfter){ //auto-combine chains after all other changes, if desired
		while(mergeAChain(newGraph));
	}
	
	outputDAXStr(dagToDAXStr(newGraph));
	return newGraph;
}

/*for the given graph, return a vector list of all tasks at the given level.*/
std::vector<igraph_integer_t> * getGraphsAtLevel(igraph_t * graph, int level){
	std::vector<igraph_integer_t> * tasksAtLevel = new std::vector<igraph_integer_t>;
	for(int i = 0; i < igraph_vcount(graph); i++){
		if(VAN(graph,"level",i) == level){
			tasksAtLevel->push_back(i);
		}
	}
		return tasksAtLevel;
}

/*Impact factor is defined as the sum of the impact factors of the child tasks, divided
 by the number of parents the child has. In other words, if a child has 2 parents, and an
 IF of 0.5, then its contribution to the parent's IF will be 0.5/2 = 0.25.
 Any task that is the direct parent of TEMP_SINK has a starting IF of 1. */
void calculateImpactFactors(igraph_t * graph, igraph_integer_t sink){
	for(int i = 0; i < igraph_vcount(graph); i++){
		SETVAN(graph,"IF",i,0.0); //start by setting all tasks to 1 IF.
	}
	
	std::queue<igraph_integer_t> que; //queue of tasks that will have their IF calculated
	/* This segment of the code initializes everything: Sets the bottom tasks to an IF of 1,
	 and puts their parents onto the queue to later calculate their IF. */
	igraph_vs_t parentsOfSink;
	igraph_vit_t parentsOfSinkIter;
	igraph_vs_adj(&parentsOfSink,sink,IGRAPH_IN);
	igraph_vit_create(graph,parentsOfSink,&parentsOfSinkIter);
	while(! IGRAPH_VIT_END(parentsOfSinkIter)){
		igraph_integer_t parent = IGRAPH_VIT_GET(parentsOfSinkIter); //for each parent of the sink 
		SETVAN(graph,"IF",parent,1.0); //set its IF to 1
		
		igraph_vs_t parents;
		igraph_vit_t parentsIter;
		igraph_vs_adj(&parents,parent,IGRAPH_IN); //get a list of each parent's parents.
		igraph_vit_create(graph,parents,&parentsIter);
		
		while(! IGRAPH_VIT_END(parentsIter)){ //for each parent of the parent of the sink
			que.push(IGRAPH_VIT_GET(parentsIter)); //add it to the queue for later
			IGRAPH_VIT_NEXT(parentsIter);
		}
		igraph_vit_destroy(&parentsIter);
		igraph_vs_destroy(&parents);
		IGRAPH_VIT_NEXT(parentsOfSinkIter);
	}
	igraph_vit_destroy(&parentsOfSinkIter);
	igraph_vs_destroy(&parentsOfSink);
	while(!que.empty()){
		igraph_integer_t nextNode = que.front();
		que.pop();
		//calculate the IF. May be incomplete, but the final time its IF is calculated
		//will be when all of its children have had their IF also calculated, so the last one
		//will be the completed IF value.
		double totalIF = 0;
		
		igraph_vs_t children;
		igraph_vit_t childrenIter;
		igraph_vs_adj(&children,nextNode,IGRAPH_OUT);
		igraph_vit_create(graph,children,&childrenIter);
		while(! IGRAPH_VIT_END(childrenIter)){ //for each of this task's chindren
			igraph_integer_t child = IGRAPH_VIT_GET(childrenIter);
			
			igraph_vs_t parentsOfChild;
			igraph_vs_adj(&parentsOfChild,child,IGRAPH_IN);
			igraph_integer_t parentCount = 0; 
			igraph_vs_size(graph, &parentsOfChild, &parentCount); //get a count of its parents
			//add its IF / parent count to the total IF of this task
			totalIF += (double) VAN(graph,"IF",child) / (double) parentCount; 
			igraph_vs_destroy(&parentsOfChild);
			IGRAPH_VIT_NEXT(childrenIter);
		}
		
		//set its IF to the total of all (children divided by parentCount) IF values.
		SETVAN(graph,"IF",nextNode,totalIF); 
		
		igraph_vit_destroy(&childrenIter);
		igraph_vs_destroy(&children);
		
		/*This part of the code adds the parents of this task to the queue. */
		igraph_vs_t parents;
		igraph_vit_t parentsIter;
		igraph_vs_adj(&parents,nextNode,IGRAPH_IN);
		igraph_vit_create(graph,parents,&parentsIter);
		while(! IGRAPH_VIT_END(parentsIter)){
			que.push(IGRAPH_VIT_GET(parentsIter));
			IGRAPH_VIT_NEXT(parentsIter);
		}
		igraph_vit_destroy(&parentsIter);
		igraph_vs_destroy(&parents);
	}
}

/**
This is a lot like the above method of horizontal clustering, but prioritizes similarity in Impact Factor.
This is a metric that measures how much 'influence' a task has based on its dependencies. For simplicity, 
if the sink node has an impact factor of 1, then for n parents of the sink, each parent will have an impact
factor of 1/n. Then, for one of those parents as an example, the m parents of that node will have an impact
factor of (1/n)/m. And so on and so forth. Nodes with the same impact factor are clustered together first.
*/
igraph_t * impactFactorClustering(igraph_t * graph, int perLevel, bool noBinRestrictions){
	igraph_t * newGraph = new igraph_t;
	igraph_copy(newGraph, graph);
	
	if(config.mergeChainsBefore){ //auto-combine chains before any other changes, if desired
		while(mergeAChain(newGraph));
	}
	
	igraph_integer_t head = levelLabel(newGraph);
	igraph_integer_t sink = (int) head + 1;
	int depth = VAN(newGraph,"level",sink);
	std::cout << "Calculating Impact Factors... \n";
	calculateImpactFactors(newGraph, sink);
	std::cout << "Finished calculating Impact Factors.\n";
	/*Delete the head and sink nodes, as they are no longer needed*/
	igraph_vs_t delSink;
	igraph_vs_1(&delSink, sink);
	igraph_delete_vertices(newGraph, delSink);
	igraph_vs_destroy(&delSink);
	
	igraph_vs_t del;
	igraph_vs_1(&del, head);
	igraph_delete_vertices(newGraph, del);
	igraph_vs_destroy(&del);
	
	//For each level, combine tasks such that for each given bin, the largest task is put into the 
	//most empty bin, and so on, until all tasks have been placed in a bin.
	for(int level = 1; level < depth; level++){
		std::vector<igraph_integer_t> * tasksAtLevel = getGraphsAtLevel(newGraph, level);
		std::vector<struct taskBin *> * taskBins = new std::vector<struct taskBin *>;
		struct taskBin * smallestBin = NULL;
		int maxBinSize = tasksAtLevel->size() / perLevel;
		for(int i = 0; i < perLevel; i++){
			taskBins->push_back(new struct taskBin);
		}
		smallestBin = taskBins->at(0);
		//Keep merging tasks until nothing is left.
		while(tasksAtLevel->size() > 0){
			
			double maxRuntime = VAN(newGraph,"runtime",tasksAtLevel->at(0));
			int indexOfMax = 0; //needed for updating the vector list later
			double maxRunAndIFMatch = -1;
			int indexOfMaxAndIFMatch = -1;
			igraph_integer_t maxTaskAndIFMatch = -1;
			/*If this first task already matches the IF of the bin, have it here. */
			if (smallestBin->lastIFAdded == VAN(newGraph,"IF",tasksAtLevel->at(0))){
				maxRunAndIFMatch = VAN(newGraph,"runtime",tasksAtLevel->at(0));
				indexOfMaxAndIFMatch = 0;
				maxTaskAndIFMatch = tasksAtLevel->at(0);
			}
			igraph_integer_t maxTask = tasksAtLevel->at(0);
			
			//find the largest task by runtime
			for(int i = 0; i < tasksAtLevel->size(); i++){
				double nextRuntime = VAN(newGraph,"runtime",tasksAtLevel->at(i));
				if(nextRuntime > maxRuntime){
					maxRuntime = nextRuntime;
					indexOfMax = i;
					maxTask = tasksAtLevel->at(i);
				}
				/*If this task also matches the IF value of the bin, add it here, too. */
				if(VAN(newGraph,"IF",tasksAtLevel->at(i)) == smallestBin->lastIFAdded){
					double nextRuntime = VAN(newGraph,"runtime",tasksAtLevel->at(i));
					if(nextRuntime > maxRunAndIFMatch){
						maxRunAndIFMatch = nextRuntime;
						indexOfMaxAndIFMatch = i;
						maxTaskAndIFMatch = tasksAtLevel->at(i);
					}
				}
			}
			//place the largest task in the smallest bin, or the largest task with a matching IF value
			if(indexOfMaxAndIFMatch < 0){
				smallestBin->totalRuntime += maxRuntime;
				smallestBin->ids.push_back(VAS(newGraph,"id",maxTask));
				smallestBin->lastIFAdded = VAN(newGraph,"IF",maxTask); //update IF value
				tasksAtLevel->erase(tasksAtLevel->begin() + indexOfMax);
			}
			else {
				smallestBin->totalRuntime += maxRunAndIFMatch;
				smallestBin->ids.push_back(VAS(newGraph,"id",maxTaskAndIFMatch));
				tasksAtLevel->erase(tasksAtLevel->begin() + indexOfMaxAndIFMatch);
			}
			
			//find new smallest bin
			smallestBin = NULL;
			while(smallestBin == NULL) {
				for(int i = 0; i < taskBins->size(); i++){
					if(smallestBin != NULL) {
						if(smallestBin->totalRuntime > taskBins->at(i)->totalRuntime && (taskBins->at(i)->ids.size() < maxBinSize || noBinRestrictions)){
						smallestBin = taskBins->at(i);
						}
					}
					else{
						if(taskBins->at(i)->ids.size() < maxBinSize){
							smallestBin = taskBins->at(i);
						}
					}
				}
				if(smallestBin == NULL){
					maxBinSize++; //if all bins are full, increment the bin size in order to put in the leftovers
				}
			}
			
		}
		//after the bins have all been properly clustered at that level...
		for(int i = 0; i < taskBins->size(); i++){
			combineMulti(newGraph, &(taskBins->at(i)->ids));
		}
		
		//memory cleanup
		for(int i = 0; i < taskBins->size(); i++){
			delete(taskBins->at(i));
		}
		delete(tasksAtLevel);
		delete(taskBins);
		std::cout << "Finished clustering level " << level << "\n";
	}
	
	if(config.mergeChainsAfter){ //auto-combine chains after all other changes, if desired
		while(mergeAChain(newGraph));
	}
	
	outputDAXStr(dagToDAXStr(newGraph));
	return newGraph;
	
}
/*Adds without duplicates the id of a task and its distance to the vector of tasks, as long as that id
  hasn't already been added to that vector. */
void addIdWithoutDuplicates(std::vector<struct taskDistance> * tasks, igraph_integer_t id, int dist){
	for(int i = 0; i < tasks->size(); i++){ //check if the id is not already on the list
		if((int) id == (int) ((tasks->at(i)).id)){
			return;
		}
	}
	struct taskDistance * newTask = new struct taskDistance;
	newTask->id = id;
	newTask->distance = dist;
	tasks->push_back(*newTask);
	delete(newTask);
}

/*Gets the minimum distance between two nodes, assuming they're at the same level.
  If the only node they have in common is TEMP_SINK, it returns INT_MAX instead. */
int distanceBetween ( igraph_t * graph, igraph_integer_t node1, igraph_integer_t node2){
	std::vector<struct taskDistance> tasks1;
	std::vector<struct taskDistance> tasks2;
	
	struct taskDistance first;
	first.id = node1;
	first.distance = 0;
	tasks1.push_back(first);
	int i = 0;
	//get a list of all tasks from the first task.
	while(i < tasks1.size()){
		igraph_integer_t parent = tasks1.at(i).id;
		int distance = tasks1.at(i).distance + 1;
		igraph_vs_t children;
		igraph_vit_t childrenIter;
		igraph_vs_adj(&children,parent,IGRAPH_OUT);
		igraph_vit_create(graph,children,&childrenIter);
		while(! IGRAPH_VIT_END(childrenIter)){ //for each of this task's chindren
			igraph_integer_t child = IGRAPH_VIT_GET(childrenIter);
			/*new ids are added only once, and only the first time it is found is used.
			  Since this is effectively a breadth-first-search, the first time the task
			  is found is the shortest distance from the parent. */
			addIdWithoutDuplicates(&tasks1, child, distance);
			IGRAPH_VIT_NEXT(childrenIter);
		}
		igraph_vit_destroy(&childrenIter);
		igraph_vs_destroy(&children);
		i++;
	}
	first.id = node2;
	first.distance = 0;
	tasks2.push_back(first);
	i = 0;
	//Do the same thing again for the second task.
	while(i < tasks2.size()){
		igraph_integer_t parent = tasks2.at(i).id;
		igraph_integer_t distance = tasks2.at(i).distance + 1;
		igraph_vs_t children;
		igraph_vit_t childrenIter;
		igraph_vs_adj(&children,parent,IGRAPH_OUT);
		igraph_vit_create(graph,children,&childrenIter);
		while(! IGRAPH_VIT_END(childrenIter)){ //for each of this task's chindren
			igraph_integer_t child = IGRAPH_VIT_GET(childrenIter);
			/*new ids are added only once, and only the first time it is found is used.
			  Since this is effectively a breadth-first-search, the first time the task
			  is found is the shortest distance from the parent. */
			addIdWithoutDuplicates(&tasks2, child, distance);
			IGRAPH_VIT_NEXT(childrenIter);
		}
		igraph_vit_destroy(&childrenIter);
		igraph_vs_destroy(&children);
		i++;
	}
	
	std::vector<struct taskDistance> matches;
	for(i = 0; i < tasks1.size(); i++){
		for(int j = 0; j < tasks2.size(); j++){ //find matches between the two lists.
			if ( (int) (tasks1.at(i).id) == (int) (tasks2.at(j).id)){
				struct taskDistance match;
				match.id = tasks1.at(i).id;
				match.distance = tasks1.at(i).distance + tasks2.at(j).distance;
				matches.push_back(match);
				j = tasks2.size(); //skip the rest of the loop now that its match was found.
			}
		}
	}
	
	int minDistanceIndex = 0;
	int minDistance = matches.at(0).distance;
	for(int i = 1; i < matches.size(); i++){
		if(matches.at(i).distance < minDistance){
			minDistanceIndex = i;
			minDistance = matches.at(i).distance;
		}
	}
	std::string idOfMatch = VAS(graph,"id",matches.at(minDistanceIndex).id);
	if( strcmp(idOfMatch.c_str(),"TEMP_SINK") == 0){
		return INT_MAX;
	}
	else{
		return minDistance;
	}
}
/*Calculates the distances between all tasks given in the list. Tasks in the list should be at the same level. 
  Returns a map of maps of those distances, such that the outer map is the first task's graph id allocated to a map of all other tasks,
  such that the second vertex id maps to the distance between the outer id and the inner id. Example code: 
  
  int distance = outermap.at(node1ID).at(node2ID);
  
  Note: The graph goes along the list starting from the first element in the vector.
  Therefore, the inner node IDs will ALWAYS be the nodes that ONLY come AFTER the outer node's position in the list.
  
  If you have a list of node ids (6, 3, 7, 8, 9)
  then if you're finding all the distances for nodes from 3, you should call
  int distance = outermap.at(3).at(7 or 8 or 9);
  for nodes AFTER 3 in the list, and
  int distance = outermap.at(6).at(3);
  for nodes BEFORE 3 in the list, which in this example is only 6.
  */
std::map<igraph_integer_t,std::map<igraph_integer_t,int> * > * calculateDistance(igraph_t * graph, std::vector<igraph_integer_t> * tasks){
	std::map<igraph_integer_t,std::map<igraph_integer_t,int> * > * distances = new std::map<igraph_integer_t,std::map<igraph_integer_t,int> * >;
	for(int i = 0; i < tasks->size(); i++){
		std::map<igraph_integer_t,int> * innerDistances = new std::map<igraph_integer_t,int>;
		for(int j = 0; j < tasks->size(); j++){
			if(i != j) {
			int dist = distanceBetween(graph, tasks->at(i),tasks->at(j));
			innerDistances->insert(std::pair<igraph_integer_t,int>(tasks->at(j),dist));
			}
		}
		distances->insert(std::pair<igraph_integer_t, std::map<igraph_integer_t,int> *>(tasks->at(i), innerDistances));
	}
	return distances;
	
}

/**
Like Impact Factor above, this is another clustering method that is similar to horizontal clustering,
but instead of clustering by that metric, they are clustered by distance from one another instead based
on their combined distance to a common descendant. For example, if nodes A and B share the same child C,
and node A is 2 edge-hops from C and node B is 3 hops, then their distance is 5.
Tasks with the lowest distance are clustered first. */
igraph_t * distanceBalancedClustering(igraph_t * graph, int perLevel, bool noBinRestrictions){
	igraph_t * newGraph = new igraph_t;
	igraph_copy(newGraph, graph);
	
	if(config.mergeChainsBefore){ //auto-combine chains before any other changes, if desired
		while(mergeAChain(newGraph));
	}
	
	igraph_integer_t head = levelLabel(newGraph);
	igraph_integer_t sink = head + 1;
	int depth = VAN(newGraph,"level",sink);
	
	/*Delete the head node, as it is no longer needed*/
	igraph_vs_t del;
	igraph_vs_1(&del, head);
	igraph_delete_vertices(newGraph, del);
	igraph_vs_destroy(&del);
	
	//For each level, combine tasks such that for each given bin, the largest task is put into the 
	//most empty bin, and so on, until all tasks have been placed in a bin.
	for(int level = 1; level < depth; level++){
		std::vector<igraph_integer_t> * tasksAtLevel = getGraphsAtLevel(newGraph, level);
		std::vector<struct taskBin *> * taskBins = new std::vector<struct taskBin *>;
		struct taskBin * smallestBin = NULL;
		int maxBinSize = tasksAtLevel->size() / perLevel;
		std::cout << "Calculating distances for level " << level << "\n";
		std::map<igraph_integer_t,std::map<igraph_integer_t,int> * > * taskDistances = calculateDistance(newGraph, tasksAtLevel);
		std::cout << "Clustering tasks for level " << level << "\n";
		for(int i = 0; i < perLevel; i++){
			taskBins->push_back(new struct taskBin);
		}
		smallestBin = taskBins->at(0);
		//Keep merging tasks until nothing is left.
		while(tasksAtLevel->size() > 0){
			
			double maxRuntime = VAN(newGraph,"runtime",tasksAtLevel->at(0));
			int indexOfMax = 0; //needed for updating the vector list later
			int distanceOfMax = INT_MAX;
			igraph_integer_t maxTask = tasksAtLevel->at(0);
			
			//find the largest task by runtime with the shortest distance
			for(int i = 0; i < tasksAtLevel->size(); i++){
				int dist = -1;
				if(smallestBin->lastAdded >= 0){
					dist = taskDistances->at(smallestBin->lastAdded)->at(tasksAtLevel->at(i));
				}
				double nextRuntime = VAN(newGraph,"runtime",tasksAtLevel->at(i));
				if(nextRuntime > maxRuntime && (dist == -1 || dist <= distanceOfMax)){
					maxRuntime = nextRuntime;
					indexOfMax = i;
					distanceOfMax = dist;
					maxTask = tasksAtLevel->at(i);
				}
				
			}
			//place the task that has the shortest distance from the task bin, and the largest runtime of those with the shortest distance, into the bin
			smallestBin->totalRuntime += maxRuntime;
			smallestBin->ids.push_back(VAS(newGraph,"id",maxTask));
			smallestBin->lastAdded = maxTask;
			tasksAtLevel->erase(tasksAtLevel->begin() + indexOfMax);
			
			//find new smallest bin
			smallestBin = NULL;
			while(smallestBin == NULL) {
				for(int i = 0; i < taskBins->size(); i++){
					if(smallestBin != NULL) {
						if(smallestBin->totalRuntime > taskBins->at(i)->totalRuntime && (taskBins->at(i)->ids.size() < maxBinSize || noBinRestrictions)){
						smallestBin = taskBins->at(i);
						}
					}
					else{
						if(taskBins->at(i)->ids.size() < maxBinSize){
							smallestBin = taskBins->at(i);
						}
					}
				}
				if(smallestBin == NULL){
					maxBinSize++; //if all bins are full, increment the bin size in order to put in the leftovers
				}
			}
			
		}
		//after the bins have all been properly clustered at that level...
		for(int i = 0; i < taskBins->size(); i++){
			combineMulti(newGraph, &(taskBins->at(i)->ids));
		}
		
		//memory cleanup
		for(int i = 0; i < taskBins->size(); i++){
			delete(taskBins->at(i));
		}
		delete(tasksAtLevel);
		delete(taskBins);
		for(std::map<igraph_integer_t,std::map<igraph_integer_t,int> * >::iterator itr = taskDistances->begin(); itr != taskDistances->end(); itr++){
			itr->second->clear();
			delete(itr->second);
		}
		taskDistances->clear();
		delete(taskDistances);
	}

	igraph_vs_t delSink;
	igraph_vs_1(&delSink, findVertexID(newGraph, "TEMP_SINK"));
	igraph_delete_vertices(newGraph, delSink);
	igraph_vs_destroy(&delSink);
	
	if(config.mergeChainsAfter){ //auto-combine chains after all other changes, if desired
		while(mergeAChain(newGraph));
	}
	
	outputDAXStr(dagToDAXStr(newGraph));
	return newGraph;
	
}

/*Converts the string ID into the igraph's internal vertex id.
Useful for utility. */
igraph_integer_t findVertexID(igraph_t * graph, std::string id){
	for(int i = 0; i < igraph_vcount(graph); i++){
		if( strcmp(VAS(graph, "id", i), id.c_str()) == 0){
			return i;
		}
	}
	return -1;
}
/*Splitting a string, taken from Stackoverflow */
void split(const std::string &s, char delim, std::vector<std::string>  * elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems->push_back(item);
    }
}

/**
Another type of horizontal clustering that clusters by runtime. However, instead of clustering
by level, this method clusters specifically through fork-joins. In this context, a fork-join is
where there is one node with many children, and all of these children all have only that node as 
a parent, and they also only have the same child node as well.
Alternatively speaking, it performs a runtime-balanced clustering on all tasks that have exactly one
in-edge and exactly one out-edge, and they come from and point to the same tasks.
*/
igraph_t * forkJoin(igraph_t * graph, int perLevel, bool noBinRestrictions){
	igraph_t * newGraph = new igraph_t;
	igraph_copy(newGraph, graph);
	if(config.mergeChainsBefore){ //auto-combine chains after all other changes, if desired
		while(mergeAChain(newGraph));
	}
	for(int i = 0; i < igraph_vcount(newGraph); i++){
		std::vector<igraph_integer_t> * tasksInFork = new std::vector<igraph_integer_t>;
		tasksInFork->push_back(i);
		igraph_integer_t task = i;
		igraph_vs_t children;
		igraph_vs_t parents;
		igraph_vit_t childrenIter;
		igraph_vit_t parentsIter;
		igraph_vs_adj(&children,task,IGRAPH_OUT);
		igraph_vs_adj(&parents,task,IGRAPH_IN);
		igraph_vit_create(newGraph,children,&childrenIter);
		igraph_vit_create(newGraph,parents,&parentsIter);
		igraph_integer_t childSize;
		igraph_integer_t parentSize; 
		igraph_vs_size(newGraph, &children, &childSize);
		igraph_vs_size(newGraph, &parents, &parentSize);
		
		if((int) childSize == 1 && (int) parentSize == 1){ //if task i has only one parent and one child
			igraph_integer_t forkHead = IGRAPH_VIT_GET(parentsIter);
			igraph_integer_t forkSink = IGRAPH_VIT_GET(childrenIter);
			
			for(int j = i+1; j < igraph_vcount(newGraph); j++){ //check for all other tasks that. . .
				igraph_integer_t task2 = j;
				igraph_vs_t children2;
				igraph_vs_t parents2;
				igraph_vit_t childrenIter2;
				igraph_vit_t parentsIter2;
				igraph_vs_adj(&children2,task2,IGRAPH_OUT);
				igraph_vs_adj(&parents2,task2,IGRAPH_IN);
				igraph_vit_create(newGraph,children2,&childrenIter2);
				igraph_vit_create(newGraph,parents2,&parentsIter2);
				igraph_vs_size(newGraph, &children2, &childSize);
				igraph_vs_size(newGraph, &parents2, &parentSize);
		
				if((int) childSize == 1 && (int) parentSize == 1){ //if task j has only one parent and one child
					igraph_integer_t forkHead2 = IGRAPH_VIT_GET(parentsIter2);
					igraph_integer_t forkSink2 = IGRAPH_VIT_GET(childrenIter2);
					
					if((int) forkHead == (int) forkHead2 && (int) forkSink == (int) forkSink2){ //and has the same parent and child as task i
						tasksInFork->push_back(j); //it is part of the same fork-join
					}
				}
				
				igraph_vit_destroy(&parentsIter2);
				igraph_vit_destroy(&childrenIter2);
				igraph_vs_destroy(&parents2);
				igraph_vs_destroy(&children2);	
			}
			if(tasksInFork->size() > perLevel){
				std::vector<struct taskBin *> * taskBins = clusterByRuntime(newGraph, tasksInFork, perLevel, noBinRestrictions);
					for(int i = 0; i < taskBins->size(); i++){
						combineMulti(newGraph, &(taskBins->at(i)->ids));
					}
					std::cout << "Found and merged a fork-join.\n";
				//memory cleanup
				for(int i = 0; i < taskBins->size(); i++){
					delete(taskBins->at(i));
				}
				delete(taskBins);
			}
			
		}
		
		
		igraph_vit_destroy(&parentsIter);
		igraph_vit_destroy(&childrenIter);
		igraph_vs_destroy(&parents);
		igraph_vs_destroy(&children);
		delete(tasksInFork);
	}
	if(config.mergeChainsAfter){ //auto-combine chains after all other changes, if desired
		while(mergeAChain(newGraph));
	}
	
	outputDAXStr(dagToDAXStr(newGraph));
	return newGraph;
}

/**
Method that calculates the makespan of a set of tasks based on their components.
It then sets the value of how many processors were used to achieve this runtime, 
and the runtime itself. It also checks to make sure that the efficiency is at or below
the specified amount in the global settings. If not, it repeats, but with fewer processors
until the proper efficiency is ahcieved.
*/
void getMultiprocRuntime(std::vector<std::string> * subgraphIDs, int * procResult, double * runtimeResult){
	double efficiency = -1;
	int maxProcs = config.maxProcs + 1;
	while(efficiency < config.minProcEfficiency){
		maxProcs--;
		igraph_vector_t * igraphVectorSub = new igraph_vector_t;
		igraph_vector_init(igraphVectorSub, subgraphIDs->size());
		for(int i = 0; i < subgraphIDs->size(); i++){
			igraph_vector_set(igraphVectorSub, i, findVertexID(config.original_graph, subgraphIDs->at(i)));
		}	
		igraph_t * subgraph = new igraph_t;
		//this subgraph^ can be destroyed without affecting the original graph, as per documentaiton
		igraph_vs_t subgraphSelector;
		igraph_vs_vector(&subgraphSelector, igraphVectorSub);
		igraph_induced_subgraph(config.original_graph, subgraph, subgraphSelector, IGRAPH_SUBGRAPH_AUTO);
		//subgraph is now the subgraph of the component ids being combined together.
		//First, calculate the "longest distance to completion" values for each one, like a reverse levelLabel.
		//This method will also calculate the number of "dependencies" a graph has before it can run. This will be
		//useful for the "ready" list later
		igraph_integer_t head = calculateB_levels(subgraph);
		double cumulativeRuntime = 0;
		//calculate total runtime of graph
		for(int i = 0; i < igraph_vcount(subgraph); i++){
			cumulativeRuntime += VAN(subgraph,"runtime",i);
		}
		std::vector<struct taskBin *> procs;
		for(int i = 0; i < maxProcs; i++){
			procs.push_back(new taskBin);
		}	
		std::vector<igraph_integer_t> ready;
		ready.push_back(head);
		while(!ready.empty()){
			//finds the next ready task with the longest critical path.
			int indexOfLargestB = 0;
			for(int i = 1; i < ready.size(); i++){
				if(VAN(subgraph, "b-level",ready.at(indexOfLargestB)) < VAN(subgraph, "b-level",ready.at(i))){
					indexOfLargestB = i;
				}	
			}
			igraph_integer_t nextTask = ready.at(indexOfLargestB);
			ready.erase(ready.begin() + indexOfLargestB); //removes it from the ready list.
			
			//finds the best proc to add it in based on its "earliest start time"
			int bestProcIndex = 0;
			int bestProcStart;
			if(procs.at(0)->totalRuntime < VAN(subgraph,"est",nextTask)){
				bestProcStart = VAN(subgraph,"est",nextTask); //the proc is free BEFORE the earliest start time
			}
			else {
				bestProcStart = procs.at(0)->totalRuntime; //the proc is free AT or AFTER the earliest start time
			}
			
			//Checks for either the "lowest" index proc that starts at or before est, 
			//or the proc that is free at the earliest time.
			for(int i = 1; i < procs.size(); i++){
				if(bestProcStart > procs.at(i)->totalRuntime && bestProcStart > VAN(subgraph,"est",nextTask)){
					bestProcIndex = i;
					bestProcStart = procs.at(i)->totalRuntime;
				}
			}
			//adds the task to the proc and updates its makespan.
			
			double newMakespan = 0;
			//If the proc is available BEFORE the est, then the new makespan is dependent on when
			//the new task starts and how long it takes to start.
			if(procs.at(bestProcIndex)->totalRuntime < VAN(subgraph,"est",nextTask)){
				newMakespan = VAN(subgraph,"est",nextTask) + VAN(subgraph,"runtime",nextTask);
			}
			//otherwise, it's available AFTER the est, so its new makespan is its current makespan + runtime of task.
			else{
				newMakespan = procs.at(bestProcIndex)->totalRuntime + VAN(subgraph,"runtime",nextTask);
			}
			procs.at(bestProcIndex)->totalRuntime = newMakespan;
			
			//update readyList
			//get all vertices that were dependent on the task added.
			igraph_vs_t children;
			igraph_vit_t childrenIter;
			igraph_vs_adj(&children,nextTask,IGRAPH_OUT);
			igraph_vit_create(subgraph,children,&childrenIter);
			
			while(! IGRAPH_VIT_END(childrenIter) ){
				igraph_integer_t child = IGRAPH_VIT_GET(childrenIter);
				//update earliest start time if their est must now be later due to the later end time of a parent task
				if(VAN(subgraph,"est",child) < newMakespan){
					SETVAN(subgraph,"est",child,newMakespan);
				}
				//decrement dependencies by 1, since that task was added to the processors.
				int dependencies = VAN(subgraph,"dependencies",child);
				dependencies--;
				SETVAN(subgraph,"dependencies",child, dependencies);
				//if there are now exactly 0 dependencies, then all previous tasks have been added to the procs.
				if(dependencies == 0){
					ready.push_back(child);
				}
				
				IGRAPH_VIT_NEXT(childrenIter);
			}
		
		
			igraph_vs_destroy(&children);
			igraph_vit_destroy(&childrenIter);
		}
	
		//return with final results.
		*procResult = 1;
		*runtimeResult = procs.at(0)->totalRuntime;
		for(int i = 1; i < procs.size(); i++){
			if(procs.at(i)->totalRuntime > 0){ //don't count processors that are unused.
				*procResult = i + 1;
				if(procs.at(i)->totalRuntime > *runtimeResult){
					*runtimeResult = procs.at(i)->totalRuntime;
				}
			}
		}
		//do-over if the efficiency isn't up to par with specifications.
		double runtimeTimesProcs = *runtimeResult * *procResult;
		
		efficiency = cumulativeRuntime / runtimeTimesProcs;
		for(int i = 0; i < procs.size(); i++){
			delete(procs.at(i));
		}
		igraph_vector_destroy(igraphVectorSub);
		delete(igraphVectorSub);
		igraph_destroy(subgraph);
		delete(subgraph);
	}
}

/**Calculates the longest amount of time needed to reach the sink node.
A lot like a level, but instead of each edge being weighted 1, the node runtime
is the weight used to calculate a B-level.
*/
igraph_integer_t calculateB_levels(igraph_t * graph){
	igraph_integer_t sink = makeSink(graph);
	igraph_integer_t head = makeHead(graph);
	for(int i = 0; i < igraph_vcount(graph); i++){
		SETVAN(graph,"b-level",i,0);
		SETVAN(graph,"dependencies",i,0);
		SETVAN(graph,"est",i,0);
	}
	std::queue<igraph_integer_t> que;
	que.push(sink); //start at the bottom
	while(!que.empty()){
		igraph_integer_t nextNode = que.front();
		que.pop();
		int distToFinish = VAN(graph,"b-level",nextNode);
		igraph_vs_t parents;
		igraph_vit_t parentIter;
		igraph_vs_adj(&parents,nextNode,IGRAPH_IN);
		igraph_vit_create(graph,parents,&parentIter);
		SETVAN(graph,"dependencies",nextNode,IGRAPH_VIT_SIZE(parentIter));
		while(! IGRAPH_VIT_END(parentIter)){
			igraph_integer_t parent = IGRAPH_VIT_GET(parentIter);
			if(VAN(graph,"b-level",parent) < distToFinish + VAN(graph,"runtime",parent)){ 
			//if the parent's level is smaller than the child's distance to finish + parent's runtime...
			//Set the parent's b-level to the new, higher value, as it is the new critical path
				SETVAN(graph,"b-level",parent,distToFinish + VAN(graph,"runtime",parent));
				que.push(parent); //push the parent back onto the queue to update its parents as well.
			}
			IGRAPH_VIT_NEXT(parentIter);
		}
		igraph_vs_destroy(&parents);
		igraph_vit_destroy(&parentIter);
	}
	return head;
}

/**
Does nothing except the optional chainMerge flag, and outputs and returns the result.
*/
igraph_t * noOp(igraph_t * graph){
	igraph_t * newGraph = new igraph_t;
	igraph_copy(newGraph, graph);
	
	if(config.mergeChainsAfter || config.mergeChainsBefore){ //auto-combine chains after all other changes, if desired
		while(mergeAChain(newGraph));
	}
	
	outputDAXStr(dagToDAXStr(newGraph));
	return newGraph;
}

/**
Customized method of clustering, in the format
"Cluster1A,cluster1B,cluster1C:cluster2A,cluster2B" and so on. In other words,
tasks to be clustered together are separated by commas, and cluster groups are 
separated by colons. */
igraph_t * customClustering(igraph_t * graph, std::string idList){
	std::vector<std::string> combinations;
	split(idList, ':', &combinations);
	igraph_t * newGraph = new igraph_t;
	igraph_copy(newGraph, graph);
	for(int i = 0; i < combinations.size(); i++){
		std::vector<std::string> sets;split(combinations.at(i), ',', &sets);
		combineMulti(newGraph, &sets);
	}
	
	outputDAXStr(dagToDAXStr(newGraph));
	return newGraph;
}

/**Used for debug, resets the permCount to 0.
*/
void reset(){
	permCount = 0;
}