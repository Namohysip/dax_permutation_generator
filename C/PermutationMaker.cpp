#include <igraph.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string.h>
#include <fstream>
#include <ctime>

void printNodes(igraph_t *);
void printEdges(igraph_t *);

void combine(igraph_t * G, igraph_integer_t node1, igraph_integer_t node2){
	double newRuntime = VAN(G, "runtime", node1) + VAN(G, "runtime", node2);
	//node1 retains its original ID
	
	igraph_vs_t out2;
	igraph_vs_t in2;
	igraph_vit_t outIter2;
	igraph_vit_t inIter2;
	
	igraph_vs_adj(&out2, node2, IGRAPH_OUT); //get all successors of node2
	igraph_vs_adj(&in2, node2, IGRAPH_IN); //get all predecessors of node2
	igraph_vit_create(G, out2, &outIter2);
	igraph_vit_create(G, in2, &inIter2);
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
			if (exists == -1) {
				igraph_add_edge(G, node1, child); 
			}
		}
		IGRAPH_VIT_NEXT(outIter2); //go to next out-vertex
		igraph_es_destroy(&edel);
	}
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
			if (exists == -1) {
				igraph_add_edge(G, parent, node1); 
			}
		}
		IGRAPH_VIT_NEXT(inIter2); //go to next in-vertex
		igraph_es_destroy(&edel);
	}
	igraph_vs_t del;
	igraph_vs_1(&del, node2);
	igraph_delete_vertices(G, del);
	SETVAN(G, "runtime", node1, newRuntime);
	
	igraph_vs_destroy(&del);
	igraph_vit_destroy(&outIter2);
	igraph_vit_destroy(&inIter2);
	igraph_vs_destroy(&out2);
	igraph_vs_destroy(&in2);
}

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

bool addWithoutDuplicates( std::vector<igraph_t *> * graphs, igraph_t * toAdd) {
	for(std::vector<igraph_t*>::iterator it = graphs->begin(); it != graphs->end(); ++it){
		if (checkAllNodes( toAdd, *it)){
			return false;
		}
	}
	graphs->push_back(toAdd);
	return true;
}


void exhaustivePerm(std::vector<igraph_t *> * graphs, igraph_t * graph){
	if(igraph_vcount(graph)< 2){
		return; //base case -- only one node left
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
				if(addWithoutDuplicates(graphs, newGraph)){
					exhaustivePerm(graphs, newGraph);
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
}

bool timedExhaustivePerm(std::vector<igraph_t *> * graphs, igraph_t * graph, double time, clock_t start, int goal){
	if(igraph_vcount(graph)< 2){
		return true; //base case -- only one node left
	}
	if (time < double(clock() - start) / CLOCKS_PER_SEC || graphs->size() > goal){ //time-up case -- return and wrap things up
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
				if(addWithoutDuplicates(graphs, newGraph)){
					if(! timedExhaustivePerm(graphs, newGraph, time, start, goal)){
						return false;
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

std::vector<igraph_t *> * exhaustivePermStart(igraph_t * graph, bool timed = false, double seconds = 0, int goal = 0){
	std::vector<igraph_t *> * graphs = new std::vector<igraph_t *>;
	graphs->push_back(graph);
	if (! timed) {
		exhaustivePerm(graphs, graph);
	}
	else {
		clock_t start = clock();
		timedExhaustivePerm(graphs, graph, seconds, start, goal);
	}
	return graphs;
}

void dagToDAX(igraph_t * graph, std::string filebase, int permCount){
	std::string name = filebase + "_p" + std::to_string(permCount) + ".xml";
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
	else std::cout << "Error writing file";
}

void outputDAX(std::vector<igraph_t *> * graphs, std::string fileBase){
	int i = 0;
	std::cout << "Writing out ";
	std::cout << graphs->size();
	std::cout << " files.\n";
	for(std::vector<igraph_t*>::iterator it = graphs->begin(); it != graphs->end(); ++it){
		dagToDAX(*it, fileBase, i);
		i++;
		if(i % 1000 == 0){
			std::cout << "Wrote out " << i << " files.\n";
		}
	}
}
