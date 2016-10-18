#include <igraph.h>
#include <iostream>
#include <vector>
#include <sstream>
#include "workflow.hpp"
#include <string>

void printEdges(igraph_t*);
void printNodes(igraph_t*);
void combine(igraph_t * , igraph_integer_t, igraph_integer_t);
bool addWithoutDuplicates( std::vector<igraph_t *> * , igraph_t *);
std::vector<igraph_t *> * exhaustivePermStart(igraph_t * graph, bool timed = false, double seconds = 0, int goal = 0);
igraph_t * getImported();
void outputDAX(std::vector<igraph_t *> *, std::string);
std::vector<std::string> * exhaustivePermHashStart(igraph_t * graph,  std::string fileBase, double seconds = 0, int goal = 0);

void test_with_small_hardcoded_graph();


int main (int argc, char* argv[]) {
	 
	igraph_i_set_attribute_table(&igraph_cattribute_table); //ALWAYS HAVE THIS. Enables attributes.
	std::cout << "Test\n";
	// Test mode
	if ((argc == 2) && (std::string(argv[1]) == "test")) {
		test_with_small_hardcoded_graph();	
		exit(0);
	}

	// Checking command-line arguments
	char *dax_file = argv[1];
	char *output_prefix = argv[2];
	double timeout;
	int max_permutations;
	int hashVal; //whether or not to use hashing
	if((argc != 6) || 
           (sscanf(argv[3],"%lf",&timeout) != 1) ||
           (sscanf(argv[4],"%d",&max_permutations) != 1) ||
		   (sscanf(argv[5],"%d", &hashVal) != 1)
	  ) {
		std::cerr << "Usage:\n";
		std::cerr << "  " << argv[0] << " test\n";
		std::cerr << "  \truns a simple hardcoded test example\n\n";
		std::cerr << "  " << argv[0] << " <dax file> <output file prefix> <timeout (floating point)> <max # of permutations (int)> [hash (int, nonzero to hash)]\n";
		std::cerr << "  \tExample: " << argv[0] << "./my_dax.xml /tmp/transformed 60.0 1000 42\n";
		std::cerr << "               (will generate files /tmp/transformed_1.xml, /tmp/transformed_2.xml, ...)\n\n";
		exit(1);
	}



	/* Load the workflow from the DAX file*/
	Workflow * workflow = new Workflow("some_workflow");
	if (workflow->load_from_xml(dax_file)) {
	  exit(1);
	}
	//printEdges(getImported());
	//printNodes(getImported());
	std::cout << "Generating permutations...\n";
	if(hashVal){
		/*Compute all transformations with hashing enabled to use up less memory */
		std::vector<std::string> * hashes = exhaustivePermHashStart(getImported(), argv[2], timeout, max_permutations);
		std::cout << "Total permutations made: " << hashes->size() << "\n";
	}
	else{
		/* Compute all transformations */
		std::vector<igraph_t *> * totalExhaust = exhaustivePermStart(getImported(), true, timeout, max_permutations);
		std::cout << "Total permutations made: " << totalExhaust->size() << "\n";
		 
		/* Output the transformed DAX files */
		outputDAX(totalExhaust, argv[2]);
	}
	exit(0);
}


