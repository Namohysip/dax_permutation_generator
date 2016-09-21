'''
Created on Apr 6, 2016

@author: Noah Higa
'''
import sys
from Job import Job

def newJob(identity, runtime = 1):
    aNewJob = Job(identity, 1, 1, runtime, 1, 1, 1, 1, 1)
    return aNewJob

def printEdges(G):
    edges = G.edges()
    for edge in edges:
        sys.stdout.write('(')
        sys.stdout.write(edge[0].__str__())
        sys.stdout.write(',')
        sys.stdout.write(edge[1].__str__())
        sys.stdout.write(') ')
    print ' '

def printNodes(G):
    nodes = G.nodes()
    for node in nodes:
        sys.stdout.write(node.__str__())
        sys.stdout.write("; ")
        
    print ' '
