# @file
# Object to simplify unpacking/building/cleaning of ASKAPsoft packages
#
# @copyright (c) 2007-2014 CSIRO
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
# @author Malte Marquarding <Malte.Marquarding@csiro.au>
#

import glob
import os
import pkg_resources
import platform
import re
import shutil
import sys
import urllib2
from ..exceptions import BuildError

if (sys.version_info[0] < 3) and (sys.version_info[1] < 6):
    print "error: Python versions less than 2.6 are unsupported. Exiting."
    sys.exit(1)

from hashlib import md5

import askapdev.rbuild.utils as utils
import askapdev.rbuild.utils.pkginfo as pkginfo
from askapdev.rbuild.dependencies import Dependency


class Builder:
    '''
    Object to set up a build environment for ASKAPsoft packages.
    This object is an abstract base class and should be subclassed for
    the different build tools.
    '''
    def __init__(self, pkgname=None, archivename=None, extractdir=None,
                 buildsubdir=None, buildcommand=None, buildtargets=[],
                 confcommand=None, installcommand=None, options=sys.argv[1:]):
        '''
        The constructor sets up a package build "environment"

        :param pkgname:        The (optional) name of the package directory.
                               By default the current directory name is used.
        :param archivename:    The (optional) archive name minus suffix.
                               The default is based on package name.
        :param extractdir:     The (optional) directory into which the archive
                               is extracted. It is created if it does not exist.
        :param buildsubdir:    The (optional) directory in which to start the
                               build.
        :param buildtargets:   The (optional) additional build targets.
        :param buildcommand:   The (optional) build command.
                               The default is 'None'.
        :param installcommand: The (optional) install command.
                               The default is 'None'.
        :param confcommand:    The command to configure the package.
                               the default is 'None'.
                               Otherwise the default is None.
        :param installcommand: The command to install the package.
                               The default is 'None'.
        :param options:        The default is 'sys.argv[1:]'.
        '''
        self._bdir = os.path.abspath(os.curdir)
        self._package = pkgname or os.path.basename(self._bdir)
        self._tarname = None
        self._archive_name = archivename
        self._extractdir = extractdir
        if buildsubdir:
            self._builddir = os.path.join(self._package, buildsubdir)
        else:
            self._builddir = self._package
        self._bcom = buildcommand
        self._btargets = buildtargets
        self._ccom = confcommand
        self._icom = installcommand
        self._patches = []
        self._replacement_list = []
        self._askaproot = os.getenv('ASKAP_ROOT')
        self._infofile = 'package.info'
        self._installdir = "install"
        self._pkgsig = '.packagesig'
        self._prefix = os.path.join(self._bdir, self._installdir)
        self.parallel = True
        # filter out general builder options
        if "-noparallel" in options:
            options.remove("-noparallel")
            self.parallel = False

        # XXX stage_dir and release name is set in rbuild but define here
        # to allow for testing using:
        # python build.py [stage, release]
        self._releasename = 'release-%s' % utils.get_svn_revision()
        self._stagedir = os.path.join(self._askaproot, 'tmp',
                                      self._releasename)
        # Iterate backwards because we are changing options list.
        for i in range(len(options) -1, -1, -1):
            if options[i].startswith('stage_dir'):
                self._stagedir = options[i].split('=')[1]
                options.pop(i)
            elif options[i].startswith('release_name'):
                self._releasename = options[i].split('=')[1]
                options.pop(i)

        self.create_signature = True
        self.version_string = ""
        self.remote_archive = None

        if utils.in_code_tree():
            self.create_signature = False
            self.version_string = utils.get_release_version()
        self._comopts = options
        self._opts = ""
        self._precallback = None
        self._postcallback = None
        self._files = []
        self._install_files = []
        self._clean_file = os.path.abspath(".clean_targets")
        self._clean_targets = self._load_clean_targets()
        self._buildfile = "build.py" # XXX could be setup.py
        self.nowarnings = False # enable or disable printing of warnings
        self._add_patches()
        self._platform = utils.get_platform()
        self._arch = self._platform['architecture']
        self._system = self._platform['system'].lower()
        self._hostname = self._platform['hostname'].lower()
        self._initscript_name = "init_package_env.sh"
        self.no_initscript = False
        self.add_extra_clean_targets(self._initscript_name)
        self.do_clean = True # default is to clean before building.
        
        siglist = [self._buildfile] + utils.get_svn_files_list()
        self._siglist = list(set(siglist))

        self._init_dependencies()


    def _fetch_remote(self):
        if utils.in_code_tree():
            return
        if self.remote_archive is None:
            return
        uitem = urllib2.urlparse.urlsplit(self.remote_archive)
        outfile = os.path.split(uitem.path)[-1]
        if (os.path.isfile(outfile)):
            return
        fullpath = self.remote_archive
        if not uitem.scheme:
            root = os.environ["RBUILD_REMOTE_ARCHIVE"]
            fullpath = os.path.sep.join((root, self.remote_archive))
        remote = urllib2.urlopen(fullpath)
        utils.q_print("info: Fetching '{}'...".format(fullpath))
        with open(outfile, "wb") as of:
            of.write(remote.read())
        remote.close()

    def _load_clean_targets(self):
        existing = []
        if os.path.exists(self._clean_file):
            with open(self._clean_file, "r") as fh:
                existing = [ line.strip() for line in fh ]
        return set(existing)

    def _save_clean_targets(self):
        if len(self._clean_targets) == 0:
            return
        with open(self._clean_file, "w") as fh:
            for tgt in self._clean_targets:
                fh.write(tgt+"\n")

    def _init_dependencies(self):
        self.dep = Dependency()
        self.dep.add_package()


    def _create_init(self):
        '''
        Create an init script for this package if in Code or 3rdParty.
        '''
        if self.no_initscript or self._bdir.find('Tools') != -1:
            return
        env = utils.Environment(self.dep, self._prefix,
                                 os.path.join(self._bdir, self._infofile))
        utils.create_init_file(self._initscript_name, env)


    def add_precallback(self, function):
        '''
        Add a callback to be executed immediately after unpacking.
        :param function: The callback function.
        '''
        self._precallback = function


    def add_postcallback(self, function):
        '''
        Add a callback to be executed after installing the package.
        :param function: the callback function
        '''
        self._postcallback = function


    def add_patches_from_dir(self, patchdir, prefix="patch-"):
        '''
        Add patches from a given patch directory.
        The patch files in this directory MUST include a relative path to
        file to patch.  The relative path is from the top-level of the
        unpacked tarball.

        This method is called by __init__() with a patchdir 'files'
        and default prefix 'patch-'.
        The patch filenames are added to self._patches list as a single string.

        :param patchdir: The directory that contains the patch files.
        :param prefix:   A prefix string which identifies patch filenames
                        [default 'patch-'].
        '''
        for patchfile in glob.glob('%s/%s*' % (patchdir, prefix)):
            self._patches.append(patchfile)


    def add_option(self, option):
        '''
        Add a configure option. Can specify multiple options separated by
        spaces.

        :param option: The option string, e.g. "--without-pgplot"
        '''
        self._opts += " " + option


    def add_env(self, key, value): # pylint: disable-msg=R0201
        '''
        Add a variable to the environment (replacing an existing one).

        :param key:   The environment variable name, e.g. "CPPFLAGS".
        :param value: The value of environment variable, e.g. "-Wall".
        '''
        os.environ[key] = value


    def append_env(self, key, value): # pylint: disable-msg=R0201
        '''
        Append a string to an environment variable (automatically separated
        by a space).

        :param key:   The environment variable name, e.g. "CPPFLAGS"
        :param value: The value to be appended, e.g. "-Wall"
        '''
        if os.environ.has_key(key):
            os.environ[key] = ' '.join((os.environ[key], value))
        else:
            os.environ[key] = value


    def add_ld_library_paths(self, libdirs):
        '''
        Add one or more library directories to the shared library directory
        search list.  The directories will be added at the front of the
        LD_LIBRARY_PATH (Linux) or DYLD_LIBRARY_PATH (Darwin) path list
        environment variable.
        The directories should be combined into a single string, with ':'
        as separator.

        :param libdirs:  A string containing the directories to be added to
                         (DY)LD_LIBRARY_PATH.
        '''
        if sys.platform == 'darwin':
            var = 'DYLD_LIBRARY_PATH'
        else:
            var = 'LD_LIBRARY_PATH'
        existingPaths = os.environ.get(var, '')
        if len(existingPaths) > 0:
            libdirs = ':'.join((libdirs, existingPaths))
        self.add_env(var, libdirs)


    def add_classpath_paths(self, classpaths):
        '''
        Add one or more library directories to the CLASSPATH directory
        search list.
        The directories will be added at the front of the CLASSPATH
        environment variable.
        The directories should be combined into a single string, with ':'
        as separator.

        :param classpaths: string containing the directories to be added to
                           the CLASSPATH.
        '''
        existingPaths = os.environ.get('CLASSPATH', '')
        if len(existingPaths) > 0:
            libdirs = ':'.join((classpaths, existingPaths))
        self.add_env('CLASSPATH', classpaths)


    def add_file(self, source, destination=None):
        '''
        Copy a file into the package before building

        :param source:      The filename of the file to copy.
        :param destination: The optional destination (package relative).
                            The default is the root level of the package
        '''

        if os.path.isfile(source):
            self._files.append((source, destination, False))
        elif os.path.isdir(source):
            for dname, dirs, files in os.walk(source):
                if '.svn' in dname:
                    continue
                for fname in files:
                    dest = dname.split(os.sep)[1:]
                    dest = os.sep.join(dest)
                    self._files.append((os.path.join(dname, fname), dest, False))

    def _copy_to_install(self):
        for src, tgt, rename in self._install_files:
            tdir = self._prefix
            if tgt:
                tdir = os.path.join(tdir, tgt) 
            if os.path.isdir(src):
                utils.copy_tree(src, tdir)
            else:
                dtgt = tdir
                if rename and tgt:
                    dtgt = os.path.dirname(tdir)
                if not os.path.exists(dtgt):
                    os.makedirs(dtgt)
                shutil.copy(src, tdir)


    def add_install_file(self, source, destination=None, rename=False):
        '''
        Copy a file into the installation

        :param source:      The filename of the file to copy.
        :param destination: The optional destination (install dir relative).
                            The default is the root level of the install dir
        :param rename:      `destination` is a new file name
        '''
        if destination is None:
            rename = False
        self._install_files.append((source, destination, rename))

    def add_extra_clean_targets(self, *targets):
        '''
        Add in extra directories to be cleaned up.

        :param targets: a list of additional directories to be removed.
                        e.g. add_extra_clean_targets('bin', 'lib', 'etc')
        '''
            
        for tgt in targets:
            pth = os.path.abspath(tgt)
            #if os.path.exists(pth):
            self._clean_targets.add(pth)

    def get_parallel_opt(self, j='auto', max_j=16):
        '''
        Ask for parallel build. Default is 'auto' which scales with number
        of cores in a machine + 1.

        :param j:     The number of build threads (default is 'auto').
        :param max_j: The maximum number of build threads (default is '16').
        '''
        if not self.parallel:
            return ""

        for opt in self._comopts:
            if opt.startswith('j='):
                j = opt.split('=')[-1]
                return " -j%d" % int(j)

        if j in ['auto', '', None, 0]:
            j = utils.number_of_cpus() + 1
        else:
            try:
                j = abs(int(j))
            except ValueError:
                utils.q_print("warn: non-numeric j value given - setting to 2")
                j = 2
        try:
            max_j = abs(int(max_j))
        except ValueError:
            utils.q_print("warn: non-integer max_j value given - setting to 16")
            max_j = 16
        if j > max_j: # sanity check
            utils.q_print("warn: j value %d is greater than max_j value %d" % (j, max_j))
            j = max_j
        if j > 1:
            return " -j%d" % j
        return ""


    def replace(self, filename, target, replacement):
        '''
        Add tuple of filename, target string and replacement string
        to allow replacement of text in a given file.


        :param filename:    The filename to be modified (relative to the
                            build directory).
        :param target:      The string to be replaced.
        :param replacement: The replacement string.
        '''
        self._replacement_list.append((filename, target, replacement))


    def _replace(self):
        '''
        Do the actual replacement of text in given files.
        '''
        for item in self._replacement_list:
            filename, target, replacement = item
            fhandle = open(os.path.join(self._builddir, filename), "r+")
            data = fhandle.read()
            fhandle.seek(0)
            fhandle.truncate()
            retarget = re.compile(target, re.MULTILINE)
            data = retarget.sub(replacement, data)
            fhandle.write(data)
            fhandle.close()


    def _create_release_envfile(self):
        '''
        For releases, everything is combined into a single tree which
        may be installed anywhere in the target system.
        Create an initialization file that will just need the location
        of the tree in order to define the required shell environment.
        '''
        env = utils.Environment(self.dep, self._prefix,
                                os.path.join(self._bdir, self._infofile),
                                development=False)
        template = "../../../templates/initenv.py.tmpl"
        filename = os.path.join(self._stagedir, "initenv.py")
        data = pkg_resources.resource_string(__name__, template)

        for k, v in env.items():
            k = "%%%"+k+"%%%"
            data  = data.replace(k, v)
        data = data.replace("%%%ldprefix%%%", env.ld_prefix)
        with open(filename, "w") as fh:
            fh.write(data)

    def _update_easyinstall(self):
        '''
        Update the release easy-install.pth file if it exists.
        '''
        curdir = os.getcwd()
        pysite = os.path.join(self._stagedir, "lib",
                              "python%s" % sys.version[0:3], "site-packages")
        pthfile = os.path.join(pysite, "easy-install.pth")
        os.chdir(pysite)
        eggs = ["%s\n" % egg for egg in glob.glob("./*.egg")]
        with open(pthfile, "r+") as pth:
            contents = pth.readlines()
            contents = ''.join(contents[0:1] + eggs + contents[-1:])
            pth.seek(0)
            pth.write(contents)
        os.chdir(curdir)


    def doc_stage(self, stage_dir):
        '''
        Copy (sphinx) documentation into the `stage_dir` and fix u
        intersphinx cross-references to be document root relative 
        (see :issue:`4775`)

        :param stage_dir: the name of the direcotry into which a release 
                          is staged before creating tarball.
        '''
        sphinxdir = os.path.join("doc","_build","html")
        if not os.path.exists(sphinxdir):
            return
        utils.q_print("info: running the documentation stage process")
        dst = os.path.abspath(self._builddir).replace(self._askaproot,
                                                      stage_dir)
        relpath = os.path.relpath(self._askaproot)
        sphinxdir = os.path.join("doc","_build","html")+os.path.sep
        dst = os.path.join(dst, self._builddir)
        if not os.path.exists(dst):
            os.makedirs(dst)        
        utils.copy_tree(sphinxdir, dst, symlinks=True,
                        overwrite=True)
        utils.q_print("info: updating documentation cross-references")

        for dname, dirs, files in os.walk(dst):
            for fname in files:
                if not fname.endswith(".html"):
                    continue
                fpath = os.path.join(dname, fname)
                with open(fpath) as f:
                    s = f.read()
                s = s.replace(self._askaproot, relpath)
                s = s.replace(sphinxdir, "")
                with open(fpath, "w") as f:
                    f.write(s)


    def _stage(self):
        '''
        Stage packages (or dependent packages) into a temporary release
        tree as a prerequsite for creating a package tarball.
        '''
        utils.q_print("info: running the stage process")
        if not os.path.exists(self._stagedir):
            os.makedirs(self._stagedir)

        if os.path.exists(self._installdir):
            os.chdir(self._installdir)

            for src in glob.glob("*"):
                dst = os.path.join(self._stagedir, src)
                # we do a binary release
                if src == 'include':
                    continue
                if src in ['lib', 'lib64']:
                    # we do a binary release
                    pattern = '*.so*'
                    if sys.platform == 'darwin':
                        pattern = '*.dylib*'
                    utils.copy_tree(src, dst, symlinks=True, pattern=pattern,
                            overwrite=True)
                    utils.copy_tree(src, dst, symlinks=True, pattern='*.jar*',
                            overwrite=True)
                elif src == "VERSION":
                    with open(os.path.join(self._stagedir, 
                                           "PACKAGE_VERSIONS"), "a+") as f:
                        f.write(open(src).read())
                else:
                    utils.copy_tree(src, dst, overwrite=True)

            os.chdir("..")
            bindir = os.path.join(self._stagedir, "bin")
            for shfile in glob.glob("%s/*.sh" % bindir):
                data = open(shfile).read()
                if data.find("ASKAP auto-generated") > 0:
                    utils.q_print("info: removing ASKAP shell wrapper file %s" %
                                   shfile)
                    os.remove(shfile)
        else: # may be a document only package.
            utils.q_print('warn: %s does not exist.' % self._installdir)

        self.doc_stage(os.path.join(self._stagedir, "sphinxdoc"))


    def _release(self):
        '''
        Create a tarball of the staged location.
        The rbuild program has -N option that stops the adding of virtualenv
        to the release.  This is for developers sharing purely python packages.
        In this case there is no point creating the release envfile and
        cannot update the easy-install.pth file.
        '''
        utils.q_print("info: running the release process")
        utils.q_print("info: resetting python scripts to use '#!/usr/bin/env python'" )
        bindir = os.path.join(self._stagedir, "bin")
        for fn in glob.glob("%s/*" % bindir):
            sedcmd = "sed -i -e '1s&^#\!.*/bin/python&#\!/usr/bin/env python&' %s" % fn
            utils.runcmd(sedcmd, shell=True)

        if os.path.exists(os.path.join(self._stagedir, "bin", "python")):
            self._create_release_envfile()
            self._update_easyinstall()
        dn, bn = os.path.split(self._stagedir.rstrip("/"))
        if not dn:
            dn = '.'
        utils.q_print("info: creating %s.tgz" % self._releasename)
        utils.runcmd("tar -C %s -czf %s.tgz %s" % (dn, self._releasename, bn))
        utils.rmtree(self._stagedir)

    def _deploy(self):
        utils.q_print("error: deploy target is deprecated.")
        sys.exit(1)


    def _precommand(self):
        if self._precallback is not None:
            self._precallback()


    def _postcommand(self):
        if self._postcallback is not None:
            self._postcallback()


    def _build_clean(self):
        for path in [self._package]:
            if os.path.exists(path):
                utils.rmtree(path)


    def _clean(self):        
        cpaths = list(self._clean_targets) + \
            [self._pkgsig, self._installdir, ".sconsign.dblite"]
        if os.path.exists(self._infofile):
            with open(self._infofile, "r") as fh:
                if fh.readline().find("# Auto-generated by build.py") >= 0:
                    cpaths.append(self._infofile)
        if not utils.in_code_tree():
            cpaths += [self._package]

        if os.path.exists(self._clean_file):
            with open(self._clean_file, "r") as fh:
                for line in fh:
                    cpaths.append(line.strip())
        cpaths.append(self._clean_file)

        for path in cpaths: 
            if os.path.exists(path):
                utils.rmtree(path)
        return True


    def _add_patches(self):
       p = utils.get_platform()
       top = 'files'
       self.add_patches_from_dir(top)

       d = top
       for k in ['architecture']:
           if p[k] != '':
                d = os.path.join(d, p[k])
                self.add_patches_from_dir(d)

       d = top
       for k in ['system', 'distribution', 'version', 'architecture']:
           if p[k] != '':
                d = os.path.join(d, p[k])
                self.add_patches_from_dir(d)


    def _configure(self):
        if self._ccom:
            utils.run(self._ccom, self.nowarnings)


    def _build(self):
        if self._bcom:
            utils.run(self._bcom, self.nowarnings)


    def _install(self):
        if self._icom:
            utils.run(self._icom, self.nowarnings)
        self._version_install()


    def _version_install(self):
        if self.version_string:
            vfile = os.path.join(self._installdir, "VERSION")
            if os.path.exists(self._installdir):
                with open(vfile, 'w') as f:
                    f.write(self.version_string+"\n")


    def _lib64_symlink(self):
        # Create lib64->lib or lib->lib64 symlinks on Linux 64 bit machines
        if self._system == 'linux' and self._arch == '64bit':
            # For virtual packages, do not create symlinks as they will be
            installdir = os.path.join(self._bdir, self._installdir)
            if os.path.realpath(installdir).find(self._askaproot) < 0:
                return

            ldir = os.path.join(self._installdir, 'lib')
            ldir64 = '%s64' % ldir

            if os.path.exists(ldir64) and os.path.isdir(ldir64) and not \
                os.path.exists(ldir):
                os.symlink("lib64", ldir)
            elif os.path.exists(ldir) and os.path.isdir(ldir) and not \
                os.path.exists(ldir64):
                os.symlink("lib", ldir64)


    def _get_archive_name(self, aname):
        archive = aname or self._package
        for suffix in [".tar.gz", ".tar.bz2", ".tgz", ".zip", ".jar"]:
            if os.path.exists(archive + suffix):
                archive += suffix
                return archive
        if os.path.abspath(os.curdir).find("3rdParty") > 0 and aname:
            utils.q_print("warn: Expected archive (%s) but none found." % aname)
        return None


    def _determine_tarfile(self):
        if self.remote_archive is not None:
            self._tarname = os.path.split(self.remote_archive)[-1]
        if self._tarname is None:
            self._tarname = self._get_archive_name(self._archive_name)
        if self._tarname:
            sig = set(self._siglist)
            sig.add(self._tarname)
            self._siglist = list(sig)

    def _unpack(self):
        if (os.path.exists(self._package) and self.is_up_to_date()) \
                or self._tarname is None:
            return
        suffdict = { ".gz": "tar zxf", ".tgz": "tar zxf", ".bz2": "tar jxf",
                     ".zip": "unzip", ".jar": "jar xf"}
        found = False
        for key, value in suffdict.iteritems():
            if self._tarname.endswith(key):
                unpack = value
                found = True
                break
        if not found:
            raise BuildError("source archive not found")

        if self._extractdir:
            unpack += " %s" % os.path.join(self._bdir, self._tarname)
            utils.run("mkdir -p %s" % self._extractdir)
            os.chdir(self._extractdir)
            utils.run("%s" % unpack)
            os.chdir(self._bdir)
        else:
            unpack += " %s" % self._tarname
            utils.run("%s" % unpack)


    def _copy(self, sources, target):
        '''
        Copy files or directories using package relative dir
        Takes self._files and copies them from source to destination
        If 'rename' is set and src is a file it will be renamed
        '''
        for src, tgt, rename in sources:
            if tgt:
                tgt = os.path.join(target, tgt)
            else:
                tgt = target

            dtgt = tgt
            if rename and os.path.isfile(src):
                dtgt = os.path.dirname(tgt)
            if not os.path.exists(dtgt):
                os.makedirs(dtgt)

            shutil.copy(src, tgt)


    def _patch(self):
        for patch in self._patches:
            # Always exit patching ok as -N will still return status>1
            # for already applied patches.
            utils.q_print("info: applying patch '%s'." % patch)
            utils.run("patch -N -p0 -s -d %s < %s; exit 0" %
                                          (self._package, patch))

    def _test(self):
        pass


    def _get_env(self):
        env = utils.Environment(self.dep, self._prefix,
                                os.path.join(self._bdir, self._infofile))
        envitems = []
        envstr = ""
        for k, v in env.items():
            if v:
                v = os.path.expandvars(v)
                envitems.append("%s=%s" % (k, v))
                # append original PATH
                if k == "PATH":
                    envitems[-1] += ":$%s" % k
                elif k == "LD_LIBRARY_PATH":
                    envitems[-1] = env.ld_prefix + envitems[-1]
        if len(envitems):
            envstr  = " ".join(['env']+envitems)
        return envstr


    def _functest(self):
        # use nose by default
        target = 'functests'
        setup = os.path.join(target, 'setup.py')
        if os.path.exists(target):
            if os.path.exists(setup):
                os.chdir(target)
                envstr = self._get_env()
                # handle explicit test target
                filetarget = ""
                for o in self._comopts:
                    if o.startswith(target+"/"):
                        filetarget = "--tests="+os.path.split(o)[-1]

                cmd = "%s python setup.py -q nosetests --with-xunit " \
                      "--xunit-file=%s-junit-results.xml %s" \
                      % (envstr, target, filetarget)
                clncmd = "python setup.py -q clean"
                utils.run("%s" % cmd, self.nowarnings, ignore_traceback=True)
                utils.runcmd("%s" % clncmd)
                os.chdir(self._bdir)
            else:
                pass # fail quietly as they may have run scons tests.
        else:
            print "error: missing functests subdirectory in %s" % \
                    os.path.relpath(self._bdir, self._askaproot)


    def _doc(self):
        pass


    ## Create a signature file containing md5sums of all files used
    # in the build.
    def _signature(self):
        if not self.create_signature:
            return
        outsig = file(self._pkgsig, "w")
        outsig.write("{\n")
        for fn in self._siglist + self._files + self._patches:
            if type(fn) == tuple: # added files are tuples (filename, dest)
                fn = fn[0]
            if fn and os.path.exists(fn):
                md5sig = md5(file(fn).read())
                outsig.write("'%s' : '%s',\n" % (fn, md5sig.hexdigest()))
            else:
                print("warn: cannot checksum as file %s does not exist." % fn)
        outsig.write("}\n")
        outsig.close()


    def _create_info(self):
        pkginfo.to_info()

    def is_up_to_date(self):
        '''
        Indicate if a package has been built and is up to date
        When a package is built, a package signature file (.packagesig)
        is created.  This is a mapping (dictionary) of filenames to md5
        checksums.  This procedure then checks if any of the files used
        in the build process have changed or if any new files have been
        added (which will not have md5 checksums recorded).
        '''
        if not self.create_signature:
            return False

        if not os.path.exists(self._pkgsig):
            return False

        fh = open(self._pkgsig, "r")
        try:
            sigdict = eval(fh.read(), {"__builtins__":None}, {})
        except (TypeError, SyntaxError, ValueError):
            return False

        for fn in self._siglist + self._files + self._patches:
            if type(fn) == tuple: # added files are tuples
                fn = fn[0]
            if fn and os.path.exists(fn):
                md5cur = md5(file(fn).read()).hexdigest()
                if md5cur != sigdict.get(fn): # .get() handles missing keys
                    return False
            else:
                return False

        return True


    def _get_system_opts(self):
        '''
        Add in the default/platform/hostname options.
        '''
        envfiles = ['%s/env.default' % self._askaproot,
                    '%s/env.%s' % (self._askaproot, self._system),
                    'env.' + self._system,
                    'env.' + self._hostname,
                   ]
        for env in envfiles:
            if os.path.exists(env):
                utils.q_print("info: processing environment file: %s" % env)
                opts = []
                for line in open(env, "r"):
                    line = line.strip()
                    if not line or not line.startswith("#"):
                        opts.append(line)
                self._opts += " " + " ".join(opts)
                utils.q_print("debug: using self._opts =>%s<=" % self._opts)


    def build(self):
        '''
        Run the complete build process.
        XXX rbuild script only allows single build targets to be specified
            but 'python build.py <targets>' allows for multiple targets
            to be specified.
        '''
        # get tar file name in case of a remote archive
        self._determine_tarfile()
        if "bclean" in self._comopts:
            self._build_clean()
        if "clean" in self._comopts:
            self._clean() # will remove .packagesig
        if "depends" in self._comopts:
            pass # handled by rbuild but for completeness list here.
        if "install" in self._comopts:
            if self.is_up_to_date():
                return
            # Run clean before building to get rid of possible old artifacts
            # in installdir.
            # The do_clean flag should only be set False for second builders.
            # In Code just remove the installdir rather than running _clean() 
            # so rebuilds will be fast and handled by underlying build
            # system e.g. scons
            if self.do_clean:
                if utils.in_code_tree():
                    if os.path.exists(self._installdir):
                        utils.q_print("info:    pre-build removal of %s dir." %
                                                              self._installdir)
                        utils.rmtree(self._installdir)
                else:
                    utils.q_print("info:    pre-build clean.")
                    self._clean()
            self._get_system_opts()
            self._fetch_remote()
            self._unpack()
            self._copy(self._files, self._package)
            self._patch()
            self._replace()
            self._precommand()
            os.chdir(self._builddir)
            self._configure()
            self._build()
            self._install()
            os.chdir(self._bdir)
            self._copy_to_install()
            self._lib64_symlink()
            self._create_info()
            self._create_init()
            self._postcommand()
            self._signature()
            self._save_clean_targets()
        if "test" in self._comopts and utils.in_code_tree():
            os.chdir(self._package)
            self._test()
            os.chdir(self._bdir)
        if "functest" in self._comopts and utils.in_code_tree():
            os.chdir(self._package)
            self._functest()
            os.chdir(self._bdir)
        if "signature" in self._comopts:
            self._signature()
        if "stage" in self._comopts:
            self._stage()
        if "release" in self._comopts:
            self._release()
        if "deploy" in self._comopts:
            self._deploy()
        if "format" in self._comopts:
            if utils.in_code_tree():
                utils.format_src_code()
            else:
                utils.q_print("warn: format only applies in the Code tree.")
        if "doc" in self._comopts:
            os.chdir(self._package)
            self._doc()
            os.chdir(self._bdir)
