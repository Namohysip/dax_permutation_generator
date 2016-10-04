/**
 * @file workflow.hpp
 * @brief Contains workflow-related classes
 */

#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <map>

#ifndef HEADER_PUGIXML_HPP
#       include "pugixml-1.7/pugixml.hpp"
#endif

class Edge;
class Task;

/**
 * @brief A workflow is a DAG of tasks, with points to
 *        source tasks and sink tasks
 */
class Workflow
{
public:
    /**
     * @brief Builds an empty Workflow
     */
    Workflow(const std::string & name);

    /**
     * @brief Destroys a Workflow
     */
    ~Workflow();

    /**
     * @brief Loads a complete workflow from an XML filename
     * @param[in] xml_filename The name of the XML file
     */
    void load_from_xml(const std::string & xml_filename);

    /**
     * @brief Adds a task to the workflow
     */
    void add_task(Task &task);

    /**
     * @brief Get a task based on its ID
     */
    Task *get_task(std::string id);

    /**
     * @brief Add an edge between a parent task and a child task
     */
    void add_edge(Task *parent, Task *child, std::string filename, double filesize);

    /**
     * @brief Get source tasks
     */
    std::vector<Task *>  get_source_tasks();

    /**
     * @brief Get sink tasks
     */
    std::vector<Task *> get_sink_tasks();

public:
    std::string name; //!< The Workflow name
    std::map<std::string, Task *> tasks; //!< Hashmap of all tasks

private:
    pugi::xml_document dax_tree; //!< The DAX tree
};

/**
 * @brief A workflow Task is some attributes, pointers to parent tasks,
 *        and pointers to children tasks.
 */
class Task
{
public:
    /**
     * @brief Constructor
     */
    Task(const int num_procs, const double execution_time, std::string id);

    /**
     * @brief Destructor
     */
    ~Task();


public:
    int num_procs; //!< The number of processors needed for the task
    double execution_time; //!< The execution time of the task
    std::string id; //!< The task id
    std::vector<Edge *> upwards; //!< Toward parents
    std::vector<Edge *> downwards; //!< Toward children

};

/**
 * @brief A workflow Edge
 */
class Edge
{
public:
    /**
     * @brief Constructor
     */
    Edge(Task *parent, Task *child, std::string filename, double filesize);

    /**
     * @brief Destructor
     */
    ~Edge();


public:
    Task *parent, *child;
    std::string filename;
    double filesize; //!< The file size
};

/**
 * @brief A helper File class in the Workflow
 */
class File
{
public:
    /**
     * @brief Constructor
     */
    File(std::string filename, double filesize);

    /**
     * @brief Destructor
     */
    ~File();

    void set_producer(Task *producer);

    void add_consumer(Task *consumer);

public:
    Task *producer;
    std::vector<Task *> consumers;
    std::string filename;
    double filesize;
};



