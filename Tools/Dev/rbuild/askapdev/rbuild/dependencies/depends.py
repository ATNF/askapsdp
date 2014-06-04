#!/usr/bin/env python
# Copyright (c) 2009 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

import os
import pprint
import sys

import networkx as nx
import askapdev.rbuild.utils as utils

# Allow relative import to work when run as standalone script.
# http://www.python.org/dev/peps/pep-0366/
if __name__ == "__main__" and __package__ is None:
    __package__ = "askapdev.rbuild.Depends"

from ..exceptions import BuildError

ASKAP_ROOT = os.environ.get("ASKAP_ROOT")
if ASKAP_ROOT is None:
    raise BuildError("ASKAP_ROOT environment variable is not defined")

stdout, stderr, returncode = utils.runcmd('svn info %s' % ASKAP_ROOT)
if returncode == 0:
    for line in stdout.split('\n'):
        if line.startswith('URL:'):
            ASKAP_URL = line.split(':',1)[1]
            break
else:
    ASKAP_URL = ''

DEPFILENAME = 'dependencies.default'
DEPTEST = '''
#
# alias=path<;optional space separated libs>
#
scimath=Code/Base/scimath/current
casacore=3rdParty/casacore/casacore-1.2.0; casa_ms casa_images cas
common=3rdParty/LOFAR/Common/Common-3.3
'''

