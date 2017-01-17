#include <igraph.h>
#include <argp.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <fstream>

#include "workflow.hpp"
#include "DAGUtilities.hpp"
#include "PermutationMaker.hpp"


/**
 * Stores command-line arguments
 */
struct MainArguments
{
	bool just_a_test = false; 
	std::string dax_filename;
	std::string output_prefix;
	std::string method;	
	double timeout = 0.0;
	int max_permutations = 0;
	int attempt_cap = 20;
	int min_size = 1;
	bool hash = false;
	bool chaining = false;
	bool freeBinCapacity = true;
	int tasksPerLevel = 1;
	std::string custom;
	
	bool abort = false;  	  // set to true while parsing argument if error
	std::string abortReason;  // Error message to print

};

enum {
	TEST = 1000,
	HASH,
	INPUT,
	OUTPUT_PREFIX,
	TIMEOUT,
	MAX_PERMUTATIONS,
	METHOD,
	MIN_SIZE,
	ATTEMPT_CAP,
	CHAINMERGE,
	TASKSPERLEVEL,
	EVENNUMTASKSPERBIN,
	CUSTOM
	
};
/**
 * Parse command-line arguments
 */

int parse_opt(int key, char *arg, struct argp_state *state)
{

    MainArguments * main_args = (MainArguments *) state->input;

    switch (key)
    {
    case TEST:
    {
	main_args->just_a_test = true;
	break;
    }
	case HASH:
	{
		main_args->hash = true;
		break;
	}
	case CHAINMERGE:
	{
		main_args->chaining = true;
		break;
	}
	case EVENNUMTASKSPERBIN:
	{
		main_args->freeBinCapacity = false;
		break;
	}
    case INPUT:
    {
        if (access(((std::string) arg).c_str(), R_OK) == -1)
        {
            main_args->abort = true;
            main_args->abortReason += "\n  invalid DAX workflow file argument: file '" + 
                                      std::string(arg) + "' cannot be read";
        }
        else
        {
            main_args->dax_filename = std::string(arg);
        }
        break;
    }
    case OUTPUT_PREFIX:
    {
		main_args->output_prefix = std::string(arg);
        break;
    }
	case CUSTOM:
	{
		main_args->custom = std::string(arg);
		break;
	}
    case TIMEOUT:
    {
	if ((sscanf(arg, "%lf",&(main_args->timeout)) != 1) ||
            (main_args->timeout < 0)) {
	  main_args->abort = true;
	  main_args->abortReason += "\n  invalid timeout argument '" +
					std::string(arg) + "'";
 	} 
	break;
    }
    case MAX_PERMUTATIONS:
    {
	if ((sscanf(arg, "%d",&(main_args->max_permutations)) != 1) ||
            (main_args->max_permutations < 0)) {
	  main_args->abort = true;
	  main_args->abortReason += "\n  invalid max permutations argument '" +
					std::string(arg) + "'";
 	} 
	break;
    }
    case METHOD:
    {
	main_args->method = (std::string) arg;
	if ( 
	    (main_args->method != "exhaustive") &&
	    (main_args->method != "random") && 
	    (main_args->method != "randomLimited") &&
	    (main_args->method != "runtimeBalance") &&
	    (main_args->method != "impactFactorBalance") &&
	    (main_args->method != "distanceBalance") &&
	    (main_args->method != "custom") 
		) {
	  main_args->abort = true;
	  main_args->abortReason += "\n  invalid method argument '" + std::string(arg) + "'";
        }
	break;
    }
	case MIN_SIZE:
	{
		if ((sscanf(arg,"%d",&(main_args->min_size)) != 1) ||
			(main_args->min_size < 1)){
				main_args->abort = true;
				main_args->abortReason += "\n invalid minimum size value '" + std::string(arg) + "'";
			}
		break;
	}
	case ATTEMPT_CAP:
	{
		if((sscanf(arg,"%d",&(main_args->attempt_cap)) != 1) ||
			(main_args->attempt_cap < 1)){
				main_args->abort = true;
				main_args->abortReason += "\n invalid attempt cap value '" + std::string(arg) + "'";
			}
		break;
	}
	case TASKSPERLEVEL:
	{
		if ((sscanf(arg,"%d",&(main_args->tasksPerLevel)) != 1) ||
			(main_args->tasksPerLevel < 1)){
				main_args->abort = true;
				main_args->abortReason += "\n invalid minimum size value '" + std::string(arg) + "'";
			}
		break;
	}
    } // end switch

    return 0;
}

