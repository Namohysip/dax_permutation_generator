
CXX_FLAGS = -std=c++11 -Ofast

default: main

OBJECTS= mainTest.o workflow.o pugixml.o DAGUtilities.o PermutationMaker.o
HEADERS= workflow.hpp
IGRAPH_HEADERS= -I/usr/local/include/igraph/ -I/usr/local/igraph/
IGRAPH_LIB= -L/usr/local/lib -ligraph 


main: $(OBJECTS)
	g++ $(CXX_FLAGS) $(OBJECTS) $(IGRAPH_LIB) -o main

main.o: main.cpp $(HEADERS)
	g++ $(CXX_FLAGS) -c main.cpp -o main.o

workflow.o: workflow.cpp $(HEADERS)
	g++ $(CXX_FLAGS) $(IGRAPH_HEADERS) -c workflow.cpp -o workflow.o

scheduler.o: scheduler.cpp $(HEADERS)
	g++ $(CXX_FLAGS) -c scheduler.cpp -o scheduler.o

pugixml.o: pugixml-1.7/pugixml.cpp pugixml-1.7/pugiconfig.hpp pugixml-1.7/pugixml.cpp pugixml-1.7/pugixml.hpp
	g++ $(CXX_FLAGS) -c pugixml-1.7/pugixml.cpp -o ./pugixml.o

DAGUtilities.o: DAGUtilities.cpp $(HEADERS)
	g++ $(CXX_FLAGS) $(IGRAPH_HEADERS) -c DAGUtilities.cpp -o DAGUtilities.o
	
mainTest.o: mainTest.cpp $(HEADERS)
	g++ $(CXX_FLAGS) $(IGRAPH_HEADERS) -c mainTest.cpp -o mainTest.o
	
PermutationMaker.o: PermutationMaker.cpp $(HEADERS)
	g++ $(CXX_FLAGS) $(IGRAPH_HEADERS) -c PermutationMaker.cpp -o PermutationMaker.o

clean:
	/bin/rm -f ./*.o
	/bin/rm -f ./main
	/bin/rm -f main.dSYM

