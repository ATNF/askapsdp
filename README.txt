ASKAPsoft Science Data Processor                                   19 May 2014
================================

These notes provide just a brief outline for building the ASKAPsoft Science
Data Processor codebase. More detailed instructions exist on the ASKAP
Computing Redmine site.

Archive Repository
==================

ASKAPsoft depends on a number of 3rd party packages. Some of these must be
supplied by the operating platform. For example a compiler such as GCC and an
MPI implementation must be present.

Other software is downloaded during the ASKAPsoft build process, and as such
HTTP access is required. If your build host relies on a HTTP proxy to access
the internet then the http_proxy & https_proxy environment variables must be
set in your environment. Here is an example, where "webproxy.myorg.com" is the
hostname for the proxy server that is running on port 8080

    export http_proxy=http://webproxy.myorg.com:8080
    export https_proxy=$http_proxy

Alternatively, if access to download these files is not available at all on the
build host a local repository can be set up. In this case the bootstrap.ini
must be configured (prior to executing the bootstrap below) to refer to the
local cache via a "file" URL. For example:

    remote_archive = file:///home/joe/askapsoft_dependencies

Checkout & Bootstrap
====================

The first step of building ASKAPsoft involved checking out a copy of the
code from the Subversion repository and then running the bootstrap script.
The boostrap script build and configures the developer tools necessary for
building the rest of the ASKAPsoft codebase:

    svn co https://svn.atnf.csiro.au/askapsdp/trunk ASKAPsoft
    cd ASKAPsoft
    /usr/bin/python2.7 bootstrap.py -n

The "-n" option to bootstrap says not to use "svn update" to update the
workspace prior to doing the bootstrap. It can be omited, however access to
the above described archive repository is needed.

Building
========

Before building ASKAPsoft, certain environment variable need to be set.
This is accomplished by sourcing the initaskap.sh script, which will exist
in $ASKAP_ROOT after the bootstrap procedure described above has been
completed. Then the recursive builder (rbuild) can be used to build the
entire ASKAPsoft tree:

    cd ASKAPsoft
    . initaskap.sh
    rbuild -S -a

The "-S" option to rbuild says not to do an "svn update" prior to building.

To build a release tarball for a specific package, or set of packages the
release target option must be set. For example to build the core data
processing applications (assuming initaskap.sh has already been sourced):

    cd $ASKAP_ROOT
    cd Code/Systems/cpapps
    rbuild -S -t release

This will create a filenamed release-<svnrev>.tgz

rbuild
======

The 'rbuild' command is the main build command for developers.  It has the
ability of updating from the subversion repository and also recursively
resolve, update and build dependencies.

To get help for 'rbuild' simply type 

------------------------------------------------------------------------------
%% rbuild -h
Usage: rbuild [options] [<package_path>]
                                                                                                        
This is the main ASKAPsoft build command for developers. It can handle                                  
dependencies, subversion updates and changes to the build system. There are
two types of build targets: recursive [depends, install, stage, release,
signature] and non-recursive [bclean, clean, doc, format, pylint, functest,
test, deploy]. The non-recursive targets only apply to the current package,                             
while recursive targets are applies to all dependencies of the current                                  
package.  The recursive behaviour may be overridden with appropriate flag. The                          
default <package_path> is the current directory.                                                        
                                                                                                        
Options:
  -h, --help            show this help message and exit                                                                            
  -a, --autobuild       mode where dependencies/packages are computed once.                                                        
                        Additionally turns on no-update and no-recursion flags                                                     
                        i.e. -n -R                                                                                                 
  -f, --force           force building of packages ignoring NO_BUILD files                                                         
  -q, --quiet           do not show all builder output [default=True]
  -v, --verbose         show all builder output                                                                                                      
  -n, --no-update       no svn updates, rebuild of myself or Tools rebuild.                                                                          
                        Equivalent to "-S -M -T"                                                                                                     
  -N, --no-recursive-update                                                                                                                          
                        no svn updates, rebuild of myself, Tools rebuild or                                                                          
                        recursion. Equivalent to "-S -M -T -R -v" or                                                                                 
                        "python build.py TARGET"                                                                                                     
  -p EXTRAOPTS, --pass-options=EXTRAOPTS
                        pass on package specific build options, e.g. "nompi=1"                                                                         
                        or specific functional tests e.g "-t functest -p                                                                             
                        functests/mytest.py"                                                                                                         
  -t TARGET, --target=TARGET                                                                                                                         
                        select TARGET from: depends, install, stage, release,                                                                        
                        signature, bclean, clean, doc, format, pylint,
                        functest, test, deploy [default=install]
  --release-name=RELEASE_NAME
                        the name of the staging directory and the release
                        tarball
  --deploy-targets=DEPLOY_TARGETS
                        the deployement targets to execute. Select any
                        DEPLOY_TARGETS from:
                        authenticate,deploy_local,deploy_remote,verify [defaul
                        t=authenticate,deploy_local,deploy_remote,verify]

  Advanced Options:
    Caution: Use these options at your own risk.

    -D, --debug         do not run the actual package building command
    -M, --no-build-myself
                        do not rebuild myself (rbuild)
    -P, --no-parallel   do not do parallel builds of packages
    -R, --no-recursion  do not apply target recursively to dependencies
    -S, --no-svn-update
                        do not perform subversion update
    -T, --no-tools      do not rebuild Tools
    -U, --update-only   Ignore any target options and just do svn update
    -V, --no-virtualenv
                        do not include virtualenv in a release
    -X, --no-exit-on-error
                        continue building ignoring any individual package
                        build failures

------------------------------------------------------------------------------

rbuild system
=============
The rbuild system is the infrastructure underlying the rbuild command.
At the package level it can be run via the rbuild script or calling the
build.py files directly. In this case there is no recursive building.

For example:
    python build.py [-q|-x] <target>

It provides the following command-line options

-q        suppress messages to stdout and just print errors
-x        exit recursivebuild when an error is encountered

and targets as for rbuild.


Troubleshooting
===============

* Make sure that you don't have a ~/.pydistutils.cfg because this can
  conflict with virtualenv.
