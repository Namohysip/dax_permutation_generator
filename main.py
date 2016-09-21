'''
Created on Feb 21, 2016

@author: Noah Higa
'''
import DAGUtilities as util
import PermutationMaker as pm
import networkx as nx
import sys
import time as t

test = nx.DiGraph()
al = list()
k = 8
for i in xrange(1,k+1):
    j = util.newJob(i, 60)
    al.append(j)
    test.add_node(j)
test.add_edge(al[0],al[1])
test.add_edge(al[0],al[2])
test.add_edge(al[0],al[3])
test.add_edge(al[1],al[4])
test.add_edge(al[2],al[5])
test.add_edge(al[3],al[6])
test.add_edge(al[4],al[7])
test.add_edge(al[5],al[7])
test.add_edge(al[6],al[7])

maker = pm.permutationMaker(test,"output/small",1,1)
start = t.time()
maker.exhaustivePermStart()
print "Time for permuting: " + str(t.time() - start)
start = t.time()
maker.outputDAX()
print "Time for writing: " + str(t.time() - start)
