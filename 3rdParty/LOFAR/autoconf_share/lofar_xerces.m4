#  lofar_xerces.m4
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
#  $Id: lofar_xerces.m4,v 1.6 2004/03/12 08:47:42 wierenga Exp $


# lofar_XERCES
#
# Macro to check for XERCES (XML parser) installation
#
# lofar_XERCES([DEFAULT-PREFIX])
#
# e.g. lofar_XERCES(["/usr/local/xerces"])
#
# If uses the library name libxerces. If needed, one has to create a
# a symlink after installing xerces. E.g.
#    cd  xerces-c1_7_0-linux7.2/lib
#    ln -s libxerces-c1_7_0.so libxerces.so
# -------------------------
#
AC_DEFUN([lofar_XERCES],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], define(DEFAULT_XERCES_PREFIX,[/usr/local/xerces]), define(DEFAULT_XERCES_PREFIX,$1))
lofar_EXTERNAL(XERCES,0,xercesc/sax2,,DEFAULT_XERCES_PREFIX)
])
