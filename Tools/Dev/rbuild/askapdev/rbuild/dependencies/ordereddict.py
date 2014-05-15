## @file
#  Module to gather dependency information for ASKAP packages
#
# @copyright (c) 2006 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
#
#  @author Malte Marquarding <malte.marquarding@csiro.au>
#

## This is an ordered dict, i.e. the keys are in the order in
#  which they have been added. It has the same interface as dict
#  but only the functions which are in use have been implemented
class OrderedDict:
    ## Create an empty container
    #  @param self the object reference
    def __init__(self):
        self._list = []

    ## Return the length of the container (the number of keys)
    #  @return an integer value
    def __len__(self):
        return len(self._list)

    ## Insert an item into the container
    #  @param self the object reference
    #  @param key the key of the item
    #  @param value the value of the item
    def __setitem__(self, key, value):
        self._list.append([key, value])

    ## Retrieve an item from the container using its key
    #  @param self the object reference
    #  @param i the key of the item to retrieve
    #  @return  the value of the item
    def __getitem__(self, i):
        found = False
        for key, value in self.iteritems():
            if key == i:
                return value
        if not found:
            raise KeyError(str(i))

    ## Get the keys of the container
    #  @param self the object reference
    #  @return a list of keys
    def keys(self):
        return [i[0] for i in self._list]

    ## Determine if a key exists in the container
    #  @param self the object reference
    #  @param k the name of the key
    #  @return a boolean inidcatinf if the key exists
    def has_key(self, k):
        return (k in self.keys())

    def __contains__(self, k):
        return self.has_key(k)

    ## Get the values of the container
    #  @param self the object reference
    #  @return a list of values
    def values(self):
        return [i[1] for i in self._list]

    ## Return a generator for this container
    #  @param self the object reference
    #  @return a generator returning (key, value) tuples
    def iteritems(self):
        for i in self._list:
            yield i[0], i[1]

    ## Move an existing item to the end of the container
    #  @param self the object reference
    #  @param key the key of the item
    def toend(self, key):
        value = self.__getitem__(key)
        del self._list[self.keys().index(key)]
        self._list.append([key, value])
