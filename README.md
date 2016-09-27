# dax_permutation_generator
Python and C code for exhaustive permutations of a DAX file

In order to use the C++ code, you'll have to install igraph for C. To do so, follow the instructions at the website:
http://igraph.org/c/ 

As seen in the Makefile, the flags -I/usr/local/igraph -L/usr/local/lib -ligraph  is needed when compiling. 
Just in case (as this happened before) be sure that usr/local/lib and usr/local/igraph actually HAS the required files,
and they weren't placed someplace else by the installer.