class Depends:
    '''
    All paths are relative to ASKAP_ROOT.
    ./depends.py $ASKAP_ROOT/Code/Base/accessors/current
    self._graph - a directed graph
    * edges are a directed connection beween nodes of a graph.
    * edges contain dictionary. We use key 'lib' to a list of libraries.
    * add an edge automatically adds nodes (which are package directories).
    * node names are package directory paths relative to ASKAP_ROOT.
    '''
    def __init__(self, rootpkgdir='.', recursion=True):
        '''
        '''
        self._rootpkgdir = os.path.relpath(rootpkgdir, ASKAP_ROOT)
        self._graph = nx.DiGraph()
        self.create_dependency_graph(self._rootpkgdir, recursion=recursion)
        self.ordered_dependencies = self.get_ordered_dependencies()
        self._check_for_multiversion()


    def create_dependency_graph(self, pkg_path, depfilename=DEPFILENAME,
                                use_repo='second', recursion=True):
        '''
        Adds new edges to the self._graph object from a packages
        dependency.default file.
        From the dependency file, pull out from each line:
        * alias,
        * relative path to dependent package,
        * optional libraries.
        If edge already in the graph skip, else add the edge (package)
        to the graph via its add_edge() method.
        use_repo ['first', 'second', 'never']
          'first'  - always check repo and if not found skip
          'second' - check work area if it exists else if not found try repo
          'never'  - check work area if not found skip
        '''
        data = ''
        rel_depfile  = os.path.join(pkg_path, depfilename)
        work_pkgdir  = os.path.join(ASKAP_ROOT, pkg_path)
        work_depfile = os.path.join(ASKAP_ROOT, rel_depfile)
        repo_depfile = os.path.join(ASKAP_URL,  rel_depfile)

        if use_repo in ['first', 'second'] and ASKAP_URL == '':
            utils.q_print('warn: depends module unable to determine ASKAP_ROOT repository URL.')
            use_repo = 'never'

        # Get contents of dependency.default file if it exists.
        # Can get from work area or from repository depending on use_repo
        # setting and whether work area and/or repo availability.
        #
        if use_repo == 'second' and os.path.exists(work_pkgdir):
            if os.path.exists(work_depfile):
                fh = open(work_depfile)
                data = fh.read()
                fh.close()
        elif use_repo != 'never':
            cmd = 'svn cat %s' % repo_depfile
            stdout, stderr, returncode = utils.runcmd(cmd)
            if returncode == 0:
                data = stdout

        # Process the dependency.default contents.
        # Format of dependencies.default line is:
        # <alias> = <relpath to dep pkg> [;] [<lib1> <lib2> ... <libN>]
        # 
        for line in data.split('\n'):
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            alias, pkg_spec = line.split('=', 1)
            pkg_spec =  pkg_spec.split(';', 1)
            dep_pkg_path = pkg_spec[0].strip()
            if len(pkg_spec) == 2:
                dep_pkg_libs = pkg_spec[1].strip().split()
            else:
                dep_pkg_libs = []
            # Check if already processed rootpkgdir.
            # Need edge check not node check because the node may already
            # exist as dependency but have not processed its
            # dependencies yet.
            if self._graph.has_edge(pkg_path, dep_pkg_path):
                continue # Already analyzed this node.
            else:
                self._graph.add_edge(pkg_path, dep_pkg_path,
                                     libs=dep_pkg_libs)
                if recursion:
                    self.create_dependency_graph(dep_pkg_path)


    def get_ordered_dependencies(self):
        '''
        Get the dependencies in the reverse order and check for dependency
        cycles.
        Return a list of nodes.
        XXX also need to return lib (edge attributes)
        See:
        http://stackoverflow.com/questions/261573/best-algorithm-for-detecting-cycles-in-a-directed-graph
        http://en.wikipedia.org/wiki/Tarjan%E2%80%99s_strongly_connected_components_algorithm
        "Therefore, the order in which the strongly connected components are
        identified constitutes a reverse topological sort of the DAG formed by
        the strongly connected components"
        '''
        # list of lists
        scc = nx.algorithms.components.strongly_connected.strongly_connected_components(self._graph)
        if nx.algorithms.dag.is_directed_acyclic_graph(self._graph):
            # Being a DAG, guarantees each list item in scc (a list) contains
            # a single node. The last item is the target directory.
            a = [item[0] for item in scc[:-1]]
            return a
        else:
            msg = 'dependency cycles\n'
            for item in scc:
                if len(item) > 1:
                    msg += '        %s\n' % ' <-> '.join(item)
            raise BuildError(msg)


    def _check_for_multiversion(self):
        '''
        Loop thru the ordered dependencies looking for different versions
        of the same package.  Each node is identified by the packages
        relative path so by sorting the paths and identifying the
        adjacent packages any common elements can be determined.
        Print out the duplicated versions and the list of immediate
        predecessors to give some idea of why there are duplicates.
        Ideally would search back through all paths and print that.
        '''
        mvd = [] # list of multiple version dependencies.
        nodes = self.ordered_dependencies
        nodes.sort()

        # Break nodes (rel paths to pkgdir) into leading path and the pkgdir.
        # e.g. ('3rdParty/blas', 'blas-1.0')
        # Compare the leading paths between consecutive nodes.  If they are
        # the same then have mutiple version dependencies.
        #
        for i in range(len(nodes) - 1):
            pkg1 = os.path.split(nodes[i])
            pkg2 = os.path.split(nodes[i+1])
            if pkg1[0] == pkg2[0]:
                mvd.append((nodes[i], nodes[i+1]))

        msg = 'multiple version dependency\n'
        if mvd:
            for item in set(mvd):   # set() removes duplicates.
                msg += '    %s != %s\n' % (item[0], item[1])
                for i in range(2):
                    parents = self._get_predecessors(item[i])
                    pred = ' <-- '.join(parents)
                    msg += '        %s <-- %s\n' % (item[i], pred) 
            raise BuildError(msg)


    def _get_predecessors(self, node):
        curr_node = node
        predecessors = []
        while curr_node:
            parents = self._graph.predecessors(curr_node)
            if parents:
                parent = parents[0] # Only show one path
                predecessors.append(parent)
                curr_node = parent
            else:
                curr_node = None
        return predecessors


    def get_dependency_lib(self):
        '''
        Check each edge to a node and collect all the listed libraries.
        '''

    
    def get_explicit(self):
        '''
        Return a list of direct dependencies not implicit ones.
        '''
        deps = []
        for n in self._graph: 
            parents = self._graph.predecessors(n)
            if self._rootpkgdir in parents:
                deps.append(n)
        return deps



#
# Main - test
#

if __name__ == '__main__':
    if len(sys.argv) > 1:
        rootdir = sys.argv[1]
    else:
        rootdir = '.'
    d = Depends(rootpkgdir=rootdir)
    print "Dependencies:\n    %s" % '\n    '.join(d.ordered_dependencies)

    #g = d._graph
    #pp = pprint.PrettyPrinter()
    #print "\nGraph structure:"
    #pp.pprint(g.adj)

    #print "\nNodes:"
    #for node in g.nodes(data=True):
    #    print node

    #print "\nEdges:"
    #for edge in g.edges(data=True):
    #    print edge

    #import matplotlib.pyplot as plt
    #nx.draw_random(g)
    #plt.savefig('depends-graph.png')