/**
 * @brief Parses Batsim's command-line arguments
 * @param[in] argc The number of arguments given to the main function
 * @param[in] argv The values of arguments given to the main function
 * @param[out] main_args Batsim's usable arguments
 * @return
 */
bool parse_main_args(int argc, char * argv[], MainArguments & main_args)
{
	std::string method_description = "\nWorkflow generation  method. String, exactly one required from the following: \n" 
						"\nexhaustive: Will exhaustively find all possible clustering options for the input workflow. " 
						"Can me modified with hash, timeout, max-permutations, min-size, and chain-merge options for better performance or more specialized output.\n\n" 
						"random: will randomly combine two tasks until time runs out or a number of clustering options were generated. --hash will have no effect. " 
						"Unlike exhaustive, random will ALWAYS use a timeout and max-permutations value.\n\n" 
						"randomLimited: A specialized version of a randomized clustering designed for a more evenly-distributed set of clusterings, from fine grained to coarse grained. Uses a specialized option, " 
						"attempt-cap, which is how many times each iteration will attempt to make a new clustering using one of the graphs. If the cap is reached, that graph will no longer be used, and there will be" 
						" fewer clustering options for the more coarse-grained clusterings. Also, max-permutations is used instead as how many clusterings per size instead.\n\n"
						"runtimeBalance: A method of clustering from other literature, where tasks at the same level of a graph are clustered based on balancing their runtime, so each clustered task is as close to"
						" the same as possible using a greedy bin-packing algorithm. For this and other 'balance' methods, --even-numtasks-per-bin can be used to make each bin have the same number of tasks, +/- 1.\n\n"
						"impactFactorBalance: Another horizontal clustering method that clusters tasks based on their impact factor. Clusters first based on the 'impact factor,' based on how much influence or 'weight' the "
						"task has on the graph, based on its dependencies. It then clusters, based on identical impact factors, by runtime.\n\n"
						"distanceBalance: Another horizontal clustering method that clusters tasks on the same level based on how 'close' their most common successor is. Then, based on the tasks that are closest together, "
						"they are then combined by runtime.\n\n"
						"custom: Given a specific graph, combine in a certain way based on parameters passed on with the --custom argument. The syntax uses the task ids as the tasks to combine. Every set of ids separated by "
						"commas , will be combined, such that the first task id remains, with the others merged in. Things separated by colons :  are individual clustering assignments. For example, taskA,taskB,taskC:taskD,taskE "
						"is a command to combine task B and C into A, and to combine E into D.\n\n";
    struct argp_option options[] = 
    {
        {"dax", INPUT, "FILENAME", 0, "The input DAX file with the original workflow (string, required)", 0},
        {"output", OUTPUT_PREFIX, "PATH_PREFIX", 0, "The filepath prefix for generating output DAX files (string, required)", 0},
        {"timeout", TIMEOUT, "SECONDS", 0, "Time out value (float, default: 0 - no timeout)", 0},
        {"max-permutations", MAX_PERMUTATIONS, "COUNT", 0, "Maximum number of permutations to generate (integer, default: 0 - no maximum)", 0},
        {"method", METHOD, "NAME", 0, method_description.c_str(), 0},
        {"test", TEST, 0, 0, "Only run a hard-coded test for iGraph", 0},
		{"hash", HASH, 0, 0, "Use hashing if applicable. Silently does nothing if hashing is unavailable for the given method.", 0},
		{"even-numtasks-per-bin", EVENNUMTASKSPERBIN,0,0,"use this flag if you want horizontally clustered tasks on the same level to have the same number of tasks, rergardless of total runtime."},
		{"min-size", MIN_SIZE,"min_size",0,"the minimum size for workflows to be output. Must be above 0. Default is 1."},
		{"attempt-cap",ATTEMPT_CAP,"attempt_cap",0,"the maximum number of attempts to make a legal graph when done randomly."},
		{"chain-merge", CHAINMERGE,0,0,"use this if you want to always merge single-parent-single-child vertices"},
		{"tasks-per-level", TASKSPERLEVEL,"TASKSPERLEVEL",0,"use this for the horizontal clustering options. It is the maximum number of tasks wanted at each level."},
		{"custom", CUSTOM,"CUSTOMSTRING",0,"use this in conjunction with the 'custom' method. For syntax, see the 'custom' description under methods."},
		
		{0, '\0', 0, 0, 0, 0} // The options array must be NULL-terminated
    };

    struct argp argp = {options, parse_opt, 0, "A tool to generate workflow configurations via task merging.", 0, 0, 0};

    // Parse argument
    argp_parse(&argp, argc, argv, 0, 0, &main_args);

    // Check we have everything we need (unless this is all the hardcoded test)
    // Check we have everything we need (unless this is all the hardcoded test)
    if (! main_args.just_a_test) {
      // We need a DAX as input
      if (main_args.dax_filename == "") {
        main_args.abort = true;
        main_args.abortReason += "\n  missing DAX file argument (--dax)";
      }
      // We need an output prefix 
      if (main_args.output_prefix == "") {
        main_args.abort = true;
        main_args.abortReason += "\n  missing output prefix argument (--output)";
      }
      // We need a method
      if (main_args.method == "") {
        main_args.abort = true;
        main_args.abortReason += "\n  missing method argument (--method)";
      }

    }

    if (main_args.abort)
    {
        fprintf(stderr, "Aborting:%s\n", main_args.abortReason.c_str());
        return false;
    }

    return true;
}


