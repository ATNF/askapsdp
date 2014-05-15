#  lofar_internal.m4
#
#  Copyright (C) 2004
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
#  $Id: lofar_internal.m4,v 1.2 2004/10/26 12:51:26 diepen Exp $


# lofar_INTERNAL
#
# Macro to check for an INTERNAL installed package
# It does the same as LOFAR_EXTERNAL, but gives a means to distinguish
# internal and external packages.
#
# lofar_INTERNAL(package-path, package, [cvs-versiontag],
#            [option], headerfile, [libraries], [searchpath].
#            [extra_cppflags],[extra_cxxflags],[extra_ldflags],[extra_libs])
#
# E.g.
#   lofar_INTERNAL(LCS/Common, Common, 1.3,, Common/LofarLogger.h)
#
#
AC_DEFUN([lofar_INTERNAL],dnl
[dnl
AC_PREREQ(2.13)dnl
lofar_EXTERNAL($2,$4,$5,$6,$7,$8,$9,$10,$11)
])
