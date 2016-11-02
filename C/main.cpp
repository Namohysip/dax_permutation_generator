#include <igraph.h>
#include <argp.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <string.h>
#include <unistd.h>


#include "workflow.hpp"
#include "DAGUtilities.hpp"
#include "PermutationMaker.hpp"


/**
 * Stores command-line arguments
 */
struct MainArguments
{
	bool just_a_test; 
	std::string dax_filename;
	std::string output_prefix;
	std::string method;	
	double timeout = 0.0;
	int max_permutations = 0;

	bool abort = false;       // set to true while parsing argument if error
	std::string abortReason;  // Error message to print

};


/**
 * Parse command-line arguments
 */

int parse_opt(int key, char *arg, struct argp_state *state)
{

    MainArguments * main_args = (MainArguments *) state->input;

    switch (key)
    {
    case 'T':
    {
	main_args->just_a_test = true;
	break;
    }
    case 'd':
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
    case 'o':
    {
	// May want to add some safety checks here
        main_args->output_prefix = std::string(arg);
        break;
    }
    case 't':
    {
	if ((sscanf(arg, "%lf",&(main_args->timeout)) != 1) ||
            (main_args->timeout < 0)) {
	  main_args->abort = true;
	  main_args->abortReason += "\n  invalid timeout argument '" +
					std::string(arg) + "'";
 	} 
	break;
    }
    case 'p':
    {
	if ((sscanf(arg, "%d",&(main_args->max_permutations)) != 1) ||
            (main_args->timeout < 0)) {
	  main_args->abort = true;
	  main_args->abortReason += "\n  invalid max permutations argument '" +
					std::string(arg) + "'";
 	} 
	break;
    }
    case 'm':
    {
	main_args->method = (std::string) arg;
	if ((main_args->method != "limited") && 
	    (main_args->method != "exhaustive") && 
	    (main_args->method != "hashed") && 
	    (main_args->method != "random") && 
	    (main_args->method != "randomLimited")) {
	  main_args->abort = true;
	  main_args->abortReason += "\n  invalid method argument '" + std::string(arg) + "'";
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
        {"dax", 'd', "FILENAME", 0, "The input DAX file with the original workflow (string, required)", 0},
        {"output", 'o', "PATH_PREFIX", 0, "The filepath prefix for generating output DAX files (string, required)", 0},
        {"timeout", 't', "SECONDS", 0, "Time out value (float, default: 0 - no timeout)", 0},
        {"max-permutations", 'p', "COUNT", 0, "Maximum number of permutations to generate (integer, default: 0 - no maximum)", 0},
        {"method", 'm', "NAME", 0, "Workflow generation  method (string, required): limited, exhaustive, hashed, random, randomLimited", 0},
        {"test", 'T', 0, 0, "Only run a hard-coded test for iGraph", 0},
        {0, '\0', 0, 0, 0, 0} // The options array must be NULL-terminated
    };

    struct argp argp = {options, parse_opt, 0, "A tool to generate workflow configurations via task merging.", 0, 0, 0};

    // Parse argument
    argp_parse(&argp, argc, argv, 0, 0, &main_args);

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

        std::string options = "exhaustive limited hashed random randomLimited";
	igraph_i_set_attribute_table(&igraph_cattribute_table); //ALWAYS HAVE THIS. Enables attributes.

	if (main_args.just_a_test) {
		test_with_small_hardcoded_graph();	
		exit(0);
	}

	// Getting command-line arguments
	std::string dax_file = main_args.dax_filename;
	std::string output_prefix = output_prefix;
	std::string method = main_args.method;
	double timeout = main_args.timeout;
	double max_permutations = main_args.max_permutations;
	
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
		clusterings = exhaustivePermStart(getImported(), false, timeout, max_permutations);
		outputDAX(clusterings, output_prefix);
		
	}
	else if (method == "limited"){
		clusterings = exhaustivePermStart(getImported(), true, timeout, max_permutations);
		std::cout << "Total permutations made: " << clusterings->size() << "\n";
		outputDAX(clusterings, output_prefix);
	}
	else if (method == "hashed"){
		/*Compute all transformations with hashing enabled to use up less memory */
		std::vector<std::string> * hashes = exhaustivePermHashStart(getImported(), output_prefix, timeout, max_permutations);
		std::cout << "Total permutations made: " << hashes->size() << "\n";
	}
	else if(method == "random"){
		clusterings = randomizedPerm(getImported(), timeout, max_permutations, output_prefix);
		std::cout << "Total permutations made: " << clusterings->size() << "\n";
	}
	else if(method == "randomLimited"){
		std::cout << "Total permutations made: " << RandomizedPermEvenSpread(getImported(), max_permutations, output_prefix) << "\n";
	}
	else {
		std::cerr << "This type of clustering option is not supported.\n";
		std::cerr << "Current options for clustering: " << options << "\n";
	}
	exit(0);
}


