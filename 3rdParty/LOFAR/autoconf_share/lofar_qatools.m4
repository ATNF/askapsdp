#  lofar_checktools.m4
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
#  $Id: lofar_qatools.m4,v 1.7 2005/05/03 13:04:48 lofar Exp $


# lofar_QATOOLS
#
# Macro to check for pure or insure checking tools
#
AC_DEFUN([lofar_QATOOLS],dnl
lofar_PURETOOLS([])dnl
lofar_INSURETOOLS([])dnl
lofar_COMPILETOOLS()dnl
[
if test "$enable_puretools" = "yes"; then
  if test "$enable_insuretools" = "yes"; then
]
    AC_MSG_ERROR([Cannot use both pure and insure tools. Reconfigure with --without-puretools or --without-insuretools])
[ fi
fi]
)dnl
#
#
#
# lofar_PURETOOLS
#
# Macro to check for and enable compilation/link with
# purify, quantify and purecov
#
# e.g. lofar_PURETOOLS(["/opt/pure/purify"])
# -------------------------
#
AC_DEFUN([lofar_PURETOOLS],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], define(DEFAULT_PURETOOLS_PREFIX,[/opt/pure]), define(DEFAULT_PURETOOLS_PREFIX,$1))
AC_ARG_WITH(puretools,
	[  --with-puretools[=PFX]  prefix where Rational Purify, Quantify or Pure Coverage is installed (default=]DEFAULT_PURETOOLS_PREFIX[)],
	[force_puretools=yes; with_puretools="$withval"],
	[force_puretools=no;  with_puretools="yes"])dnl
[
if test "$with_puretools" = "no"; then
  enable_puretools=no
else
  enable_puretools=yes
  if test "$with_puretools" = "yes"; then
    puretools_prefix=]DEFAULT_PURETOOLS_PREFIX
  [
  else
    puretools_prefix=$withval
  fi
  ]
  #
  # Check for purify, quantify and purecov
  #
  AC_MSG_CHECKING([for purify, quantify and purecov in $puretools_prefix])
  AC_MSG_RESULT([])
  AC_CHECK_FILE([$puretools_prefix/purify/purify.h],
		[lofar_cv_purify_lib=yes],
		[lofar_cv_purify_lib=no])
  AC_CHECK_FILE([$puretools_prefix/purify/libpurify_stubs.a],
		[lofar_cv_purify_lib="$lofar_cv_purify_lib yes"],
		[lofar_cv_purify_lib="$lofar_cv_purify_lib no"])
  AC_CHECK_FILE([$puretools_prefix/purify/purify],
		[lofar_cv_purify=yes],
		[lofar_cv_purify=no])
  AC_CHECK_FILE([$puretools_prefix/quantify/quantify],
		[lofar_cv_quantify=yes],
		[lofar_cv_quantify=no])
  AC_CHECK_FILE([$puretools_prefix/purecov/purecov],
		[lofar_cv_purecov=yes],
		[lofar_cv_purecov=no])
  dnl
  [
  have_puretools=no;
  have_purify=0;
  have_quantify=0;
  have_purecov=0;
  if test $lofar_cv_purify = yes ; then
    have_purify=1;
    have_puretools=yes;
    if test "$lofar_cv_purify_lib" = "yes yes"; then ]
      AC_DEFINE(HAVE_PURIFY, 1, [Define if using Rational Purify])dnl
      [	CPPFLAGS="$CPPFLAGS -I$puretools_prefix/purify"
	LDFLAGS="$LDFLAGS -L$puretools_prefix/purify"
	LIBS="$LIBS -lpurify_stubs"
    fi
  fi
  if test $lofar_cv_quantify = yes ; then
    have_quantify=1;
    have_puretools=yes;
  fi
  if test $lofar_cv_purecov = yes ; then
    have_purecov=1;
    have_puretools=yes;
  fi

  if test $have_puretools = yes ; then

    PURETOOLS_PATH="$puretools_prefix"

    # Get absolute top_srcdir for puretool
    lofar_srcdir=`cd $srcdir && pwd`
    lofar_base=`echo $lofar_srcdir | sed -e "s%/LOFAR/.*%/LOFAR%"`
    lofar_puredir=$lofar_base/autoconf_share

    CC="$lofar_puredir/puretool $have_purify $have_quantify $have_purecov "'$(top_builddir)'" $puretools_prefix $CC"
    CXX="$lofar_puredir/puretool $have_purify $have_quantify $have_purecov "'$(top_builddir)'" $puretools_prefix $CXX"
    ]
    dnl
    AC_SUBST(CPPFLAGS)dnl
    AC_SUBST(LDFLAGS)dnl
    AC_SUBST(LIBS)dnl
    AC_SUBST(CC)dnl
    AC_SUBST(CXX)dnl
    dnl
    [
  else
    if test "$force_puretools" = "yes"; then]
      AC_MSG_ERROR([Could not find Rational Purify, Quantify or Pure Coverage in $puretools_prefix])
      [
    fi
    enable_puretools=no
  fi
fi]
])
#
#
#
# lofar_INSURETOOLS
#
# Macro to check for and enable compilation/link with insure++
#
# e.g. lofar_INSURETOOLS(["/opt/insure/bin"])
# -------------------------
#
AC_DEFUN([lofar_INSURETOOLS],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], define(DEFAULT_INSURETOOLS_PREFIX,[/usr/local/parasoft/bin.linux2]), define(DEFAULT_INSURETOOLS_PREFIX,$1))
AC_ARG_WITH(insuretools,
	[  --with-insuretools[=PFX]  prefix where Parasoft insure tools are installed (default=]DEFAULT_INSURETOOLS_PREFIX[)],
	[force_insuretools="yes"; with_insuretools="$withval"],
	[force_insuretools="no";  with_insuretools="yes"])dnl
