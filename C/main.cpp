#include <igraph.h>
#include <iostream>
#include <vector>
#include <sstream>
#include "workflow.hpp"
#include <string>
#include <string.h>

void printEdges(igraph_t*);
void printNodes(igraph_t*);
void combine(igraph_t * , igraph_integer_t, igraph_integer_t);
bool addWithoutDuplicates( std::vector<igraph_t *> * , igraph_t *);
std::vector<igraph_t *> * exhaustivePermStart(igraph_t * graph, bool timed = false, double seconds = 0, int goal = 0);
igraph_t * getImported();
void outputDAX(std::vector<igraph_t *> *, std::string);
std::vector<std::string> * exhaustivePermHashStart(igraph_t * graph,  std::string fileBase, double seconds = 0, int goal = 0);
std::vector<igraph_t *> * randomizedPerm(igraph_t * graph, double time, int max, std::string fileBase);
int RandomizedPermEvenSpread(igraph_t * graph, int maxPerLevel, std::string fileBase);

void test_with_small_hardcoded_graph();

int main (int argc, char* argv[]) {
	 std::string options = "exhaustive limited hashed random randomLimited";
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
           (sscanf(argv[4],"%d",&max_permutations) != 1)
	  ) {
		std::cerr << "Usage:\n";
		std::cerr << "  " << argv[0] << " test\n";
		std::cerr << "  \truns a simple hardcoded test example\n\n";
		std::cerr << "  " << argv[0] << " <dax file> <output file prefix> <timeout (floating point)> <max # of permutations OR base number of permutations for randomLimited (int)> <clustering method>\n";
		std::cerr << "Current options for clustering: " << options << "\n";
		std::cerr << "  \tExample: " << argv[0] << "./my_dax.xml /tmp/transformed 60.0 1000 hashed\n";
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
	std::vector<igraph_t *> * clusterings;
	if(!strcmp(argv[5], "exhaustive")){
		clusterings = exhaustivePermStart(getImported(), false, timeout, max_permutations);
		outputDAX(clusterings, argv[2]);
		
	}
	else if (!strcmp(argv[5], "limited")){
		clusterings = exhaustivePermStart(getImported(), true, timeout, max_permutations);
		std::cout << "Total permutations made: " << clusterings->size() << "\n";
		outputDAX(clusterings, argv[2]);
	}
	else if (!strcmp(argv[5], "hashed")){
		/*Compute all transformations with hashing enabled to use up less memory */
		std::vector<std::string> * hashes = exhaustivePermHashStart(getImported(), argv[2], timeout, max_permutations);
		std::cout << "Total permutations made: " << hashes->size() << "\n";
	}
	else if(!strcmp(argv[5], "random")){
		clusterings = randomizedPerm(getImported(), timeout, max_permutations, argv[2]);
		std::cout << "Total permutations made: " << clusterings->size() << "\n";
	}
	else if(!strcmp(argv[5], "randomLimited")){
		std::cout << "Total permutations made: " << RandomizedPermEvenSpread(getImported(), max_permutations, output_prefix) << "\n";
	}
	else {
		std::cerr << "This type of clustering option is not supported.\n";
		std::cerr << "Current options for clustering: " << options << "\n";
	}
	exit(0);
}


