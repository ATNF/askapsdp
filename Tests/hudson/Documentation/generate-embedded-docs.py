#!/usr/bin/env python
'''
This program searches through ASKAPsoft for the various locally written
documentation and makes it available through the Redmine 'embedded' plugin.
The embedded plugin has a global root location which for ASKAP Redmine is
  phoenix:/var/www/vhosts/pm.atnf.csiro.au/embedded/askap
For each Redmine project its documentation is in the relative location
  <project id>/html
e.g. for Computing IPT
  cmpt/html 
Below this, the name of the directory specifies the single stylesheet and
single javascript file that is applied to that directory and all its sub
directories.  These have to be configured in Redmine.

The ASKApsoft documentation is produced in Code by running
  python autobuild.py -t doc

To Do:
1. build and install documentation in Tools/Dev
2. make the individual packages form a unified view.

'''

import datetime
import os
import sys

DEBUG = 0

CODE     = '${WORKSPACE}/trunk/Code'
TARGET   = '/var/www/vhosts/pm.atnf.csiro.au/embedded/askap/cmpt/html'
RHOST    = 'phoenix'
TAB      = ' '*4
PRE_SSH  = 'eval `ssh-agent` && ssh-add ${HOME}/.ssh/id_dsa-nokeys'
POST_SSH = 'kill ${SSH_AGENT_PID}'

INDEX_HEAD='''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
        "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">

  <head>
    <title>Welcome to pm.atnf.CSIRO.AU</title>
  </head>

  <body>
    <h2> CMPT %s Documents </h2>
    <p>
    The documentation in this area is %s auto-generated.
    </p>

    <h3> Packages </h3>
'''

INDEX_TAIL='''
    <p>
    <i>Last modified: %s.</i>
    <br/>
    &copy; Copyright CSIRO Australia %s.
    <a href="http://www.csiro.au/index.asp?type=aboutCSIRO&amp;xml=mrcopyright&amp;style=generic">CSIRO Media Release information</a>,
    <a href="http://www.csiro.au/index.asp?type=aboutCSIRO&amp;xml=disclaimer&amp;style=generic">Legal Notice and Disclaimer</a>,
    <a href="http://www.csiro.au/index.asp?type=aboutCSIRO&amp;xml=privacy&amp;stylesheet=generic">Privacy Statement</a>.
    </p>

  </body>
</html>
'''


def BuildTree(path, partial_paths, old_indent):
    '''
    Given a package directory path, return indented HTML list items of
    the path.
    Need to keep track of partial paths already seen.
    '''

    path_components = path.split('/')
    p = ''
    index = ''

    for comp in path_components[0:-1]:
        p += comp + '/'
        if not p in partial_paths:
            partial_paths.add(p)
            indent = len(p.split('/'))
            if indent < old_indent:
                index += TAB*old_indent + "</ul>\n"
            index += TAB*indent + "<li> %s </li>\n" % comp
            index += TAB*(indent+1) + "<ul>\n"
    return index


def CopyDocs(doc_type, doc_loc, find_term):
    '''
    The ASKAPsoft system uses different type of documentation depending on the
    language used. e.g. java -> javadocs, python -> sphinx, C++ -> doxygen
    This is the "doc_type" parameter.
    The documentation is produced in different locations depending
    on the type.  This is the "doc_loc" parameter.
    To double check we are getting the correct documentation (and avoid any
    third party documentation) search for a particular file. This is the
    "find_term" parameter.

    This function does two things.
    1. Creates a document type specific index file that is copied to the
       remote machine in the top level document type directory.
       e.g. cmpt/html/[doxygen|javadocs|sphinx]
       This contains an unordered list <ul> which points to the
       subdirectories containing documentation.
    2. Copies the ASKAPsoft documentation to the remote machine rooted in the
       top level document type directory, keeping the relative path from the
       build system down to the package level i.e. the 'doc_loc' is stripped
       from the end of the path.
    '''

    indent = 0
    partial_paths = set()
    index = INDEX_HEAD % (doc_type.capitalize(), doc_type)
    index += TAB*2 + "<ul>\n" # Start indented under '<h3> Packages </h3>'

    # PKGS is a file like object which contains a list of paths to the
    # package level which should contain documentation of specified type. e.g.
    # /tmp/ASKAPsoft/Code/Base/py-loghandlers
    # /tmp/ASKAPsoft/Code/Base/py-parset

    sed_term = "/%s/%s" % (doc_loc, find_term)
    cmd = 'find %s -name %s | sed "s#%s##"' % (CODE, find_term, sed_term)
    if DEBUG:
        print '     ', cmd
    PKGS = os.popen(cmd)

    for pkgpath in PKGS:
        pkgpath = pkgpath.strip() # Needed for next line to work correctly.
        pkg = pkgpath.split('/')[-1] # Last bit is package name.
        subtree = pkgpath.split('/Code/')[-1] # Only stuff to right of Code.
        source = "%s/%s/" % (pkgpath, doc_loc) # Recreate the full path.
        target = "%s/%s/%s" % (TARGET, doc_type, subtree)
        print "info: %s" % pkg

        if os.path.isdir(source): # It should be a directory.
            old_indent = indent
            index += BuildTree(subtree, partial_paths, old_indent)
            indent = len(subtree.split('/')) + 1
            if old_indent > indent: # have moved back up one or more levels.
                index += TAB*old_indent + "</ul>\n"
            index += TAB*indent + \
                    '<li><a href="%s"> %s </a></li>\n' % (subtree, pkg)
            if DEBUG:
                print "      src = %s" % source
                print "      dst = %s" % target
            else:
                os.system("%s && ssh %s mkdir -p %s; %s" %
                          (PRE_SSH, RHOST, target, POST_SSH))
                os.system("%s && rsync -av --delete %s %s:%s; %s" %
                          (PRE_SSH, source, RHOST, target, POST_SSH))

    # Finish off all outstanding unordered lists <ul>.
    # The number outstanding can be determined from the current indent level
    # remember the default indent level for the list is 2.
    for i in reversed(range(2, indent+1)):
        index += TAB*i + "</ul>\n"

    # Add the tail to the index file and write out so that it can
    # be scp to remote destination.
    index += INDEX_TAIL % (today.strftime("%d %b %Y"), today.year)
    f = open('/tmp/index.html', 'w')
    f.write(index)
    f.close()
    if DEBUG:
        print '\n\n', index
    else:
        os.system("%s && scp /tmp/index.html %s:%s/%s; %s" %
                  (PRE_SSH, RHOST, TARGET, doc_type, POST_SSH))


#
# Main Program
#

today = datetime.date.today()

print "info: Doxygen"
CopyDocs("doxygen", "current/html", "doxygen.css")
print "info: Javadocs"
CopyDocs("javadocs", "current/doc", "stylesheet.css")
print "info: Sphinx"
CopyDocs("sphinx", "current/doc/_build/html", "objects.inv")
print "info: Ice"
CopyDocs("ice", "current/slice/doc", "_sindex.html")
