/**
 * @file workflow.cpp
 * @brief Contains workflow-related functions
 */

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string.h>
#include <igraph.h>
#include "pugixml-1.7/pugixml.hpp"

#include "workflow.hpp"
#include "PermutationMaker.hpp"


using namespace std;
using namespace pugi;

//void setImported(igraph_t *);

Workflow::Workflow(const std::string & name)
{
    this->name = name;
}

Workflow::~Workflow()
{
    // delete name;   TOFIX
    // name = nullptr;
    tasks.clear();

}
int Workflow::load_from_xml(const std::string &xml_filename)
{

	std::cout << "Adding tasks...\n";
	igraph_t * graph = new igraph_t;
	std::map<std::string,int> idVals;
	std::map<std::string,int>::iterator idIter;
	
	igraph_i_set_attribute_table(&igraph_cattribute_table);
	igraph_empty(graph, 0, IGRAPH_DIRECTED);
	
    // XML document creation
    xml_parse_result result = dax_tree.load_file(xml_filename.c_str());
    //std::cerr << result << "\n";
    if (!result) {
       	std::cerr << "Invalid DAX file\n";
	return 1;
    }

    // Create file map
    std::map<std::string, File *> file_map;

    // Root node
    xml_node dag = dax_tree.child("adag");

    // Loop through job nodes and add files to the filemap

    for (xml_node job = dag.child("job"); job; job = job.next_sibling("job"))
    {

	// Create a task and add it to the workflow
        Task *task = new Task (1, strtod(job.attribute("runtime").value(),NULL), job.attribute("id").value());
		this->add_task(*task);
	//Also add it to the igraph
		igraph_add_vertices(graph, 1, 0);
		igraph_integer_t ident = igraph_vcount(graph) - 1;
		SETVAS(graph, "id", ident, job.attribute("id").value());
		SETVAS(graph, "components", ident, job.attribute("id").value());
		SETVAN(graph, "runtime", ident, (double) strtod(job.attribute("runtime").value(),NULL));
		SETVAN(graph, "procs", ident, 1);
	//add its id key-value pair to the map as well.
		idVals.insert ( std::pair<std::string,int>(job.attribute("id").value(), (int) ident));

	// Create the files
        for (xml_node uses = job.child("uses"); uses; uses = uses.next_sibling("uses"))
        {
		
		const char *filename = uses.attribute("file").value();
		double filesize = std::stol(uses.attribute("size").value());
		File *file;
		if (file_map.find(std::string(filename)) == file_map.end()) {
		  file = new File(std::string(filename), filesize);
		  file_map[filename] = file;
		} 
		if (!strcmp(uses.attribute("link").value(),"output")) {
			file_map[filename]->set_producer(task);
		} else { // input or intermediate
			file_map[filename]->add_consumer(task);
		}

        }
    }

	std::cout << "Adding edges based on file dependencies...\n";
    // Build edges based on files
    for(std::map<std::string, File *>::iterator it = file_map.begin(); it != file_map.end(); ++it) {
	    File *file = it->second;
	    if (file->producer) { // If there was no producer, ignore this since it's some
                                  // static file or something
		for (int i=0; i < file->consumers.size(); i++) {
	    	  //cout << "NEW EDGE: " << file->producer->id << "->" <<
                  //          file->consumers.at(i) << " for file " <<file->filename << " (" <<
                  //          file->filesize << " bytes)\n";
                  this->add_edge(file->producer, file->consumers.at(i), 
                                     file->filename, file->filesize);
				igraph_integer_t from = idVals.find(file->producer->id)->second;
				igraph_integer_t to = idVals.find(file->consumers.at(i)->id)->second;
				igraph_add_edge(graph, from, to);
				
		}	
            }
    }

    // TODO: Free the files 


	std::cout << "Adding edges based on control dependencies...\n";
    // Build zero-weight edges based on control dependencies (could be very few)
    for (xml_node edge_bottom = dag.child("child"); edge_bottom; edge_bottom = edge_bottom.next_sibling("child"))
      {
        Task *dest = get_task(edge_bottom.attribute("ref").value());

	for (xml_node edge_top = edge_bottom.child("parent"); edge_top; edge_top = edge_top.next_sibling("parent"))
	  {
	    Task *source = get_task(edge_top.attribute("ref").value());
	    
	    // Check whether there is already a dependency
            int already_there = 0;
	    for (int i=0; i < source->downwards.size(); i++) {
              if (source->downwards.at(i)->child == dest) {
                already_there = 1;
              } 
            }
 	    if (! already_there) {
	      add_edge(source , dest, "control", 0);
		  igraph_integer_t from = idVals.find(source->id)->second;
				igraph_integer_t to = idVals.find(dest->id)->second;
				igraph_add_edge(graph, from, to);
		  
            }
	  }
      }

	std::cout << "Done!\n";
	getGlobalSettings()->original_graph = graph;
	return 0;
}


void Workflow::add_task(Task &task) {
  this->tasks[task.id] = &task;
}

Task * Workflow::get_task(std::string id) {
	  return this->tasks[id];
}

void Workflow::add_edge(Task *parent, Task *child, std::string filename, double filesize)
{ 
  Edge *edge = new Edge(parent, child, filename, filesize);
  child->upwards.push_back(edge);
  parent->downwards.push_back(edge);
}


std::vector<Task *> Workflow::get_source_tasks() {
  std::vector<Task *> task_list;
  for(std::map<std::string, Task *>::iterator it = this->tasks.begin(); it != this->tasks.end(); ++it) {
    if ((it->second)->upwards.empty()) {
      task_list.push_back(it->second);
    }
  }
  return task_list;
}

std::vector<Task *> Workflow::get_sink_tasks() {
  std::vector<Task *> task_list;
  for(std::map<std::string, Task *>::iterator it = this->tasks.begin(); it != this->tasks.end(); ++it) {
    if ((it->second)->downwards.empty()) {
      task_list.push_back(it->second);
    }
  }
  return task_list;
}



Task::Task(const int num_procs, const double execution_time, std::string id)
{
    this->num_procs = num_procs;
    this->execution_time = execution_time;
    this->id.assign(id);
}

Task::~Task()
{
    delete &(this->id);
    this->upwards.clear();
    this->downwards.clear();
}


Edge::Edge(Task *parent, Task *child, std::string filename, double filesize) 
{
    this->parent = parent;
    this->child = child;
    this->filename = filename;
    this->filesize = filesize;
}

Edge::~Edge() 
{
}


File::File(std::string filename, double filesize)
{
    this->filename = filename;
    this->filesize = filesize;
    this->producer = nullptr;
}

File::~File()
{
    delete &(this->filename);
    this->consumers.clear();
}

void File::set_producer(Task *producer) {
    this->producer = producer;
}

void File::add_consumer(Task *consumer) {
    this->consumers.push_back(consumer);
}
