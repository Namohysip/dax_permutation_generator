#include <iostream>
#include <igraph.h>

#include "DAGUtilities.hpp"

igraph_t * imported;
void printNodes(igraph_t* graph){
	igraph_vector_t gtypes, vtypes, etypes;
	igraph_strvector_t gnames, vnames, enames;
	long int i;
	
	long int j;
	
	igraph_vector_init(&gtypes, 0);
	igraph_vector_init(&vtypes, 0);
	igraph_vector_init(&etypes, 0);
	igraph_strvector_init(&gnames, 0);
	igraph_strvector_init(&vnames, 0);
	igraph_strvector_init(&enames, 0);
	
	igraph_cattribute_list(graph, &gnames, &gtypes, &vnames, &vtypes, &enames, &etypes);
	
	for(i = 0; i < igraph_vcount(graph); i++){
		std::cout << "ID: " << VAS(graph, "id", i) << "," << VAN(graph, "runtime" ,i) << "\n";
	}
	
	igraph_vector_destroy(&gtypes);
	igraph_vector_destroy(&vtypes);
	igraph_vector_destroy(&etypes);
	igraph_strvector_destroy(&gnames);
	igraph_strvector_destroy(&enames);
	igraph_strvector_destroy(&vnames);
	
}

void printEdges(igraph_t* graph){
	igraph_vector_t gtypes, vtypes, etypes;
	igraph_strvector_t gnames, vnames, enames;
	long int i;
	
	long int j;
	
	igraph_vector_init(&gtypes, 0);
	igraph_vector_init(&vtypes, 0);
	igraph_vector_init(&etypes, 0);
	igraph_strvector_init(&gnames, 0);
	igraph_strvector_init(&vnames, 0);
	igraph_strvector_init(&enames, 0);
	
	igraph_es_t edges;
	igraph_es_all(&edges, IGRAPH_EDGEORDER_FROM);
	igraph_eit_t edgeIter;
	igraph_eit_create(graph, edges, &edgeIter);
	
	
	
	
	while (! IGRAPH_EIT_END(edgeIter)){
		igraph_integer_t eid = IGRAPH_EIT_GET(edgeIter);
		igraph_integer_t from;
		igraph_integer_t to;
		igraph_edge(graph, eid, &from, &to);
		
		std::cout << VAS(graph, "id", from) << "," << VAS(graph, "id", to) << "\n";
		
		IGRAPH_EIT_NEXT(edgeIter);
	}
	
	igraph_vector_destroy(&gtypes);
	igraph_vector_destroy(&vtypes);
	igraph_vector_destroy(&etypes);
	igraph_strvector_destroy(&gnames);
	igraph_strvector_destroy(&enames);
	igraph_strvector_destroy(&vnames);
	igraph_es_destroy(&edges);
}

void setImported(igraph_t * graph){
	imported = graph;
}

igraph_t * getImported(){
	return imported;
}
