## @file
# Object to simplify unpacking/building/cleaning of ASKAPsoft packages
#
# @copyright (c) 2007 CSIRO
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
# @author Tony Maher <Tony.Maher@csiro.au>
#

import os
import glob

from builder import Builder
import askapdev.rbuild.utils as utils

class Ant(Builder):
    '''
    A subclass of Builder for Ant based projects.
    It overwrites _clean(), _build(), _install(), _test(), and  _doc().
    It initialises buildcommand and installcommand.
    '''
    def __init__(self, pkgname=None, archivename=None, extractdir=None,
                 buildsubdir=None, buildtargets=[], buildcommand="ant",
                 installcommand="ant", installsubdir=""):
        '''
        The constructor sets up a package build "environment".

        :param pkgname:        The (optional) name of the package directory.
                               By default the directory name is used.
        :param archivename:    The (optional) archive name minus suffix.
                               The default is based on package name.
        :param extractdir:     The (optional) directory into which the archive
                               is extracted. It is created if it does not exist.
        :param buildsubdir:    The (optional) directory in which to start the
                               build.
        :param buildtargets:   The (optional) additional build targets.
        :param buildcommand:   The (optional) build command (default is 'ant').
        :param installcommand: The (optional) install command (default is 'ant').
        '''
        Builder.__init__(self, pkgname=pkgname,
                               archivename=archivename,
                               extractdir=extractdir,
                               buildsubdir=buildsubdir,
                               buildcommand=buildcommand,
                               buildtargets=[""] + buildtargets,
                               installcommand=installcommand)
        self.parallel = False
        if installsubdir:
            self.installdir  = os.path.join(self._prefix, installsubdir)
        else:
            self.installdir = self._prefix
        self._run_script = []

    def _clean(self):
        if utils.in_code_tree():
            utils.run("%s %s clean" % (self._icom, self._opts))
        Builder._clean(self)


    def _build(self):
        pass

    def _add_ant_options(self):
        if utils.in_code_tree():
            self.add_option("-Daskap_version='%s'" 
                            % utils.get_release_version())
            self.add_option('-Dclasspath=%s' % self.dep.get_classpath())
            self.add_option('-Dinstall.dir=%s' % self.installdir)
            # Add dependencies' directories for ant
            for kv in self.dep.get_rootdirs(mapped=True):
                self.add_option('-Ddep_%s=%s' % kv)

    def add_run_script(self, name, entrypoint):
        """Add shell wrapper scripts for deployed and development environments
        
        :param name: name of the script e.g. monica.sh
        :param entrypoint: entrypoint of jar e.g. askap/monicalocal/MoniCAManager
        
        """
        self._run_script.append((name, entrypoint))

    def _create_run_script(self):
        env = utils.Environment(self.dep, self._prefix,
                                os.path.join(self._bdir, self._infofile),
                                development=False)
        env2 = utils.Environment(self.dep, self._prefix,
                                 os.path.join(self._bdir, self._infofile),
                                 development=True)
        for sname, entrypoint in self._run_script: 
            bindir = os.path.join(self._prefix, 'bin')
            if not os.path.exists(bindir):
                os.makedirs(bindir)
            exename = os.path.join(bindir, sname)
            #remove trailing lib entry
            cp = env['CLASSPATH'].rsplit(":",1)[0]
            body = """#!/bin/sh\nexec /usr/bin/java $JAVA_OPTS -cp {0} {1} $*"""
            with open(exename, "w") as of:
                of.write(body.format(cp, entrypoint))
            os.chmod(exename, 0755)

            # use own jar from current dir
            jar, cp = env2['CLASSPATH'].split(":",1)
            jar = jar.rsplit("/", 1)[-1]
            cp = ":".join((jar,cp))
            with open(sname, "w") as of:
                of.write(body.format(cp, entrypoint))
            os.chmod(sname, 0755)
            self.add_extra_clean_targets(sname)

                     
    def _install(self):
        self._add_ant_options()
        if not utils.in_code_tree():
            self.add_option('-Dprefix=%s' % self._prefix)
        utils.run("%s %s install" %
                     (self._icom, self._opts), self.nowarnings)
        if self._run_script:            
            self._create_run_script()

    def _test(self):
        self._add_ant_options()
        if utils.in_code_tree():
            utils.run("%s %s test" % (self._icom, self._opts))


    def _doc(self):
        self._add_ant_options()
        if utils.in_code_tree():
            utils.run("%s %s doc" % (self._icom, self._opts))
