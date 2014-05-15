#  lofar_qt.m4
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
#  $Id: lofar_qt.m4,v 1.6 2004/03/12 08:47:42 wierenga Exp $


# lofar_QT
#
# Macro to check for QT installation
#
# lofar_QT([OPTION,DEFAULT-PREFIX])
#
# e.g. lofar_QT(["/usr/local/qtr12"])
# -------------------------
#
AC_DEFUN([lofar_QT],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], [qt_lfr_option=0], [qt_lfr_option=$1])
ifelse($2, [], define(DEFAULT_QT_PREFIX,[/usr/lib/qt-2.3.1]), define(DEFAULT_QT_PREFIX,$2))
lofar_EXTERNAL(QT,[$qt_lfr_option],qt.h,qt-mt,DEFAULT_QT_PREFIX)
])
