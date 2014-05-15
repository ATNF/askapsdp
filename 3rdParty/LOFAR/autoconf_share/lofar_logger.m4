#  lofar_logger.m4
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
#  $Id: lofar_logger.m4,v 1.1 2004/10/29 09:38:19 loose Exp $


# lofar_LOG4CPLUS
#
# Macro to check for LOG4CPLUS installation
#
# lofar_LOG4CPLUS(option)
#     option 0 means that Log4cplus++ is optional, otherwise mandatory.
#
# e.g. lofar_LOG4CPLUS(1)
# -------------------------
#
AC_DEFUN([lofar_LOG4CPLUS],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], [lfr_option=0], [lfr_option=$1])
lofar_EXTERNAL(log4cplus,[$lfr_option],log4cplus/logger.h,,)
])
