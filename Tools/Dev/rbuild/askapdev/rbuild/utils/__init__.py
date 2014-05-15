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

'''
This contains a collection of utility functions.

The package __init__py file just imports functions from explicit files
(usually one per function) so users simply import from askapdev.rbuild.utils
package namespace.

'''

from run import run
from run_scons import run_scons
from tag_name import tag_name
from parse_error import parse_error
from copy_tree import copy_tree
from runcmd import runcmd
from rmtree import rmtree
from get_python_version import get_python_version
from create_python_tree import create_python_tree, get_site_dir
from q_print import q_print
from number_of_cpus import number_of_cpus
from format_src_code import format_src_code
from create_init_file import create_init_file
from which import which
from get_svn_revision import get_svn_revision
from get_svn_files_list import get_svn_files_list
from get_svn_branch_info import get_svn_branch_info
from get_release_version import get_release_version
from get_package_name import get_package_name
from get_platform import get_platform
from environment import Environment
from reposupdate import update_command, update_tree
from in_code_tree import in_code_tree
from in_dev_tree import in_dev_tree
