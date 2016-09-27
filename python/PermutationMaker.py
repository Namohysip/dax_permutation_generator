'''
Created on Feb 21, 2016

@author: Noah Higa
'''
from networkx.algorithms.dag import is_directed_acyclic_graph
import threading
import networkx as nx
import Queue as q

class permutationMaker (threading.Thread):
    #initialization and constructor method.
    #dag is the source dag
    #output is a string that is the base name of the file output
    #See run method for clusterType use
    #Thread is the thread cap, to be used for multithreaded methods
    def __init__(self, dag, output, clusterType = 0, threads = 1):
        #This will be used if you decide to run the program with
        #the run command through a thread. See run for what each cluster
        #Type corresponds to.
        self.clustering = clusterType
        #This is the source (original) dag to work with
        self.dag = dag
        #The maximum number of threads if performing the multithreaded version
        self.threadCap = threads
        #Initializes the current number of threads to 1
        self.activeThreads = 1
        #Locks to be used for multithreading
        self.counterMutex = threading.Lock()
        self.listMutex = threading.Lock()
        
        #The list of permutations to be made of the source
        self.listOfPerm = []
        #The base file name and the perm count, which will be
        #eventually output as <outputname>_px.txt
        #where x is the permNo
        self.base = output
        self.permNo = 0
        
    #Method that will be run if thread.start() is called.
    #Check __init__ on what "clustering" is for.
    def run(self):
        #Clustering = 0 runs the exhaustive code.
        #If the threadCap is less than 2, it runs the
        #Sequential code. Otherwise, it runs the multithreaded code.
        if self.clustering == 0:
            if self.threadCap < 2:
                self.exhaustivePermStart()
            else:
                self.exhaustivePermMulti(self.dag, 0)
        #Clustering = 1 makes it cluster by level
        #Currently only clusters with 2 nodes per level
        elif self.clustering == 1:
            self.clusterByLevel(2)
    
    #This is the start of the sequential code.
    #Automatically appends the original dag into the list.
    def exhaustivePermStart(self):
        self.listOfPerm = []
        self.listOfPerm.append(self.dag)
        self.exhaustivePerm(self.dag)
    
    #Recursively generates every possible permutation of
    #the dag given. NOTE: This does NOT add the original
    #unclustered DAG to the list of permutations
    def exhaustivePerm(self, dag):
        if len(dag.edges()) < 1:
            return #Base case: Only one node left
        nodes = dag.nodes()
        for node in nodes:
            for otherNode in nodes:
                if(node != otherNode): #Compare all pairs of nodes
                    newDag = dag.copy()
                    #Find the corresponding jobs in the cloned dag.
                    n1 = self.findJob(newDag,node)
                    n2 = self.findJob(newDag,otherNode)
                    if(n1 != None and n2 != None): #Make ure the dags still exist
                        newDag = self.combine(newDag,n1,n2) #Combine the two nodes
                        if (self.checkLegal(newDag)): #Check if it's still a DAG
                            #Check for duplicates. Don't want to run through the same
                            #branch of code twice in a row.
                            if self.addWithoutDuplicates(self.listOfPerm,newDag):
                                self.exhaustivePerm(newDag) #If so, add and keep going
    
    #Multithreaded version of the sequential code above.
    #Refer to the sequential version for the basic operations.
    #Comments here will be focused on how it was made parallel.
    def exhaustivePermMulti(self, dag, depth):
        if len(dag.edges()) < 1:
            return #Base case: Only one node left
        threads = [] #This will track threads that were created in this branch.
        nodes = dag.nodes()
        for node in nodes:
            for otherNode in nodes:
                if(node != otherNode): #Compare all pairs of nodes
                    newDag = dag.copy()
                    n1 = self.findJob(newDag,node)
                    n2 = self.findJob(newDag,otherNode)
                    if(n1 != None and n2 != None):
                        newDag = self.combine(newDag,n1,n2)
                        if (self.checkLegal(newDag)): #Check if it's still a DAG
                            #Critical section: Checking for duplicates in the list
                            #Could be a huge problem for parallelism since this
                            #critical section is O(n^2) where n is the number of dags.
                            self.listMutex.acquire()
                            added = self.addWithoutDuplicates(self.listOfPerm,newDag)
                            self.listMutex.release()
                            
                            if added:
                                #Critical section: Checking for if there is room to make
                                #Another thread to compute the dags of the subgraph that
                                #was found to be valid. Should be a short section; it's just
                                # a comparison and an increment.
                                self.counterMutex.acquire()
                                makeNewThread = self.activeThreads < self.threadCap
                                if makeNewThread:
                                    self.activeThreads += 1
                                self.counterMutex.release()
                                if makeNewThread:
                                    t = threading.Thread(target = self.exhaustivePermMulti, args =  (newDag, 0))
                                    threads.append(t) #Adds the new thread to the list of child threads
                                    t.start()
                                else:
                                    #If a new thread was not created, it just continues sequentially.
                                    self.exhaustivePermMulti(newDag, depth+1)
        for t in threads:
            t.join() #Wait for all children threads to complete.
        #If depth is0 and it's out of the big for loops, then
        #It finished computing its original dag, and when it returns, the
        #Thread is finished. So active threads will be decremented.
        if(depth == 0):
            self.counterMutex.acquire()
            self.activeThreads -= 1
            self.counterMutex.release()
            
    #A key method. Combines node 2 into node 1,
    #and then removes node 2 from the dag.
    #Both nodes MUST be in the dag G.
    def combine(self, G, node1, node2):
        #adds the work in node2 into node 1
        #To keep things deterministic, the lowest id
        #is kept. If DAG job ids are made such that
        #lower IDs are closer to the root, then jobs
        #are always merged "horizontally" or "upward"
        node1.job_id = min(node1.job_id,node2.job_id)
        node1.requested_time += node2.requested_time
        node1.actual_time += node2.actual_time
        
        #Gets successors and predecessors of node 2.
        preds = G.predecessors(node2)
        succs = G.successors(node2)
        #redirects all edges involving node 2 to point to node 1,
        #and all edges pointing from node 2 to point from node 1.
        for node in preds:
            G.remove_edge(node,node2)
            G.add_edge(node,node1)
        for node in succs:
            G.remove_edge(node2,node)
            G.add_edge(node1,node)
        G.remove_node(node2) #This also removes all edges associated with node2
        #If node1 and 2 are directly linked
        #a self-cycle could form but can be removed without consequence
        if G.has_edge(node1,node1):
            G.remove_edge(node1,node1)
        return G
        
        
    #Checks if the graph is still a DAG.
    def checkLegal(self, G):
        return is_directed_acyclic_graph(G)
    
    #If the graph is a duplicate, it doesn't add it
    #It returns false if it isn't added, true otherwise    
    def addWithoutDuplicates(self, c, dag):
        for graph in c:
                if self.checkAllNodes(graph, dag):
                    return False
        c.append(dag)
        return True
    
    #Returns true if all the edges have a match with one another,
    #False if at least one node has no match with another
    #Assumes that there are zero nodes that have no in or out edges.
    def checkAllNodes(self, dag1, dag2):
        edges1 = dag1.edges()
        edges2 = dag2.edges()
        #Trivially, if two graphs have different numbers of
        #edges and nodes, they are not the same.
        if(len(edges1) != len(edges2)):
            return False
        if(len(dag1.nodes()) != len(dag2.nodes())):
            return False
        for e1 in edges1:
            found = False
            for e2 in edges2:
                #For every edge, check if there is a match.
                if self.isSameJob(e1[0],e2[0]):
                    if self.isSameJob(e1[1],e2[1]):
                        found = True
            #If e1 doesn't find a single match in graph 2's edges,
            #then it isn't the same graph.
            if not found: 
                return False
        #This is only true if every edge has a match.
        #Because it is impossible for there to be duplicate edges
        #and because the graphs have the same number of edges,
        #There must be a 1:1 match between every edge for it to be true.
        return True
        
        #Basically checks if all its values are identical
        #Can't just check ID because of merging
    def isSameJob(self, job1, job2):
        if job1.job_id != job2.job_id:
            return False
        if job1.actual_time != job2.actual_time:
            return False
        if job1.requested_time != job2.requested_time:
            return False
        return True
    
        #Should only be used for finding
        #the same job after a G.copy() command
    def findJob(self, G,job):
        nodes = G.nodes()
        for node in nodes:
            if self.isSameJob(node,job):
                return node
            
    #Returns the node that has no predecessors
    def findHead(self, G):
        nodes = G.nodes()
        for node in nodes:
            if len(G.predecessors(node)) < 1:
                return node
    
    #Returns the node that has no successors.
    def findSink(self,G):
        nodes = G.nodes()
        for node in nodes:
            if len(G.successors(node)) < 1:
                return node
    
    #Returns a list of all the permutations that have been calculated.
    #Note: If no permutation method is ever called, the list will be empty.   
    def getList(self):
        return self.listOfPerm
    
    
    #Writes from the listOfPerm all the dags into files.
    #See dagToFile for implementation for each individual dag.
    def outputFiles(self):
        for dag in self.listOfPerm:
            self.dagToFile(dag)
    
    #Same as outputting files, but in DAX format
    def outputDAX(self):
        for dag in self.listOfPerm:
            self.dagToDAX(dag)
    
    def dagToFile(self,dag):
        #file format:
        # (Tasks): id req act procs(always 1))
        # (Edges): e id1 id2
        # Keep ids contiguous, so ignore the given job ID
        # The ids will also be output as negative values.
        #MakeContiguous makes ids starting at 0, but evertthing is incremented
        #before output. Ie tasks will have ids -1, -2, -3. . . 
        
        #The file name will be the given file base, followed by _p
        #and then its current number, in text file format.
        #i.e. example_p0.txt
        name = self.base + "_p" + str(self.permNo) + ".txt"
        self.permNo += 1
        f = open(name,'w')
        #Make the nodes contiguous for ease of use later.
        nodes = self.makeContiguous(dag)
        edges = nx.edges(dag)
        string = ''
        #Writing out each string to the file.
        for n in nodes:
            string = "T -" + str(n.job_id+1) + " " + str(n.requested_time) + " " + str(n.actual_time) + " 1\n"
            f.write(string)
        for e in edges:
            string = "E -" + str(e[0].job_id+1) + " -" + str(e[1].job_id+1) + "\n"
            f.write(string)
        f.close()
    
    def dagToDAX(self, dag):
        
        name = self.base + "_p" + str(self.permNo) + ".xml"
        self.permNo += 1
        f = open(name,'w')
        #Make the nodes contiguous for ease of use later.
        nodes = self.makeContiguous(dag)
        preamble = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n"
        preamble += "<adag xmlns=\"http://pegasus.isi.edu/schema/DAX\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://pegasus.isi.edu/schema/DAX http://pegasus.isi.edu/schema/dax-2.1.xsd\" version=\"2.1\" count=\"1\" index=\"0\" name=\"NAME\" jobCount=\""
        preamble += str(len(nodes))
        preamble += "\" fileCount=\"0\" childCound=\""
        preamble+= str(len(nodes) - 1)
        preamble += "\">\n\n"
        
        f.write(preamble)
        
        for job in nodes:
            line = "<job id=\"ID0000t"
            line += str(job.job_id)
            line += "\" namespace=\"NAMESPACE\" name=\"Task\" version=\"1.0\" runtime=\""
            line += str(job.actual_time)
            line += "\">\n</job>\n"
            f.write(line)
        f.write("\n")
        for child in nodes:
            if(len(dag.predecessors(child)) > 0):
                line = "<child ref=\"ID0000t"
                line += str(child.job_id)
                line += "\">"
                for parent in dag.predecessors(child):
                    line += "\n  <parent ref=\"ID0000t"
                    line += str(parent.job_id)
                    line += "\" />"
                line += "\n</child>\n"
                f.write(line)
        f.write("\n</adag>")
        f.close()
        
    #This function makes all IDs contiguous
    #and returns an array of the nodes in sorted order
    #I.e. there will be no gaps between IDs
    def makeContiguous(self,dag):
        nodes = nx.nodes(dag)
        #Outer loop: This will be done for every node
        for index in xrange(len(nodes)):
            #inner loop: Find next minimum value that hasn't been sorted
            i = index #Anything before i has already been sorted
            minimum = index #The first minimum is set to the first unsorted task
            while i < len(nodes):
                #If the next node on the list has a lower id, make that the minimum
                if nodes[i].job_id < nodes[minimum].job_id:
                    minimum = i
                i += 1
            #Set the minimum value to the next index.
            #I.e. if there were job ids 5, 3, 2, 7
            #The sort would make it 2, 3, 5, 7
            #And changing the ids would make it 0, 1, 2, 3
            #Index could already be equal to minimum,
            #But if all IDs are unique, will never be less
            nodes[minimum].job_id = index
            #Swap the minimum job id into where it should
            #be in the sorted array
            swap = nodes[minimum]
            nodes[minimum] = nodes[index]
            nodes[index] = swap
        #Returns an array of the modified nodes.
        return nodes
    
    #This method gives a 'label' to all nodes in the graph.
    #Must be a DAG for the code to work.
    #The label is an integer correspond to its level, such that
    #the head is level 0, the sink has the highest level, and
    #The longest path from the head to any other node is that integer.
    def levelLabel(self,dag):
        #Get the head, and initialize the level of all nodes to 0
        head = self.findHead(dag)
        nx.set_node_attributes(dag,'level',0)
        nx.set_node_attributes(dag,'added',False)
        #This queue will be where nodes are added
        que = q.Queue()
        #Start by putting the head into the queue.
        que.put(head)
        #Waits until the sink is found
        while not que.empty():
            #Gets the next node in the queue.
            #A node may potentially be added more than once even after it
            #is removed from the queue, due to a node in the upper levels
            #eventually leading to what was "once" a higher level node.
            #For example, if the head node directly pointed to node B,
            #originally making it level 1, but then some node at level 3 also
            #points to it, making it actually level 4. In this case, it should be
            #re-added to the queue so it and its descendants get its levels updated.
            #But because a dag is acyclic, eventually the queue will be empty.
            #Nodes with only the head for a parent will never be added again, and then
            #their exclusive children won't be, and so on.
            nextNode = que.get()
            nx.set_node_attributes(dag,'added',{nextNode:False})
            #All children will be the parent's level + 1
            lev = dag.node[nextNode]['level'] + 1
            s = dag.successors(nextNode) #the children
            for lower in s:
                #If the node has a level that is less than its parents level+1, update it.
                if dag.node[lower]['level'] < lev:
                    nx.set_node_attributes(dag,'level',{lower:lev})
                    #Since the node has been updated, add it to the queue unless it's already there.
                    if not dag.node[lower]['added']:
                        que.put(lower)
                        nx.set_node_attributes(dag,'added',{lower:True})
        #Return the labeled dag
        return dag
    
    #This is level-based clustering.
    #NodesPerLevel identifies how many nodes there can be, at max, per level.
    #I.e. if there is one head node, pointing to 5 children, which all point to the sink,
    #And nodesPerLevel is set to 2, then those nodes will be combined to make only 2 nodes.
    #If it was set to 6, however, it would do nothing to that level.
    def clusterByLevel(self, nodesPerLevel):
        #Label the dag.
        dag = self.levelLabel(self.dag.copy())
        sink = self.findSink(dag)
        #Get how far to go based on where the sink is.
        depth = dag.node[sink]['level']
        
        #For each level, ignoring the head and sink (because there's only one on those levels)
        for i in xrange(1,depth):
            nodes = list()
            #Add in all nodes that are at that level
            #Is there a faster way to do this?
            for node in dag.nodes():
                if dag.node[node]['level'] == i:
                    nodes.append(node)
            j = 0
            combined = 0
            gap = len(nodes) / nodesPerLevel
            extra = len(nodes) % nodesPerLevel
            if(gap > 0):
                for i in xrange(0,extra):
                    self.combine(dag,nodes[i * gap],nodes[gap*nodesPerLevel+i])
                    combined += 1
                while len(nodes) - combined > nodesPerLevel:
                    for i in xrange(1,gap):
                        self.combine(dag,nodes[j*gap],nodes[j*gap+i])
                        combined += 1
                    j += 1
        self.addWithoutDuplicates(self.listOfPerm,dag)
        return dag
