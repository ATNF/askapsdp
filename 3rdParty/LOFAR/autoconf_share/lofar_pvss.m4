#  lofar_pvss.m4
#
#  Copyright (C) 2003
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
#  $Id: lofar_pvss.m4,v 1.2 2002/12/16 14:25:06 diepen Exp $


# lofar_PVSS
#
# Macro to check for PVSS installation
#
# lofar_PVSS(option, [DEFAULT-PREFIX])
#     option 0 means that PVSS is optional, otherwise mandatory.
#
# e.g. lofar_PVSS(1)
# --------------------
#
AC_DEFUN([lofar_PVSS],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], [lfr_option=0], [lfr_option=$1])
AC_ARG_WITH(pvss,
	[  --with-pvss[=PFX]     prefix where pvss libraries are installed],
	[with_pvss=$withval],
	[with_pvss=""])
[
enable_pvss=no
if test "$lfr_option" = "1"; then
  enable_pvss=yes
fi
if test "$with_pvss" != "no"; then
  if test "$with_pvss" = ""; then
    pvss_prefix=
  else
    if test "$with_pvss" = "yes"; then
      pvss_prefix=
    else
      pvss_prefix=$with_pvss
    fi
    enable_pvss=yes
  fi
##
## Look for library in suggested locations or in its lib subdir
##
  pvss_inclist=$pvss_prefix;
  if test "$pvss_prefix" = ""; then
    pvss_inclist="/opt/pvss/pvss2_v3.0/api/include"
  fi
  for bdir in $pvss_inclist
  do
    ]AC_CHECK_FILE([$bdir/Basics/Utilities/Util.hxx],
			[lfr_lib_pvssd=$bdir/../lib.linux_RH90],
			[lfr_lib_pvssd=no])[
  done

  INCL_ROOT=$pvss_inclist
  PVSS_CPPFLAGS="-D__UNIX -D__PC -DHAS_TEMPLATES=1 -DBC_DESTRUCTOR -Dbcm_boolean_h -DOS_LINUX -DLINUX -DLINUX2 -DDLLEXP_BASICS= -DDLLEXP_CONFIGS= -DDLLEXP_DATAPOINT= -DDLLEXP_MESSAGES= -DDLLEXP_MANAGER= -DDLLEXP_CTRL= -I${INCL_ROOT}/Basics/Variables -I${INCL_ROOT}/Basics/Utilities -I${INCL_ROOT}/Basics/NoPosix -I${INCL_ROOT}/BCMNew -I${INCL_ROOT}/Configs -I${INCL_ROOT}/Datapoint -I${INCL_ROOT}/Messages -I${INCL_ROOT}/Manager"
  PVSS_CXXFLAGS="-fmessage-length=0"
  PVSS_VERSION=V30_876_RH90
  if test "$lfr_lib_pvssd" != "no"; then
    ]AC_CHECK_FILE([$lfr_lib_pvssd/libBasics$PVSS_VERSION.so],
			[lfr_lib_pvss=$lfr_lib_pvssd],
			[lfr_lib_pvss=no])[
  fi

  if test "$lfr_lib_pvss" != "no" ; then
    PVSS_LDFLAGS="-L$lfr_lib_pvss"
		PVSS_OBJS="$lfr_lib_pvss/DpConfig.o $lfr_lib_pvss/DpConfigManager.o"
    PVSS_LIBS="$PVSS_OBJS -lManager$PVSS_VERSION -lMessages$PVSS_VERSION -lDatapoint$PVSS_VERSION -lBasics$PVSS_VERSION -lbcm$PVSS_VERSION -ldl -lwklin -lPVSSUtil_RH90"

    echo "PVSS" >> pkgext
    echo "$PVSS_CPPFLAGS" >> pkgextcppflags
    echo "$PVSS_CXXFLAGS" >> pkgextcxxflags
    echo "$PVSS_OBJS" >> pkgextobjs

    CPPFLAGS="$CPPFLAGS $PVSS_CPPFLAGS"
    CXXFLAGS="$CXXFLAGS $PVSS_CXXFLAGS"
    LDFLAGS="$LDFLAGS $PVSS_LDFLAGS"
    LIBS="$LIBS $PVSS_LIBS"
]
dnl
    AC_SUBST(CPPFLAGS)dnl
    AC_SUBST(CXXFLAGS)dnl
    AC_SUBST(LDFLAGS)dnl
    AC_SUBST(LIBS)dnl
dnl
    AC_DEFINE(HAVE_PVSS, 1, [Define if pvss is installed])dnl
[
  else
    if test "$enable_pvss" = "yes" ; then
]
      AC_MSG_ERROR([Could not find pvss library in $pvss_prefix])
[
    fi
    enable_pvss=no
  fi
fi]
AM_CONDITIONAL(HAVE_PVSS, [test "$enable_pvss" = "yes"])
])


