#  lofar_corba.m4
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
#  $Id: lofar_corba.m4,v 1.10 2004/03/12 08:47:42 wierenga Exp $


# lofar_CORBA
#
# Macro to check for suitable Corba implementation
#
AC_DEFUN([lofar_CORBA],dnl
[dnl
AC_PREREQ(2.13)dnl
lofar_HEADER_VISIBROKER([])
lofar_HEADER_TAO([])
[
if test "$enable_vbroker" = "yes"; then
  if test "$enable_tao" = "yes"; then
   ]AC_MSG_ERROR([Can not have both Visibroker and TAO used at the same time])[
  fi
fi
have_corba=0;
if test "$enable_vbroker" = "yes"; then
  have_corba=1;
  ]AC_DEFINE(HAVE_CORBA, 1, [Defined if Corba is used])[
  ]AC_DEFINE(HAVE_VBROKER, 1, [Defined if Visibroker is used])[
  echo CORBA >> pkgext
  echo VBROKER >> pkgext
fi
if test "$enable_tao" = "yes"; then
  have_corba=1;
  ]AC_DEFINE(HAVE_CORBA, 1, [Defined if Corba is used])[
  ]AC_DEFINE(HAVE_TAO, 1, [Defined if TAO is used])[
  echo CORBA >> pkgext
  echo TAO >> pkgext
fi
]
AM_CONDITIONAL(HAVE_CORBA, test $have_corba = 1)
])dnl
dnl
#
# lofar_HEADER_VISIBROKER([DEFAULT-PREFIX])
#
# e.g. lofar_HEADER_VISIBROKER("/var/local/inprise/vbroker")
# -------------------------
#
# Macro to check corba.h header file
#
AC_DEFUN([lofar_HEADER_VISIBROKER],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], define(DEFAULT_VBROKER_PREFIX,[/opt/inprise/vbroker]), define(DEFAULT_VBROKER_PREFIX,$1))
AC_ARG_WITH(vbroker,
	[  --with-vbroker[=PFX]    prefix where Visibroker is installed (default=]DEFAULT_VBROKER_PREFIX[)],
	[with_vbroker=$withval],
	[with_vbroker="no"])
[
if test "$with_vbroker" = "no"; then
  enable_vbroker=no
else
  if test "$with_vbroker" = "yes"; then
    vbroker_prefix=]DEFAULT_VBROKER_PREFIX
[
  else
    vbroker_prefix=$withval
  fi
  enable_vbroker=yes
]
##
## Check in normal location and suggested location
##
AC_CHECK_FILE([$vbroker_prefix/include/corba.h],
			[lofar_cv_header_vbroker=yes],
			[lofar_cv_header_vbroker=no])
dnl
[
  if test $lofar_cv_header_vbroker = no; then
     vbroker_prefix="/usr/local/inprise/vbroker"]
##
## Check in alternative location
##
AC_CHECK_FILE([$vbroker_prefix/include/corba.h],
			[lofar_cv_header_vbroker=yes],
			[lofar_cv_header_vbroker=no])
[
  fi
  if test $lofar_cv_header_vbroker = yes ; then

    VBROKER_PATH="$vbroker_prefix"

    VBROKER_CPPFLAGS="-I$VBROKER_PATH/include"
    VBROKER_CXXFLAGS="-Wno-reorder -Wno-switch -Wno-unused"
    VBROKER_LDFLAGS="-L$VBROKER_PATH/lib"
    VBROKER_LIBS="-lcosev_r -lcosnm_r -lvport_r -lorb_r -lpthread"

    echo "$VBROKER_CPPFLAGS" >> pkgextcppflags
    echo "$VBROKER_CXXFLAGS" >> pkgextcxxflags

    CPPFLAGS="$CPPFLAGS $VBROKER_CPPFLAGS"
    CXXFLAGS="$CXXFLAGS $VBROKER_CXXFLAGS"
    LDFLAGS="$LDFLAGS $VBROKER_LDFLAGS"
    LIBS="$LIBS $VBROKER_LIBS"
    IDLCXX="$vbroker_prefix/bin/idl2cpp"
    IDLFLAGS=""
]
dnl
AC_SUBST(CPPFLAGS)dnl
AC_SUBST(CXXFLAGS)dnl
AC_SUBST(LDFLAGS)dnl
AC_SUBST(LIBS)dnl
AC_SUBST(IDLCXX)dnl
AC_SUBST(IDLFLAGS)dnl
dnl
[
  else]
AC_MSG_ERROR([Could not find Inprise Visibroker in $vbroker_prefix])
[
    enable_vbroker=no
  fi
fi]
])
#
# lofar_HEADER_TAO([DEFAULT-PREFIX])
#
# e.g. lofar_HEADER_TAO("/usr/local/ACE")
# -------------------------
#
# Macro to check corba.h header file
#
AC_DEFUN([lofar_HEADER_TAO],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], define(DEFAULT_TAO_PREFIX,[/usr/local/ACE]), define(DEFAULT_TAO_PREFIX,$1))
AC_ARG_WITH(tao,
	[  --with-tao[=PFX]        prefix where TAO is installed (default=]DEFAULT_TAO_PREFIX[)],
	[with_tao=$withval],
	[with_tao="no"])
[
if test "$with_tao" = "no"; then
  enable_tao=no
else
  if test "$with_tao" = "yes"; then
    tao_prefix=]DEFAULT_TAO_PREFIX
[
  else
    tao_prefix=$withval
  fi
  enable_tao=yes
]
##
## Check in normal location and suggested location
##
AC_CHECK_FILE([$tao_prefix/TAO/tao/corba.h],
			[lofar_cv_header_tao=yes],
			[lofar_cv_header_tao=no])
dnl
[
  if test $lofar_cv_header_tao = no; then
     tao_prefix="/usr/local/ACE"]
##
## Check in alternative location
##
AC_CHECK_FILE([$tao_prefix/TAO/tao/corba.h],
			[lofar_cv_header_tao=yes],
			[lofar_cv_header_tao=no])
[
  fi
  if test $lofar_cv_header_tao = yes ; then

    TAO_PATH="$tao_prefix"

    TAO_CPPFLAGS="-I$TAO_PATH -I$TAO_PATH/TAO -I$TAO_PATH/TAO/tao"
    TAO_LDFLAGS="-L$TAO_PATH/lib"
    TAO_LIBS=""

    echo "$TAO_CPPFLAGS" >> pkgextcppflags

    CPPFLAGS="$CPPFLAGS $TAO_CFLAGS"
    LDFLAGS="$LDFLAGS $TAO_LDFLAGS"
    LIBS="$LIBS $TAO_LIBS"
    IDLCXX="$tao_prefix/TAO/TAO_IDL/tao_idl"
    IDLFLAGS=""
]
dnl
AC_SUBST(CFLAGS)dnl
AC_SUBST(CPPFLAGS)dnl
AC_SUBST(LDFLAGS)dnl
AC_SUBST(LIBS)dnl
AC_SUBST(IDLCXX)dnl
AC_SUBST(IDLFLAGS)dnl
dnl
[
  else]
AC_MSG_ERROR([Could not find ACE TAO in $tao_prefix])
[
    enable_tao=no
  fi
fi]
])
