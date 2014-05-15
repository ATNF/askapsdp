#  lofar_python.m4
#
#  Copyright (C) 2002
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#  $Id: lofar_python.m4,v 1.3 2006/06/08 07:15:10 diepen Exp $


# lofar_PYTHON
#
# Macro to check for PYTHON installation
#
# lofar_PYTHON(option)
#     option 0 means that PYTHON is optional, otherwise mandatory.
#
# e.g. lofar_PYTHON(1)
# -------------------------
#
AC_DEFUN([lofar_PYTHON],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], [lfr_python_option=0], [lfr_python_option=$1])
lofar_EXTERNAL(python,[$lfr_python_option],Python.h,"python+vers","/usr/include/python+vers /usr/local/include/python+vers",,,,'-ldl -lutil')
])
