#
'''
Common functions for the create packages scripts.
'''

import pkg_resources

import os
import re
import subprocess
import sys

import askapdev.rbuild.utils as utils

utests_dir = "tests"
ftests_dir = "functests"
test_dirs  = [utests_dir, ftests_dir]

askap_root = os.getenv("ASKAP_ROOT")

if askap_root == None:
    print "Error: environment variable ASKAP_ROOT does not exist."
    sys.exit(1)


def add_to_svn(dirpath):
    (stdout, stderr, status) = utils.runcmd("svn add --no-ignore %s" % dirpath)
    if status == 0:
        print stdout.strip()
        print "info: templates add to repository. You will need to do a commit."
        return True
    else:
        return False


def set_svn_ignores(dirpath, ignorelist):
    '''Add a list of file name patterns to ignore in the given directory.
    '''
    # Repeated calls to propset overwrite previous values, thus to set
    # multiple patterns need to given them in a single incanation.
    # Slight difficulty is that propset values must be separated by a new line.
    cmd = "svn propset --file /dev/stdin svn:ignore %s" % dirpath
    ignorestr = "\n".join(ignorelist)
    proc = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    stdout = proc.communicate(input=ignorestr)[0]
    proc.wait()
    print "info:", stdout.strip(), ignorelist


def write_file(template_fn, out_dir, out_fn):
    data = pkg_resources.resource_string(__name__, template_fn)
    outfile = os.path.join(out_dir, out_fn)
    fhandle = open(outfile, 'w')
    fhandle.write(data)
    fhandle.close()


def update_template_files(fullpath, options, filenames):
    '''If options are supplied, change the template SConstruct file to use
    the supplied values.  Also modify the package.info file to fill
    in component and package names.
    '''
    for filename in filenames:
        full_fn = os.path.join(fullpath, filename)
        fhandle = open(full_fn, 'r+')
        data = fhandle.read()
        for key, value in options.items():
            if value is not None:
                if key == 'keywords': # which is a multi-value item
                    value = ', '.join(["'%s'" % item for item in value.split()])
                elif key == 'ice': # value is True/False
                    if value:
                        value = 'interfaces=Code/Interfaces/slice/current'
                    else:
                        value = ''
                data = re.sub('@@@%s@@@' % key, value, data)
        fhandle.seek(0)
        fhandle.truncate()
        fhandle.write(data)
        fhandle.close()


def create_test_dirs(fullpath):
    for testdir in test_dirs:
        os.makedirs(os.path.join(fullpath, testdir), 0755)


def get_func_test_dir(fullpath):
    return os.path.join(fullpath, ftests_dir)
