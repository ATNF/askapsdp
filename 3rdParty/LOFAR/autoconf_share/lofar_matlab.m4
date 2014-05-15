#  lofar_matlab.m4
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
#  $Id: lofar_matlab.m4,v 1.7 2004/03/12 08:47:42 wierenga Exp $


# lofar_MATLAB
#
# Macro to check for MATLAB installation
#
# lofar_MATLAB([DEFAULT-PREFIX])
#
# e.g. lofar_MATLAB(["/usr/local/matlabr12"])
# -------------------------
#
AC_DEFUN([lofar_MATLAB],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], define(DEFAULT_MATLAB_PREFIX,[/usr/local/matlabr12]), define(DEFAULT_MATLAB_PREFIX,$1))
AC_ARG_WITH(matlab,
	[  --with-matlab[=PFX]     prefix where Matlab is installed (default=]DEFAULT_MATLAB_PREFIX[)],
	[with_matlab=$withval],
	[with_matlab="no"])
[
if test "$with_matlab" = "no"; then
  enable_matlab=no
else
  if test "$with_matlab" = "yes"; then
    matlab_prefix=]DEFAULT_MATLAB_PREFIX
[
  else
    matlab_prefix=$withval
  fi
  enable_matlab=yes
]
##
## Check in normal location and suggested location
##
AC_CHECK_FILE([$matlab_prefix/extern/include/matlab.h],
			[lofar_cv_header_matlab=yes],
			[lofar_cv_header_matlab=no])dnl
[
  if test $lofar_cv_header_matlab = yes ; then

	MATLAB_PATH="$matlab_prefix"

	MATLAB_CPPFLAGS="-I$MATLAB_PATH/extern/include"
	MATLAB_LDFLAGS="-L$MATLAB_PATH/extern/lib/glnx86"
	MATLAB_LIBS="-lmx -leng"

	echo MATLAB >> pkgext
	echo "$MATLAB_CPPFLAGS" >> pkgextcppflags

	CPPFLAGS="$CPPFLAGS $MATLAB_CPPFLAGS"
	LDFLAGS="$LDFLAGS $MATLAB_LDFLAGS"
	LIBS="$LIBS $MATLAB_LIBS"
]
dnl
AC_SUBST(CPPFLAGS)dnl
AC_SUBST(LDFLAGS)dnl
AC_SUBST(LIBS)dnl
dnl
AC_DEFINE(HAVE_MATLAB, 1, [Define if Matlab is installed])dnl
[
  else]
AC_MSG_ERROR([Could not find Matlab in $matlab_prefix])
[
    enable_matlab=no
  fi
fi]
AM_CONDITIONAL(HAVE_MATLAB, [test "$enable_matlab" = "yes"])
])