void test_with_small_hardcoded_graph() {

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
	 SETVAS(&graph, id, 0,"0");
	 SETVAS(&graph, id, 1,"1");
	 SETVAS(&graph, id, 2,"2");
	 SETVAS(&graph, id, 3,"3");
	 SETVAS(&graph, id, 4,"4");
	 
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

	 
	 igraph_t hash;
	 int max = 8;
	 
	 std::cout << "Add using hash method for series graph of size " << std::to_string(max) << "\n";
	 igraph_empty(&hash, 0, IGRAPH_DIRECTED);
	 igraph_add_vertices(&hash, max, 0);
	 SETVAS(&hash, "id", 0, "0");
	 SETVAN(&hash, "runtime", 0, 60);
	 for ( int i = 1; i < max; i++){
		std::stringstream con;
		con << i;
		std::string str = con.str();
		 SETVAS(&hash, "id", i, str.c_str());
		 SETVAN(&hash, "runtime", i, 60);
		 igraph_add_edge(&hash, i-1, i);
	 }
	
	std::vector<std::string> * graphs = exhaustivePermHashStart(&hash, "test", 100, 5000);
	 std::cout << graphs->size() << "\n";
	graphs->clear();
	std::cout << "No problems so far \n";
	delete(graphs);
	 std::vector<igraph_t *> * notHashed = exhaustivePermStart(&hash, true, 100, 5000);
	std::cout << notHashed->size() << "\n";
	for(std::vector<igraph_t *>::iterator it = notHashed->begin(); it != notHashed->end(); ++it){
		igraph_destroy(*it);
	}
	notHashed->clear();
	delete(notHashed); 
	
	 igraph_vs_destroy(&del);
	 igraph_es_destroy(&edel);
	 igraph_destroy(&notDuplicate);
	 igraph_destroy(&hash);
	 igraph_destroy(&another); ///be sure to clear up memory
	 igraph_destroy(&graph);

	 /*
	 
	 igraph_t vbasic;
	 igraph_empty(&vbasic, 0, IGRAPH_DIRECTED);
	 igraph_add_vertices(&vbasic, 4, 0);
	 SETVAS(&vbasic, "id", 0, "0");
	 SETVAS(&vbasic, "id", 1, "1");
	 SETVAS(&vbasic, "id", 2, "2");
	 SETVAS(&vbasic, "id", 3, "3");
	 SETVAN(&vbasic, "runtime", 0, 60);
	 SETVAN(&vbasic, "runtime", 1, 60);
	 SETVAN(&vbasic, "runtime", 2, 60);
	 SETVAN(&vbasic, "runtime", 3, 60);
	 igraph_add_edge(&vbasic, 0, 1);
	 igraph_add_edge(&vbasic, 0, 2);
	 igraph_add_edge(&vbasic, 1, 3);
	 igraph_add_edge(&vbasic, 2, 3);
	 std::vector<igraph_t *> * vexhaust = exhaustivePermStart(&vbasic, false);
	 std::cout << vexhaust->size();
	 
	 for(std::vector<igraph_t*>::iterator it = vexhaust->begin(); it != vexhaust->end(); ++it){
		printNodes(*it);
		printEdges(*it);
	} 
	 */
	 /*
	 	 std::cout << "Now testing exhaustivePerm and file output on basic DAG structure: \n";
	 igraph_t basic;
	 
	 igraph_empty(&basic, 0, IGRAPH_DIRECTED);
	 igraph_add_vertices(&basic, 8, 0);
	 SETVAS(&basic, "id", 0, "0");
	 SETVAS(&basic, "id", 1, "1");
	 SETVAS(&basic, "id", 2, "2");
	 SETVAS(&basic, "id", 3, "3");
	 SETVAS(&basic, "id", 4, "4");
	 SETVAS(&basic, "id", 5, "5");
	 SETVAS(&basic, "id", 6, "6");
	 SETVAS(&basic, "id", 7, "7");
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
	 
	 std::vector<igraph_t *> * exhaust = exhaustivePermStart(&basic, false);
	 std::cout << exhaust->size();
	 
	 outputDAX(exhaust, "permutationOutput/test");
	 */
	 /*
	 igraph_t stress;
	 int max = 10;
	 std::cout << "Simple stress-testing: series graph of size " << max << ":\n";
	 igraph_empty(&stress, 0, IGRAPH_DIRECTED);
	 igraph_add_vertices(&stress, max, 0);
	 SETVAS(&stress, "id", 0, "0");
	 SETVAN(&stress, "runtime", 0, 60);
	 for ( int i = 1; i < max; i++){
		std::stringstream con;
		con << i;
		std::string str = con.str();
		 SETVAS(&stress, "id", i, str.c_str());
		 SETVAN(&stress, "runtime", i, 60);
		 igraph_add_edge(&stress, i-1, i);
	 }
	 
	 std::vector<igraph_t *> * stressList = exhaustivePermStart(&stress, false);
	 std::cout << stressList->size(); 
	 
	 */

}
