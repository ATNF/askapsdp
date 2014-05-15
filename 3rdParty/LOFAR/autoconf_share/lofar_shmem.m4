#  lofar_shmem.m4
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
#  $Id: lofar_shmem.m4,v 1.1 2005/01/06 17:02:30 loose Exp $


# lofar_SHMEM
#
# Macro to check for Shared Memory support.
#
AC_DEFUN([lofar_SHMEM],dnl
[dnl
AC_PREREQ(2.13)dnl
have_shmem=0;
[
if test "$with_shmem" = "yes"; then
  have_shmem=1;
  ]AC_DEFINE(HAVE_SHMEM, 1, [Defined if shared memory is used])[
  echo SHMEM >> pkgext
fi
]
AM_CONDITIONAL(HAVE_SHMEM, test $have_shmem = 1)
])dnl
dnl
