# dax_permutation_generator
Python and C code for exhaustive permutations of a DAX file


In order to use the C++ code, you'll have to install igraph for C. To do so, follow the instructions at the website:
http://igraph.org/c/ 

As seen in the Makefile, the flags -I/usr/local/igraph -L/usr/local/lib -ligraph  is needed when compiling. 
Just in case (as this happened before) be sure that usr/local/lib and usr/local/igraph actually HAS the required files,
and they weren't placed someplace else by the installer.

The C code will also require the pugixml library; version 1.7 was used for the workflow.cpp code.


This code is used to take in a given workflow in the DAX format and output some or all possible clustering options
for that given workflow, in the form of a DAG. Be warned: all possible clustering options of a workflow is equivalent
to finding a power set, and is therefore exponential in complexity. Don't expect your computer to have all the 
memory to make all possible clustering options, nor enough disk space to store all of the resulting DAX files,
nor enough time in the universe to actually output it all in the first place.

That being said, you can also get a random set of clustered tasks, as well as outputs based on heuristics previously proposed
in this field, such as horizontal runtime balanced clustering, impact factor balanced clustering, or distance balanced clustering.
Also has a custom method.

For more information, run the program with the --help flag to see all of the program's flags, methods, and options.
