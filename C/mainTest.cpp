#include <igraph.h>
#include <iostream>

void printEdges(igraph_t*);
void printNodes(igraph_t*);
void combine(igraph_t * , igraph_integer_t, igraph_integer_t);

 int main () {
	 
	 //How to construct a graph
	 igraph_i_set_attribute_table(&igraph_cattribute_table); //ALWAYS HAVE THIS. Enables attributes.
	 igraph_t graph;
	 igraph_t another;
	 
	 std::cout << "Original graph: \n";
	 igraph_empty(&graph, 0, IGRAPH_DIRECTED);
	 igraph_add_vertices(&graph, 5, 0);
	 igraph_add_edge(&graph, 0, 1);
	 igraph_add_edge(&graph, 1, 2);
	 igraph_add_edge(&graph, 2, 3);
	 igraph_add_edge(&graph, 3, 4);
	 
	 //how to set attributes
	 const char * runtime = "runtime";
	 const char * id = "id";
	 SETVAN(&graph, runtime, 3, 60); //This sets the runtime attribute to be 60 for vertex 3
	 SETVAN(&graph, runtime, 0, 60);
	 SETVAN(&graph, runtime, 1, 60);
	 SETVAN(&graph, runtime, 2, 60);
	 SETVAN(&graph, runtime, 4, 60);
	 SETVAN(&graph, id, 0,0);
	 SETVAN(&graph, id, 1,1);
	 SETVAN(&graph, id, 2,2);
	 SETVAN(&graph, id, 3,3);
	 SETVAN(&graph, id, 4,4);
	 
	 //How to copy graphs (including attributes!)
	 printEdges(&graph);
	 printNodes(&graph);
	 igraph_copy(&another, &graph);
	 std::cout << "And now for the copy: \n";
	 printEdges(&graph);
	 printNodes(&graph);
	 
	 //How to delete edges
	 //Note: Delete and modify edges BEFORE deleting a vertex so ids aren't messed up.
	 igraph_es_t edel;
	 igraph_integer_t eid;
	 igraph_get_eid(&graph, &eid, (igraph_integer_t) 0, (igraph_integer_t) 1, (igraph_bool_t) true, (igraph_bool_t) false);
	 igraph_es_1(&edel, eid);
	 igraph_delete_edges(&graph, edel);
	 std::cout << "edge (0,1) has been deleted \n";
	 printEdges(&graph);
	 printNodes(&graph);
	 
	 //How to delete vertices:
	 //WARNING: Deleting a vertex results in all vertex ids after the id deleted to decrement
	 igraph_vs_t del;
	 igraph_vs_1(&del, (igraph_integer_t) 2);
	 igraph_delete_vertices(&graph, del);
	 std::cout << "vertex 2 has been deleted \n";
	 printEdges(&graph);
	 printNodes(&graph);
	 
	 
	 
	 std::cout << "The graph copy: \n";
	 printEdges(&another);
	 printNodes(&another);
	 
	 
	 //Testing combine method:
	 std::cout << "Combining nodes 1 and 2 in the copy: \n";
	 combine(&another, 1, 2);
	 printEdges(&another);
	 printNodes(&another);
	 
	 igraph_vs_destroy(&del);
	 igraph_es_destroy(&edel);
	 igraph_destroy(&graph);
	 igraph_destroy(&another); ///be sure to clear up memory
 }
