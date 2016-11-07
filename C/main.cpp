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
	ATTEMPT_CAP
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
		if(true){
			main_args->output_prefix = std::string(arg);
			std::cout << std::string(arg);
		}
		else {
			main_args->abort = true;
			main_args->abortReason += "\n  invalid output file base name: '" +
									  std::string(arg) + "' is invalid or directory does not exist.";
		}
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
	    (main_args->method != "randomLimited")) {
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
    struct argp_option options[] =
    {
        {"dax", INPUT, "FILENAME", 0, "The input DAX file with the original workflow (string, required)", 0},
        {"output", OUTPUT_PREFIX, "PATH_PREFIX", 0, "The filepath prefix for generating output DAX files (string, required)", 0},
        {"timeout", TIMEOUT, "SECONDS", 0, "Time out value (float, default: 0 - no timeout)", 0},
        {"max-permutations", MAX_PERMUTATIONS, "COUNT", 0, "Maximum number of permutations to generate (integer, default: 0 - no maximum)", 0},
        {"method", METHOD, "NAME", 0, "Workflow generation  method (string, required): limited, exhaustive, hashed, random, randomLimited", 0},
        {"test", TEST, 0, 0, "Only run a hard-coded test for iGraph", 0},
		{"hash", HASH, 0, 0, "Use hashing if applicable. Silently does nothing if hashing is unavailable for the given method.", 0},
		{"min_size", MIN_SIZE,"min_size",0,"the minimum size for workflows to be output. Must be above 0. Default is 1."},
		{"attempt_cap",ATTEMPT_CAP,"attempt_cap",0,"the maximum number of attempts to make a legal graph when done randomly."},
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
        main_args.abortReason += "\n  missing DAX file argument (-d)";
      }
      // We need an output prefix 
      if (main_args.output_prefix == "") {
        main_args.abort = true;
        main_args.abortReason += "\n  missing output prefix argument (-o)";
      }
      // We need a method
      if (main_args.method == "") {
        main_args.abort = true;
        main_args.abortReason += "\n  missing method argument (-m)";
      }

    }

    if (main_args.abort)
    {
        fprintf(stderr, "Aborting:%s\n", main_args.abortReason.c_str());
        return false;
    }

    return true;
}


void test_with_small_hardcoded_graph();

int main (int argc, char* argv[]) {

	MainArguments main_args;
        if (!parse_main_args(argc, argv, main_args))
            return 1;

        std::string options = "exhaustive random randomLimited";
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
	int min_size = main_args.min_size;
	bool hashed = main_args.hash;
	
	/* Load the workflow from the DAX file*/
	Workflow * workflow = new Workflow("some_workflow");
	if (workflow->load_from_xml(dax_file)) {
	  exit(1);
	}
	//printEdges(getImported());
	//printNodes(getImported());
	std::cout << "Generating permutations...\n";
	std::vector<igraph_t *> * clusterings;
	if(method == "exhaustive"){
		if(hashed){
			/*Compute all transformations with hashing enabled to use up less memory */
			std::vector<std::string> * hashes = exhaustivePermHashStart(getImported(), output_prefix, timeout, max_permutations);
			std::cout << "Total permutations made: " << hashes->size() << "\n";
		}
		else{
			if (timeout <= 0 && max_permutations <= 0){
						clusterings = exhaustivePermStart(getImported(), output_prefix);
						std::cout << "Total permutations made: " << clusterings->size() << "\n";
			}
			else{
				clusterings = exhaustivePermStart(getImported(), output_prefix, true, timeout, max_permutations);
				std::cout << "Total permutations made: " << clusterings->size() << "\n";
			}
		}
	}
	else if(method == "random"){
		clusterings = randomizedPerm(getImported(), timeout, max_permutations, output_prefix);
		std::cout << "Total permutations made: " << clusterings->size() << "\n";
	}
	else if(method == "randomLimited"){
		std::cout << "Total permutations made: " << RandomizedPermEvenSpread(getImported(), max_permutations, min_size, attempt_cap, output_prefix) << "\n";
	}
	else {
		std::cerr << "This type of clustering option is not supported.\n";
		std::cerr << "Current options for clustering: " << options << "\n";
	}
	exit(0);
}


