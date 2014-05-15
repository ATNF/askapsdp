## @metapackage.py
# Object to simplify unpacking/building/cleaning of ASKAPsoft systems
# metapackages
#
# @copyright (c) 2007-2013 CSIRO
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
# @author JC Guzmn <Juan.Guzman@csiro.au>
#

import os
import datetime
import sys
import socket
from builder import Builder
import askapdev.rbuild.utils as utils


## Implementation of Builder for a metapackage. A metapackage is a package
#  that do not contain any software, instead contains configuration files
#  and dependencies. This metapackage is used to create a unique software release
#  for a collection of several packages, for example TOS, CP.
#  It copies directory tree specified in the constructor into install directory.
#  If Macro Substitutions and Include (MSI) files and directories are added it requires the msi tool (part of EPICS
#  extensions) to be present in the dependencies. MSI is used to create final files from templates and/or
#  substitutions files.
#  It also creates config jar files specified in the add_config_jar_files() method.
#  It also creates a RELEASE information file that includes: meta-package name, version, username, hostname and
#  all the dependencies list.
#  It overwrites _install() method.
class Bunch(object):
    def __init__(self, **kwds):
        self.__dict__.update(kwds)
        
class MetaPackage(Builder):
    def __init__(self, metapkgname=None, datadir=None, msibasedir=None, jarbasedir=None):
        ## The constructor sets up a package build "environment"
        #  @param self           The current object.
        #  @param metapkgname    The name of the meta-package directory. By default the
        #  current directory name is used (basename of current directory).
        #  @param datadir        The base directory name to be copied recursively in install dir, i.e. every file and
        #  subdirectory will be copied recursively into install directory. Defaults to "files/data".
        #  @param msibasedir     The base directory name of the msi template/substitutions files and subdirectories.
        #  It is pre-pended to every input directory entered in add_msi_template() and add_msi_template_dir() calls.
        #  Defaults to "files/msi".
        #  @param jarbasedir     The base directory name of the jar configuration directories. It is pre-pended to
        #  input directories entered in the add_config_jar_dir() method.
        Builder.__init__(self, pkgname='.')
        self._quiet = False
        if '-q' in self._comopts or '--quiet' in self._comopts:
            self._quiet = True
        self._versionstr = str(utils.get_svn_branch_info())
        self._metapkgname = metapkgname or os.path.basename(os.path.abspath(os.curdir))
        self._datadir = datadir or os.path.join('files', 'data')
        # List of directories that needs to be copied to installdir
        self._datatrees = []
        if os.path.exists(self._datadir):
            self._datatrees.append(self._datadir)
        # List of jar command and outputdir that needs to be executed
        self._jar_cmdlist = []
        self._jarcmd = 'jar cvf'
        self._jarbasedir = jarbasedir or os.path.join('files', 'jar')
        # List of msi commands and outputdir that needs to be executed
        self._msibasedir = msibasedir or os.path.join('files', 'msi')
        self._msi_cmdlist = []
        # Check that msi tool is in the dependencies
        try:
            msipath = self.dep.get_install_path('msi')
            self._msi_in_dep = True
        except:
            msipath = ''
            self._msi_in_dep = False
        ea = self.dep.get_env().get('EPICS_HOST_ARCH', None)
        if ea is None:
            print "Couldn't determine EPICS architecture"
            sys.exit(1)
        os.environ['EPICS_HOST_ARCH'] = ea
        self.epics_host_arch = ea
        self._msicmd = ''
        if self._msi_in_dep:
            self._msicmd = os.path.join(msipath, 'bin', ea, 'msi')
            
    def _add_msi_cmd(self, template=None, subsfile=None, outfile=None, includes=None, subs=None, fmode=None):
        # Forms the argument string for the msi command
        if outfile is None or outfile == '':
            print "Warning! adding a msi command without output file (template=%s, subsfile=%s)" % (template, subsfile)
            return
        # Extract the output file path
        outpath = os.path.split(outfile)[0]
        msiargs = ''
        if template is not None:
            msiargs = "%s" % template
        msiargs = "-o %s %s" % (outfile, msiargs)
        if subsfile is not None and subsfile != '':
            msiargs = "-S %s %s" % (subsfile, msiargs)
        msisubs = ''
        if subs is not None:
            for m in subs:
                msisubs = "-M %s %s" % (m, msisubs)
            msiargs = "%s %s" % (msisubs, msiargs)
        msiinc = ''
        if includes is not None:
            for dir in includes:
                msiinc = "-I%s %s" % (dir,msiinc)
            msiargs = "%s %s" % (msiinc, msiargs)
        self._msi_cmdlist.append(Bunch(outdir=outpath, msiargs=msiargs, outfile=outfile, fmode=fmode))

    ## Add msi template file (invokes msi tool with a template input file)
    #  @param self           The current object
    #  @param inputfile      The input file name (including extension) and path
    #  relative to the base directory (self._msibasedir or basedir argument).
    #  @param basedir        The base directory name (defaults to self._msibasedir) to
    #  be pre-pended to the inputfile and include paths to form the full input path template to the msi
    #  tool.
    #  @param outdir         The output directory relative to the install directory.
    #  Defaults to head of inputfile (path until last /).
    #  @param templinc       Include path list for searching included templates relative (-I) to msi base
    #  directory. By default it adds msi base directory into the include paths.
    #  @param subs           Macro values list (-M argument to msi tool).
    def add_msi_template(self, inputfile, basedir=None, outdir=None, templinc=None, subs=None):
        bdir = basedir or self._msibasedir
        outputdir = outdir or os.path.split(inputfile)[0]
        outfile = os.path.splitext(os.path.basename(inputfile))[0]
        p = os.path.split(inputfile)[0]
        if p == '':
            newinc = [bdir]
        else:
            newinc = [os.path.join(bdir, p)]
        if templinc is not None:
            for inc in templinc:
                newinc.append(os.path.join(bdir, inc))
        fmode = os.stat(os.path.join(bdir, inputfile)).st_mode
        self._add_msi_cmd(os.path.join(bdir,inputfile), '', os.path.join(self._installdir, outputdir, outfile), includes=newinc, subs=subs, fmode=fmode)

    ## Add msi template directory. Searches for ".template" files and add them to the msi list
    #  @param self           The current object
    #  @param inputdir       The input directory name relative to base directory (see next param).
    #  @param basedir        The base directory name (defaults to self._msibasedir) to
    #  be pre-pended to the input directory and include paths to form the full input path to the msi tool.
    #  @param outdir         The output directory relative to the install directory.
    #  Defaults to input directory.
    #  @param templinc       Include path list for searching included templates relative (-I) to msi base
    #  directory. By default it adds msi base directory into the include paths.
    #  @param subs           Macro values list (-M argument to msi tool) to be applied to the entire directory.
    def add_msi_template_dir(self, inputdir, basedir=None, outdir=None, templinc=None, subs=None):
        bdir = basedir or self._msibasedir
        outputdir = outdir or inputdir
        inputpath = os.path.join(bdir, inputdir)
        for fname in os.listdir(inputpath):
            if fname.endswith(".template"):
                self.add_msi_template(inputfile=os.path.join(inputdir,fname), basedir=bdir, outdir=outputdir, templinc=templinc, subs=subs)

    ## Add msi substitutions directory. Searches for ".substitutions" files and add them to the msi command
    #  list using the dbTemplate format (no input template file required)
    #  @param self           The current object
    #  @param inputdir       The input directory name relative to base directory (see next param).
    #  @param basedir        The base directory name (defaults to self._msibasedir) to
    #  be pre-pended to the input directory and include paths to form the full input path to the msi tool.
    #  @param outdir         The output directory relative to the install directory.
    #  Defaults to input directory.
    #  @param templinc       Include path list for searching included templates relative (-I) to msi base
    #  directory. By default it adds msi base directory into the include paths.
    #  @param subs           Macro values list (-M argument to msi tool) to be applied to the entire directory.
    def add_msi_subs_dir(self, inputdir, basedir=None, outdir=None, templinc=None, subs=None):
        bdir = basedir or self._msibasedir
        outputdir = outdir or inputdir
        inputpath = os.path.join(bdir, inputdir)
        newtinc = [inputpath]
        if templinc is not None:
            for t in templinc:
                newtinc.append(os.path.join(bdir,t))
        outpath = os.path.join(self._installdir, outputdir)
        for fname in os.listdir(inputpath):
            if fname.endswith(".substitutions"):
                self._add_msi_cmd(subsfile=os.path.join(inputpath, fname), 
                                  outfile=os.path.join(outpath, os.path.splitext(fname)[0]),
                                  includes=newtinc, subs=subs)
    ## Add extra directory tree into the directory trees to be copied recursively into install
    #  @param self    The current object
    #  @param inputdir The input directory to be copied recursively into install directory.
    def add_data_tree(self, inputdir):
        self._datatrees.append(inputdir)

    ## Add jar configuration directory. The jar output file name corresponds 
    #  to the basename
    #  of the input directory.
    #  @param self    The current object
    #  @param inputdir The input directory containing files to be added to the 
    #   jar output file
    #  @param basedir  The base directory name to be pre-pended to the input directory. Defaults
    #  to self._jarbasedir (see class constructor).
    #  @param outdir   The output directory name. The output file path gets formed by joining
    #  the install, output directory and jar output file name.
    def add_config_jar_dir(self, inputdir, basedir=None, outdir=None):
        if basedir is None:
            inputpath = os.path.join(self._jarbasedir, inputdir)
        else:
            inputpath = os.path.join(basedir, inputdir)
        outpath = outdir or os.path.split(inputdir)[0]
        jarfile = os.path.basename(inputdir)
        jarout = os.path.join(self._installdir, outpath, jarfile)
        jararg = "%s.jar -C %s ." % (jarout, inputpath)
        self._jar_cmdlist.append(Bunch(outdir=os.path.join(self._installdir, outpath), jarargs=jararg))
        
    def _run_msi(self):
        if not self._msi_in_dep:
            print "Error: MSI command tool is not in the dependencies. Skipping MSI files"
            return
        for msibunch in self._msi_cmdlist:
            if not self._quiet:
                print "mkdir -p %s" % msibunch.outdir
            utils.run("mkdir -p %s" % msibunch.outdir)
            if not self._quiet:
                print "%s %s" % (self._msicmd, msibunch.msiargs)
            utils.run("%s %s" % (self._msicmd, msibunch.msiargs))
            if not self._quiet:
                print "Setting mode %o of output file %s" % (msibunch.fmode, msibunch.outfile)
            os.chmod(msibunch.outfile, msibunch.fmode)
            
    def _run_jar(self):
        for jarbunch in self._jar_cmdlist:
            if not self._quiet:
                print "mkdir -p %s" % jarbunch.outdir
            utils.run("mkdir -p %s" % jarbunch.outdir)
            if not self._quiet:
                print "%s %s" % (self._jarcmd, jarbunch.jarargs)
            utils.run("%s %s" % (self._jarcmd, jarbunch.jarargs))

    def _run_copy_trees(self):
        for dir in self._datatrees:
            if not self._quiet:
                print "Copying tree %s in install directory" % dir
            utils.copy_tree(dir, self._installdir, overwrite=True)

    ## Get all dependencies as single string
    def _get_dependencies(self):
        str = ''
        for k,v in self.dep._deps.iteritems():
            str += k + '=' + v['path'] +'\n'
        return str

    def _create_release_info(self, path):
        filepath = os.path.join(path, 'RELEASE')
        with file(filepath, 'w') as f:
            txt = """\
%s
Version = %s-rev%s
Released by = %s
Date = %s
Hostname = %s

Packages List
-------------
%s
""" % (self._metapkgname, self._versionstr, utils.get_svn_revision(), 
       os.getenv('LOGNAME'),
       datetime.datetime.today().strftime('%Y-%m-%d %H:%M:%S'),
       socket.gethostname(), self._get_dependencies())

            f.write(txt)
   
    def _install(self):
        Builder._install(self)
        # Create the install directory
        utils.run("mkdir -p %s" % self._installdir)
        self._run_copy_trees()
        # msi has to be first in the case we need to create jar files after 
        # post-executing msi tool
        self._run_msi()
        self._run_jar()
        self._create_release_info(self._installdir)
