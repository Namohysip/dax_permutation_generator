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
	double timeout = 0.0;

	bool abort = false;  // set to true while parsing argument if error
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
	  main_args->abortReason += "\n invalid timeout value '" +
					std::string(arg) + "'";
 	} 
	break;
    }


#if 0
    case 'k':
    {
        main_args->terminate_with_last_workflow = true;
        break;
    }
    case 'W':
    {
        // format:   FILENAME[:start_time]
        vector<string> parts;
        boost::split(parts, (const std::string)std::string(arg),
                     boost::is_any_of(":"), boost::token_compress_on);

        if (access(parts.at(0).c_str(), R_OK) == -1)
        {
            main_args->abort = true;
            main_args->abortReason += "\n  invalid WORKFLOW_FILE argument: file '" + parts.at(0) + "' cannot be read";
        }
        else
        {
            string workflow_filename = parts.at(0).c_str();
            double workflow_start_time = 0.0;

            if (parts.size() == 2)
                workflow_start_time = std::stod(parts.at(1));

            MainArguments::WorkflowDescription desc;
            desc.filename = absolute_filename(workflow_filename);
            desc.name = generate_sha1_string(desc.filename);
	    desc.workload_name = desc.name;
            desc.start_time = workflow_start_time;

            XBT_INFO("Workflow '%s' corresponds to workflow file '%s'.", desc.name.c_str(), desc.filename.c_str());
            main_args->workflow_descriptions.push_back(desc);
        }
        break;
    }
    case 'e':
        main_args->export_prefix = arg;
        break;
    case 'E':
        main_args->energy_used = true;
        break;
    case 'h':
        main_args->allow_space_sharing = true;
        break;
    case 'H':
        main_args->redis_hostname = arg;
        break;
    case 'l':
    {
        int ivalue = stoi(arg);

        if ((ivalue < -1) || (ivalue == 0))
        {
            main_args->abort = true;
            main_args->abortReason += "\n  invalid M positional argument (" + to_string(ivalue) + "): it should either be -1 or a strictly positive value";
        }

        main_args->limit_machines_count = ivalue;
        break;
    }
    case 'L':
    {
        main_args->limit_machines_count_by_workload = true;
        break;
    }
    case 'm':
        main_args->master_host_name = arg;
        break;
    case 'P':
    {
        int ivalue = stoi(arg);

        if ((ivalue <= 0) || (ivalue > 65535))
        {
            main_args->abort = true;
            main_args->abortReason += "\n  invalid PORT positional argument (" + to_string(ivalue) +
                                     "): it should be a valid port: integer in range [1,65535].";
        }

        main_args->redis_port = ivalue;
        break;
    }
    case 'q':
        main_args->verbosity = VerbosityLevel::QUIET;
        break;
    case 's':
        main_args->socket_filename = arg;
        break;
    case 't':
        main_args->enable_simgrid_process_tracing = true;
        break;
    case 'T':
        main_args->enable_schedule_tracing = false;
        break;
    case 'v':
    {
        string sArg = arg;
        boost::to_lower(sArg);
        if (sArg == "quiet")
            main_args->verbosity = VerbosityLevel::QUIET;
        else if (sArg == "network-only")
            main_args->verbosity = VerbosityLevel::NETWORK_ONLY;
        else if (sArg == "information")
            main_args->verbosity = VerbosityLevel::INFORMATION;
        else if (sArg == "debug")
            main_args->verbosity = VerbosityLevel::DEBUG;
        else
        {
            main_args->abort = true;
            main_args->abortReason += "\n  invalid VERBOSITY_LEVEL argument: '" + string(sArg) + "' is not in {quiet, network-only, information, debug}.";
        }
        break;
    }
#endif
    }

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

    }

    if (main_args.abort)
    {
        fprintf(stderr, "Error:%s\n", main_args.abortReason.c_str());
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
	const char *dax_file = main_args.dax_filename.c_str();
	const char *output_prefix = main_args.output_prefix.c_str();
	double timeout = main_args.timeout;
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


