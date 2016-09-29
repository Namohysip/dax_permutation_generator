#include <igraph.h>
#include <iostream>
#include <vector>

void printEdges(igraph_t*);
void printNodes(igraph_t*);
void combine(igraph_t * , igraph_integer_t, igraph_integer_t);
bool addWithoutDuplicates( std::vector<igraph_t *> * , igraph_t *);
std::vector<igraph_t *> exhaustivePermStart(igraph_t *);

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
	 printEdges(&another);
	 printNodes(&another);
	 
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
	 
	 std::cout << "The original graph shouldn't have been affected: \n";
	 
	 printEdges(&graph);
	 printNodes(&graph);
	 
	 std::vector<igraph_t *> list;
	 bool result = addWithoutDuplicates(&list, &graph);
	 result = addWithoutDuplicates(&list, &another);
	 
	 std::cout << "Adding two different graphs into the list. Size: " << list.size() << "\n";
	 
	 addWithoutDuplicates(&list, &another);
	 
	 std::cout << "Adding another, duplicate graph into the list. Size: " << list.size() << "\n";
	 

	 
	 
	 igraph_t notDuplicate;
	 igraph_copy(&notDuplicate, &another);
	 SETVAN(&notDuplicate, "runtime", 0, 30);
	 addWithoutDuplicates(&list, &notDuplicate);
	 std::cout << "Added something with same nodecount but different attributes" << list.size() << "\n";
	 
	 /*
	 igraph_t vbasic;
	 igraph_empty(&vbasic, 0, IGRAPH_DIRECTED);
	 igraph_add_vertices(&vbasic, 4, 0);
	 SETVAN(&vbasic, "id", 0, 0);
	 SETVAN(&vbasic, "id", 1, 1);
	 SETVAN(&vbasic, "id", 2, 2);
	 SETVAN(&vbasic, "id", 3, 3);
	 SETVAN(&vbasic, "runtime", 0, 60);
	 SETVAN(&vbasic, "runtime", 1, 60);
	 SETVAN(&vbasic, "runtime", 2, 60);
	 SETVAN(&vbasic, "runtime", 3, 60);
	 igraph_add_edge(&vbasic, 0, 1);
	 igraph_add_edge(&vbasic, 0, 2);
	 igraph_add_edge(&vbasic, 1, 3);
	 igraph_add_edge(&vbasic, 2, 3);
	 std::vector<igraph_t *> vexhaust = exhaustivePermStart(&vbasic);
	 std::cout << vexhaust.size();
	 
	 for(std::vector<igraph_t*>::iterator it = vexhaust.begin(); it != vexhaust.end(); ++it){
		printNodes(*it);
		printEdges(*it);
	} */
	 /*
	 	 std::cout << "Now testing exhaustivePerm on basic DAG structure: \n";
	 igraph_t basic;
	 
	 igraph_empty(&basic, 0, IGRAPH_DIRECTED);
	 igraph_add_vertices(&basic, 8, 0);
	 SETVAN(&basic, "id", 0, 0);
	 SETVAN(&basic, "id", 1, 1);
	 SETVAN(&basic, "id", 2, 2);
	 SETVAN(&basic, "id", 3, 3);
	 SETVAN(&basic, "id", 4, 4);
	 SETVAN(&basic, "id", 5, 5);
	 SETVAN(&basic, "id", 6, 6);
	 SETVAN(&basic, "id", 7, 7);
	 SETVAN(&basic, "runtime", 0, 60);
	 SETVAN(&basic, "runtime", 1, 60);
	 SETVAN(&basic, "runtime", 2, 60);
	 SETVAN(&basic, "runtime", 3, 60);
	 SETVAN(&basic, "runtime", 4, 60);
	 SETVAN(&basic, "runtime", 5, 60);
	 SETVAN(&basic, "runtime", 6, 60);
	 SETVAN(&basic, "runtime", 7, 60);
	 
	 igraph_add_edge(&basic, 0, 1);
	 igraph_add_edge(&basic, 0, 2);
	 igraph_add_edge(&basic, 0, 3);
	 igraph_add_edge(&basic, 1, 4);
	 igraph_add_edge(&basic, 2, 5);
	 igraph_add_edge(&basic, 3, 6);
	 igraph_add_edge(&basic, 4, 7);
	 igraph_add_edge(&basic, 5, 7);
	 igraph_add_edge(&basic, 6, 7);
	 
	 std::vector<igraph_t *> exhaust = exhaustivePermStart(&basic);
	 std::cout << exhaust.size();
	 */
	 
	 igraph_t stress;
	 int max = 14;
	 std::cout << "Simple stress-testing: series graph of size " << max << ":\n";
	 igraph_empty(&stress, 0, IGRAPH_DIRECTED);
	 igraph_add_vertices(&stress, max, 0);
	 SETVAN(&stress, "id", 0, 0);
	 SETVAN(&stress, "runtime", 0, 60);
	 for ( int i = 1; i < max; i++){
		 SETVAN(&stress, "id", i, i);
		 SETVAN(&stress, "runtime", i, 60);
		 igraph_add_edge(&stress, i-1, i);
	 }
	 
	 std::vector<igraph_t *> stressList = exhaustivePermStart(&stress);
	 std::cout << stressList.size();
	 igraph_vs_destroy(&del);
	 igraph_es_destroy(&edel);
	 igraph_destroy(&graph);
	 igraph_destroy(&another); ///be sure to clear up memory
 }
