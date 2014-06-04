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
import time
import sys
import subprocess

import SCons.Builder
import SCons.Action

def run_tests(target, source, env):
    reports = {"errors": [], "failures": []}
    startt = time.time()
    n = 0
    if len(source) == 0:
        return
    for test in source:
        ptest = str(test)
        p = subprocess.Popen(ptest, shell=True, 
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE, env=env["ENV"])
        retcode = p.wait()
        errtype = None
        ptest = os.path.basename(ptest)
        if retcode != 0:
            out, err = p.communicate()
            err = err.strip()
            if err.startswith("/bin/sh"):
                sys.stdout.write("E")
                reports["errors"].append(("ERROR", ptest, err))
            else:
                sys.stdout.write("F")
                reports["failures"].append(("FAILED", ptest, err))
        else:
            sys.stdout.write(".")
        n += 1

    print
    dtime = time.time()-startt
    status = None
    if len(reports["errors"]) > 0 or len(reports["failures"]) > 0:
        errstrings = []
        for key, value in reports.items():     
            if len(value):
                errstrings.append("%s=%d" % (key, len(value)))
            for typ, prog, err in value:
                print "="*70
                print "%s: %s" % (typ, prog)
                print "-"*70
                print err
                print
        status = "FAILED (%s)" % ",".join(errstrings)
    else:
        status = "OK"
    print "-"*70
    print "Ran %d tests in %0.3fs\n" % (n, dtime)
    print status
    return 0

def runtest_emitter(target, source, env):
    # create dummy target
    return (str(source)+"tmp", source)

def runtest_str(target, source, env):
    return ""


functest_builder = \
    SCons.Builder.Builder(action=SCons.Action.Action(run_tests, 
                                                     runtest_str),
                          emitter=runtest_emitter)

def generate(env):
    env["BUILDERS"]["FuncTest"] = functest_builder
    

def exists(env):
    return True
