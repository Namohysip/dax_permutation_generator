#include <igraph.h>
#include <algorithm>
#include <iostream>

void combine(igraph_t * G, igraph_integer_t node1, igraph_integer_t node2){
	int newRuntime = VAN(G, "runtime", node1) + VAN(G, "runtime", node2);
	int newId = std::min(VAN(G, "id", node1), VAN(G, "id", node2));
	
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
			igraph_add_edge(G, node1, child); 
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
		
	    std::cout << eid << "\n";
		igraph_delete_edges(G, edel);
		
		if (! ( (int) node1 == (int) parent)){ //ignore self-loops
			igraph_add_edge(G, node1, parent); 
		}
		IGRAPH_VIT_NEXT(inIter2); //go to next in-vertex
		igraph_es_destroy(&edel);
	}
	igraph_vs_t del;
	igraph_vs_1(&del, node2);
	igraph_delete_vertices(G, del);
	SETVAN(G, "runtime", node1, newRuntime);
	SETVAN(G, "id", node1, newId);
	
	igraph_vs_destroy(&del);
	igraph_vit_destroy(&outIter2);
	igraph_vit_destroy(&inIter2);
	igraph_vs_destroy(&out2);
	igraph_vs_destroy(&in2);
}