[
if test "$with_insuretools" = "no"; then
  enable_insuretools=no
else
  enable_insuretools=yes
  if test "$with_insuretools" = "yes"; then
    insuretools_prefix=]DEFAULT_INSURETOOLS_PREFIX
[
  else
    insuretools_prefix=$with_insuretools
  fi
]
#
# Check for insure, inuse, tca, and Chaperon
#
AC_MSG_CHECKING([for insure, inuse, tca, and Chaperon in $insuretools_prefix])
AC_MSG_RESULT([])
AC_CHECK_FILE([$insuretools_prefix/insure],
		[lofar_cv_insure=yes],
		[lofar_cv_insure=no])
AC_CHECK_FILE([$insuretools_prefix/inuse],
		[lofar_cv_inuse=yes],
		[lofar_cv_inuse=no])
AC_CHECK_FILE([$insuretools_prefix/tca],
		[lofar_cv_tca=yes],
		[lofar_cv_tca=no])
AC_CHECK_FILE([$insuretools_prefix/Chaperon],
		[lofar_cv_Chaperon=yes],
		[lofar_cv_Chaperon=no])
dnl
[
  have_insurepp=no;
  have_insure=0;
  have_inuse=0;
  have_tca=0;
  have_Chaperon=0;
  if test $lofar_cv_insure = yes ; then
    have_insure=1;
    have_insurepp=yes;
  fi
  if test $lofar_cv_inuse = yes ; then
    have_inuse=1;
    have_insurepp=yes;
  fi
  if test $lofar_cv_tca = yes ; then
    have_tca=1;
    have_insurepp=yes;
  fi
  if test $lofar_cv_Chaperon = yes ; then
    have_Chaperon=1;
    have_insurepp=yes;
  fi

  if test $have_insurepp = yes ; then

    INSURETOOLS_PATH="$insuretools_prefix"

    # Get absolute top_srcdir for insuretool
    lofar_curwd=`pwd`
    lofar_srcdir=`cd $srcdir && pwd`
    lofar_base=`echo $lofar_srcdir | sed -e "s%/LOFAR/.*%/LOFAR%"`
    lofar_insuredir=$lofar_base/autoconf_share

    CC="$lofar_insuredir/insuretool $have_insure $have_inuse $have_tca "'$(top_builddir)'" $insuretools_prefix $CC"
    CXX="$lofar_insuredir/insuretool $have_insure $have_inuse $have_tca "'$(top_builddir)'" $insuretools_prefix $CXX"

    \rm -f psrc .psrc
    touch psrc
    echo "insure++.c_as_cpp    on" >> psrc
    echo "coverage-map-file    $lofar_curwd/tca.map" >> psrc
    echo "coverage-log-file    $lofar_curwd/tca.log" >> psrc
    echo 'header_ignore        /usr/include/c++/*,/usr/include/c++/3.2.2/*' >> psrc
    echo 'function_ignore      *_default_alloc_template*' >> psrc
    echo '#threaded_runtime     on' >> psrc
    echo 'demangle             full_types' >> psrc
    echo 'insure++.coverage_boolean off' >> psrc
    echo '#insure++.inuse       on' >> psrc
    echo '#insure++.unsuppress  RETURN_FAILURE' >> psrc
    echo 'insure++.summarize coverage' >> psrc
    ln -s psrc .psrc
]
dnl
AC_SUBST(CPPFLAGS)dnl
AC_SUBST(LDFLAGS)dnl
AC_SUBST(LIBS)dnl
AC_SUBST(CC)dnl
AC_SUBST(CXX)dnl
dnl
[
  else
    if test "$force_insuretools" = "yes"; then]
AC_MSG_ERROR([Could not find Parasoft insure, inuse, tca, or Chaperon in $insuretools_prefix])
[
    fi
    enable_insuretools=no
  fi
fi]
])
#
#
#
# lofar_COMPILETOOLS
#
# Macro to check for and enable compileline logging option
#
# -------------------------
#
AC_DEFUN([lofar_COMPILETOOLS],dnl
[dnl
AC_PREREQ(2.13)dnl
AC_ARG_WITH(compiletools,
	[  --with-compiletools       enable compile command logging],
	[with_compiletools="$withval"],
	[with_compiletools="yes"])dnl
[
if test "$with_compiletools" = "yes"; then
    # Get absolute top_srcdir for comiletool
    lofar_srcdir=`cd $srcdir && pwd`
    lofar_base=`echo $lofar_srcdir | sed -e "s%/LOFAR/.*%/LOFAR%"`
    lofar_tooldir=$lofar_base/autoconf_share

#    CC="$lofar_tooldir/compiletool CC $CC"
#    CXX="$lofar_tooldir/compiletool CXX $CXX"
    LIBTOOL="$lofar_tooldir/compiletool LIBTOOL $LIBTOOL"
fi
]
dnl
AC_SUBST(LIBTOOL)dnl
dnl
])
