#  lofar_dtl.m4
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
#  $Id: lofar_dtl.m4,v 1.2 2005/01/13 12:48:46 loose Exp $


# lofar_DTL
#
# Macro to check for DTL installation
#
# lofar_DTL(option)
#     option 0 means that DTL is optional, otherwise mandatory.
#
# e.g. lofar_DTL(1)
# -------------------------
#
AC_DEFUN([lofar_DTL],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], [lfr_dtl_option=0], [lfr_dtl_option=$1])
lofar_EXTERNAL(odbc,0,sql.h)
lofar_EXTERNAL(dtl,[$lfr_dtl_option],dtl/DTL.h,,,,"gnu:-Wno-unused-parameter gnunew:-Wno-unused-parameter")
])
