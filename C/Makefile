
CXX = g++
#CXX_FLAGS = -std=c++11 -Ofast
CXX_FLAGS = -std=c++11 -pg -g
EXEC = dax_permutation_generator



default: $(EXEC)

OBJECTS= main.o workflow.o pugixml.o DAGUtilities.o PermutationMaker.o sha256.o hardCodedTest.o
HEADERS= workflow.hpp sha256.hpp
IGRAPH_HEADERS= -I/usr/local/include/igraph/ -I/usr/local/igraph/
IGRAPH_LIB= -L/usr/local/lib -ligraph

ifeq ($(OS),Windows_NT)
	IGRAPH_LIB += -largp
	endif


$(EXEC): $(OBJECTS)
	$(CXX) $(CXX_FLAGS) $(OBJECTS) $(IGRAPH_LIB) -o $(EXEC)

main.o: main.cpp $(HEADERS)
	$(CXX) $(CXX_FLAGS) $(IGRAPH_HEADERS) -c main.cpp -o main.o

workflow.o: workflow.cpp $(HEADERS)
	$(CXX) $(CXX_FLAGS) $(IGRAPH_HEADERS) -c workflow.cpp -o workflow.o

scheduler.o: scheduler.cpp $(HEADERS)
	$(CXX) $(CXX_FLAGS) -c scheduler.cpp -o scheduler.o

pugixml.o: pugixml-1.7/pugixml.cpp pugixml-1.7/pugiconfig.hpp pugixml-1.7/pugixml.cpp pugixml-1.7/pugixml.hpp
	$(CXX) $(CXX_FLAGS) -c pugixml-1.7/pugixml.cpp -o ./pugixml.o

DAGUtilities.o: DAGUtilities.cpp $(HEADERS)
	$(CXX) $(CXX_FLAGS) $(IGRAPH_HEADERS) -c DAGUtilities.cpp -o DAGUtilities.o

hardCodedTest.o: hardCodedTest.cpp $(HEADERS)
	$(CXX) $(CXX_FLAGS) $(IGRAPH_HEADERS) -c hardCodedTest.cpp -o hardCodedTest.o
	
PermutationMaker.o: PermutationMaker.cpp $(HEADERS)
	$(CXX) $(CXX_FLAGS) $(IGRAPH_HEADERS) -c PermutationMaker.cpp -o PermutationMaker.o
	
sha256.o: sha256.cpp sha256.hpp
	$(CXX) $(CXX_FLAGS) -c sha256.cpp -o sha256.o

clean:
	/bin/rm -f ./*.o
	/bin/rm -f $(EXEC) $(EXEC).dSYM
