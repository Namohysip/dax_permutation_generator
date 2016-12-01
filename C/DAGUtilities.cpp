#include <iostream>
#include <igraph.h>
#include <string.h>
#include "DAGUtilities.hpp"

//igraph_t * imported;

void printNodes(igraph_t* graph){
	long int i;
	
	for(i = 0; i < igraph_vcount(graph); i++){
		std::cout << "ID: " << VAS(graph, "id", i) << "," << VAN(graph, "runtime" ,i) << "\n";
	}
	
	
}

void printEdges(igraph_t* graph){
	long int i;
	
	long int j;
	
	igraph_es_t edges;
	igraph_es_all(&edges, IGRAPH_EDGEORDER_FROM);
	igraph_eit_t edgeIter;
	igraph_eit_create(graph, edges, &edgeIter);
	
	
	int headcount = 0;
	int sinkcount = 0;
	
	while (! IGRAPH_EIT_END(edgeIter)){
		igraph_integer_t eid = IGRAPH_EIT_GET(edgeIter);
		igraph_integer_t from;
		igraph_integer_t to;
		igraph_edge(graph, eid, &from, &to);
		
		std::cout << VAS(graph, "id", from) << "," << VAS(graph, "id", to) << "\n";
		if(strcmp(VAS(graph,"id",from), "TEMP_HEAD") == 0){
			headcount++;
		}
		if(strcmp(VAS(graph,"id",to), "TEMP_SINK") == 0){
			sinkcount++;
		}
		IGRAPH_EIT_NEXT(edgeIter);
	}
	if(headcount > 0 && sinkcount > 0){
	std::cout << "Number of heads: " << headcount << "\n";
	std::cout << "Number of sinks: " << sinkcount << "\n";
	}
	igraph_es_destroy(&edges);
	igraph_eit_destroy(&edgeIter);
}

void printNodesWithLevelsAndIF(igraph_t * graph){
	
	for(int i = 0; i < igraph_vcount(graph); i++){
		std::cout << "ID: " << VAS(graph, "id", i) << "," << VAN(graph, "runtime" ,i)
				<< " level: " << VAN(graph,"level",i) << " IF: " << VAN(graph,"IF",i) << "\n";
	}
}

/*
void setImported(igraph_t * graph){
	imported = graph;
}

igraph_t * getImported(){
	return imported;
}
*/
