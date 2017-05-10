#include <igraph.h>
#include <iostream>
#include <vector>
#include <sstream>
#include "workflow.hpp"
#include <string>
#include <fstream>
#include <string.h>
#include <unistd.h>

#include "DAGUtilities.hpp"
#include "PermutationMaker.hpp"

void test_with_small_hardcoded_graph();

double calculateSD(double data[], int size);

void test_with_small_hardcoded_graph() {
/*
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
	 const char * cmp = "components";
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
	 SETVAS(&graph, cmp, 0,"0");
	 SETVAS(&graph, cmp, 1,"1");
	 SETVAS(&graph, cmp, 2,"2");
	 SETVAS(&graph, cmp, 3,"3");
	 SETVAS(&graph, cmp, 4,"4");
	 
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
	 
	getGlobalSettings()->original_graph = &another;
	 
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
	 int max = 30;
	 
	// std::cout << "Add using hash method for series graph of size " << std::to_string(max) << "\n";
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
	delete(graphs);
	 std::vector<igraph_t *> * notHashed = exhaustivePermStart(&hash, true, 100, 5000);
	std::cout << notHashed->size() << "\n";
	for(std::vector<igraph_t *>::iterator it = notHashed->begin(); it != notHashed->end(); ++it){
		igraph_destroy(*it);
	}
	notHashed->clear();
	delete(notHashed); 
	 igraph_destroy(&another); ///be sure to clear up memory
	 igraph_destroy(&graph);
	 igraph_vs_destroy(&del);
	 igraph_es_destroy(&edel);
	 igraph_destroy(&notDuplicate);
	 igraph_destroy(&hash);
	*/
	getGlobalSettings()->maxGraphs = 2000;
	getGlobalSettings()->timeLimit = 600;
	getGlobalSettings()->fileBase = "output/test";
	
	std::string files[] = {"workflows/DAG.xml","workflows/Epigenomics_1.xml","workflows/1000genome.xml"};
	std::string methods[] = {"Exhaustive","Hashed Exhaustive","Random","Random Even-Spread","HRB","HIFB","HDB","Fork-Join"};
	igraph_t * import = getGlobalSettings()->original_graph;
	int trials = 10;
	std::ofstream resultsFile("results.txt");
	for(int fileName = 0; fileName < 3; fileName++){
		resultsFile << "~~~~~~~~~~~~~~~~~~~\r\n" << files[fileName] << ":\r\n";
		Workflow * workflow = new Workflow("some_workflow");
		if (workflow->load_from_xml(files[fileName])) {
		  exit(1);
		} 
		double times[trials];
		for(int procs = 1; procs < 5; procs += 3){
			resultsFile << "\r\n Efficienct: 0.75, processors: " << procs << "\r\n";
			for(int type = 3; type < 4; type++){
				double total = 0;
				resultsFile << "\r\n" << methods[type] << ":\r\n";
				for(int i = 0; i < trials; i++){
					clock_t start = clock(); //timer
					int count = 1;
					switch(type){
						case(0): 
						{
							std::vector<igraph_t*> * clusterings = exhaustivePermStart(getGlobalSettings()->original_graph);
							for(int j = 1; j < clusterings->size(); j++){
								igraph_destroy(clusterings->at(j));
							}
							clusterings->clear();
							delete(clusterings);
							break;
						}
						case(1):
						{
							std::vector<std::string> * hashes = exhaustivePermHashStart(getGlobalSettings()->original_graph);
							hashes->clear();
							delete(hashes);
							break;
						}
						case(2):
						{
							std::vector<igraph_t*> * clusterings = randomizedPerm(getGlobalSettings()->original_graph);
							for(int j = 1; j < clusterings->size(); j++){
								igraph_destroy(clusterings->at(j));
							}
							clusterings->clear();
							delete(clusterings);
							break;
						}
						case(3):
						{
							getGlobalSettings()->maxGraphs = 5;
							count = RandomizedPermEvenSpread(getGlobalSettings()->original_graph);
							resultsFile << "Amount created: " << count << "\r\n";
							getGlobalSettings()->maxGraphs = 2000;
							break;
						}
						case(4):
						{
							horizontalClustering(getGlobalSettings()->original_graph, 2, true);
							break;
						}
						case(5):
						{
							impactFactorClustering(getGlobalSettings()->original_graph, 2, true);
							break;
						}
						case(6):
						{
							distanceBalancedClustering(getGlobalSettings()->original_graph, 2, true);
							break;
						}
						case(7):
						{
							forkJoin(getGlobalSettings()->original_graph, 2,true);
							break;
						}
					default: {break;}
					}
					double time = double(clock() - start) / CLOCKS_PER_SEC;
					if(type != 3){
						times[i] = time;
					}
					else{
						times[i] = 2000 * (time / (double) count);
					}
					std::cout << "Time taken : " << time << "\n";
					resultsFile << "Time: " << time << "\r\n";
					total += times[i];
					reset();
					
				}
				double average = total / (double) trials;
				std::cout << "Average time: " << average << "\n";
				double SD = calculateSD(times, trials);
				std::cout << "Standard Deviation: " << SD << "\n";
				double percentDiff = 100 * SD / average;
				std::cout << "% diff of SD and average: " << percentDiff << "%\n";
				resultsFile << "Average: " << average << "\r\nStandard Deviation: " << SD << "\r\nPercentDiff: " << percentDiff << "\r\n";
			}
		}
	}
	
	/*
	printEdges(getGlobalSettings()->original_graph);
	igraph_integer_t head = levelLabel(getGlobalSettings()->original_graph);
	std::cout << VAS(getGlobalSettings()->original_graph,"id",head) << "\n";
	std::cout << VAS(getGlobalSettings()->original_graph,"id",(int) head + 1) << "\n";
	
	std::cout << "for 'hash' graph: \n";
	head = levelLabel(&hash);
	std::cout << VAS(&hash,"id",head) << "\n";
	std::cout << VAS(&hash,"id",(int) head + 1) << "\n";
	
	
	*/

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

	resultsFile.close();
}
/*Taken and modified from programiz*/
double calculateSD(double data[], int size)
{
    double sum, mean, standardDeviation = 0.0;

    int i;

    for(i = 0; i < size; ++i)
    {
        sum += data[i];
    }

    mean = sum/size;

    for(i = 0; i < size; ++i)
        standardDeviation += pow(data[i] - mean, 2);

    return sqrt(standardDeviation / (size - 1));
}