int main (int argc, char* argv[]) {

	MainArguments main_args;
        if (!parse_main_args(argc, argv, main_args))
            return 1;

	igraph_i_set_attribute_table(&igraph_cattribute_table); //ALWAYS HAVE THIS. Enables attributes.

	if (main_args.just_a_test) {
		test_with_small_hardcoded_graph();	
		exit(0);
	}

	// Getting command-line arguments
	std::string dax_file = main_args.dax_filename;
	std::string output_prefix = main_args.output_prefix;
	std::string method = main_args.method;
	double timeout = main_args.timeout;
	int max_permutations = main_args.max_permutations;
	int attempt_cap = main_args.attempt_cap;
	bool hashed = main_args.hash;
	bool freeBinCapacity = main_args.freeBinCapacity;
	int tasksPerLevel = main_args.tasksPerLevel;
	std::string custom = main_args.custom;
	
	/* Load the workflow from the DAX file*/
	Workflow * workflow = new Workflow("some_workflow");
	if (workflow->load_from_xml(dax_file)) {
	  exit(1);
	}
	//printEdges(getGlobalSettings()->original_graph);
	//printNodes(getGlobalSettings()->original_graph);
	
	getGlobalSettings()->fileBase = main_args.output_prefix;
	getGlobalSettings()->maxGraphs = main_args.max_permutations;
	getGlobalSettings()->timeLimit = main_args.timeout;
	getGlobalSettings()->mergeChains = main_args.chaining;
	getGlobalSettings()->minSize = main_args.min_size;
	getGlobalSettings()->attemptCap = main_args.attempt_cap;
	
	std::cout << "Generating permutations...\n";
	std::vector<igraph_t *> * clusterings;
	if(method == "exhaustive"){
		if(hashed){
			/*Compute all transformations with hashing enabled to use up less memory */
			std::vector<std::string> * hashes = exhaustivePermHashStart(getGlobalSettings()->original_graph);
			std::cout << "Total permutations made: " << hashes->size() << "\n";
		}
		else
		{
			
			clusterings = exhaustivePermStart(getGlobalSettings()->original_graph);
			std::cout << "Total permutations made: " << clusterings->size() << "\n";
		}
	}
	else if(method == "random"){
		clusterings = randomizedPerm(getGlobalSettings()->original_graph);
		std::cout << "Total permutations made: " << clusterings->size() << "\n";
	}
	else if(method == "randomLimited"){
		std::cout << "Total permutations made: " << RandomizedPermEvenSpread(getGlobalSettings()->original_graph) << "\n";
	}
	else if(method == "runtimeBalance"){
		horizontalClustering(getGlobalSettings()->original_graph, tasksPerLevel, freeBinCapacity);
		std::cout << "Done!\n";
	}
	else if(method == "impactFactorBalance"){
		impactFactorClustering(getGlobalSettings()->original_graph, tasksPerLevel, freeBinCapacity);
		std::cout << "Done!\n";
	}else if(method == "distanceBalance"){
		distanceBalancedClustering(getGlobalSettings()->original_graph, tasksPerLevel, freeBinCapacity);
		std::cout << "Done!\n";
	}
	else if (method == "custom"){
		customClustering(getGlobalSettings()->original_graph, custom);
		std::cout << "Done!\n";
	}
	else {
		std::cerr << "This type of clustering option is not supported.\n";
	}
	exit(0);
}


