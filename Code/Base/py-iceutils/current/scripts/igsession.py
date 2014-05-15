#!/usr/bin/env python

# Copyright (c) 2010 CSIRO
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
import os
import optparse
import time
from askap.iceutils.icegrid import IceGridSession
usage = "usage: %prog [options] <application xml files>"
desc = """This script starts up an icegrid instance and adds given applications
to it. It shuts down icegrid on Ctrl-C."""
parser = optparse.OptionParser(usage, description=desc)

parser.add_option('-d','--debug', dest='debug',
                  default=False, action='store_true',
                  help="Write debug aoutput for the applications in directory"
                       " 'debug_logs'.")

parser.add_option('-w', '--wait', dest='wait',
                  type="int",
                  default=None,
                  help="Add a delay between applications [default: no delay]")

parser.add_option('-c', '--config', dest='config',
                  default='config.icegrid',
                  type="string",
                  help="Name of the ice configuration file [default: %default]")


(opts, args) = parser.parse_args()

def main():
    if not os.path.exists(opts.config):
        raise IOError("Config file '%s' does not exist." % opts.config)
    igs = IceGridSession(opts.config)
    igs.debug = opts.debug
    igs.startup()
    print "Started IceGrid"
    for xml in args:
        igs.add_application(xml)
        if opts.wait:
            time.sleep(opts.wait)
    print "Running the following application(s): %s"\
        % " ".join(igs.get_application_names())
    print "Ctrl-C to quit."
    try:
        while True:
            time.sleep(10)
    except KeyboardInterrupt, ex:
        print "Shutting Down"
        igs.shutdown()

if __name__ == "__main__":
    main()
