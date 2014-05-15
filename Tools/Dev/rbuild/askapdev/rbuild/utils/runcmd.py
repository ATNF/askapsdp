## Package for various utility functions to execute build and shell commands
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
# @author Malte Marquarding <Malte.Marquarding@csiro.au>
#
import select
import subprocess
import sys

def runcmd(cmd, executable=None, shell=False, realtime=False):
    '''Run a command and get back the results as a tuple containing
    stdout, stderr and the return status code.
    This function uses Popen with pipes and so the output is buffered in
    memory.  Therefore do not use where the expected output is very large.
    Normally would run Popen with argument:
      env={"PATH": "/usr/bin:/bin:/usr/sbin:/sbin"}
    but as we are running in the ASKAPsoft build environment,  want to
    inherit the path.
  
    :param cmd: the command to be run.  This may be a string or a sequence
                of strings that form the command.
                If the command contains shell metacharacters e.g. want to do
                redirection or pipes then it must be a string and the shell
                parameter must be set to True.
    :param executable: allow specification of particular shell 
                       amongst other things. (default None)
    :param shell: boolean controlling whether to spawn a shell (default False)
    :param realtime: want output from command in realtime and not buffered
                     until command finished.

    .. note::

    In Popen() when shell=False the command must be a sequence.
    Being pedantic, we can accept a string if it is just the executable
    with no arguments.
    So to be safe always convert a string to a sequence unless we are
    using a shell.
'''
    if type(cmd) == str and not shell:
        cmd = cmd.split()
    proc = subprocess.Popen(cmd, executable=executable, shell=shell,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    if realtime:
        stdout = []
        stderr = []

        while True:
            reads = [proc.stdout.fileno(), proc.stderr.fileno()]
            ret = select.select(reads, [], [])

            for fd in ret[0]:
                if fd == proc.stdout.fileno():
                    read = proc.stdout.readline()
                    sys.stdout.write(read)
                    stdout.append(read)
                if fd == proc.stderr.fileno():
                    read = proc.stderr.readline()
                    sys.stderr.write(read)
                    stderr.append(read)

            if proc.poll() != None:
                break

        stdout = ''.join(stdout)
        stderr = ''.join(stderr)
    else:
        stdout, stderr = proc.communicate()
 
    return (stdout, stderr, proc.returncode)
