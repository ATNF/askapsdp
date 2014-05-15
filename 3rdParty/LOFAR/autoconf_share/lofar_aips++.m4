#  lofar_aips.m4
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
#  $Id: lofar_aips++.m4,v 1.33 2006/10/24 03:12:48 diepen Exp $


# lofar_AIPSPP
#
# Macro to check for AIPS++ installation
# If AIPS++ functions are used that use LAPACK, lofar_LAPACK should
# also be made part of the configure.in (after lofar_AIPSPP).
#
# lofar_AIPSPP(option,libraries)
#     option 0 means that AIPS++ is optional, otherwise mandatory.
#     the libraries are optional and can be given as e.g. "-ltables -lcasa"
#
# e.g. lofar_AIPSPP
# -------------------------
#
AC_DEFUN([lofar_AIPSPP],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], [lfr_option=0], [lfr_option=$1])
ifelse($2, [], [lfr_aipslibs="-ltasking -lms -lfits -lmeasures -ltables -lscimath -lscimath_f -lcasa -lglish -lsos -lnpd"], [lfr_aipslibs=$2])
AC_ARG_WITH(aipspp,
	[  --with-aipspp[=PFX]         enable use of AIPS++ (via AIPSPATH or explicit path)],
	[with_aipspp="$withval"],
	[with_aipspp=""])
AC_ARG_WITH(casa,
	[  --with-casa[=PFX]           enable use of casa (via casapath or explicit path)],
	[with_casa="$withval"],
	[with_casa=""])
AC_ARG_WITH(pgplot,
	[  --with-pgplot[=PFX]         enable use of PGPLOT if needed by AIPS++],
	[with_pgplot="$withval"],
	[with_pgplot=""])
AC_ARG_WITH(wcs,
	[  --with-wcs[=PFX]            specific path for wcslib if needed by AIPS++],
	[with_wcs="$withval"],
	[with_wcs=""])
[


if test "$with_pgplot" = ""; then
    with_pgplot=no;
fi
if test "$with_wcs" = ""; then
    with_wcs=yes;
fi

# the path where the libraries can be found
AIPSPP_LIB_PATH=""
# the path where the include files can be found
AIPSPP_INC_PATH=""

# if casa and aips are both set, casa will be used
# check the --with-casa option
if test "$with_casa" = ""; then
  # --with-casa was not given, so look in casapath if it exists
  ]AC_MSG_CHECKING([whether casapath environment variable is set])[
  if test "${casapath}set" != "set"; then
    ]AC_MSG_RESULT([yes])[
    AIPSPP_LIB_PATH="${AIPSPP_LIB_PATH} ${casapath}"
    AIPSPP_INC_PATH="${AIPSPP_INC_PATH} ${casapath}/include/casa"
  else
    ]AC_MSG_RESULT([no])[
  fi
elif test "$with_casa" = "no"; then
  # do nothing
  echo
elif test "$with_casa" = "yes"; then
  # --with-casa was given, so look in casapath if it exists otherwise give an error
  ]AC_MSG_CHECKING([whether casapath environment variable is set])[
  if test "${casapath}set" != "set"; then
    ]AC_MSG_RESULT([yes])[
    AIPSPP_LIB_PATH="${AIPSPP_LIB_PATH} ${casapath}"
    AIPSPP_INC_PATH="${AIPSPP_INC_PATH} ${casapath}/include/casa"
  else
    echo
    ]AC_MSG_RESULT([no])[
    ]AC_MSG_ERROR([--with-casa=yes has been given, but casapath has not been set])[
  fi
else
  # the casa path was given manually so look there
  AIPSPP_LIB_PATH="${AIPSPP_LIB_PATH} ${AIPSPP_LIB_PATH} ${with_casa}"
  AIPSPP_INC_PATH="${AIPSPP_INC_PATH} ${AIPSPP_INC_PATH} ${with_casa}/include/casa"
fi

# check the --with-aipspp option
if test "$with_aipspp" = ""; then
  # --with-aips was not given, so look in AIPSPATH if it exists
  ]AC_MSG_CHECKING([whether AIPSPATH environment variable is set])[
  if test "${AIPSPATH}set" != "set"; then
    ]AC_MSG_RESULT([yes])[
    AIPSPP_LIB_PATH="${AIPSPP_LIB_PATH} `expr "$AIPSPATH" : "\(.*\) .* .* .*"`/`expr "$AIPSPATH" : ".* \(.*\) .* .*"`"
    AIPSPP_INC_PATH="${AIPSPP_INC_PATH} `expr "$AIPSPATH" : "\(.*\) .* .* .*"`/code/include"
  else
    ]AC_MSG_RESULT([no])[
  fi
elif test "$with_aipspp" = "no"; then
  # do nothing
  echo
elif test "$with_aipspp" = "yes"; then
  # --with-aips was given, so look in AIPSPATH if it exists otherwise give an error
  ]AC_MSG_CHECKING([whether AIPSPATH environment variable is set])[
  if test "${AIPSPATH}set" != "set"; then
    ]AC_MSG_RESULT([yes])[
    AIPSPP_LIB_PATH="${AIPSPP_LIB_PATH} `expr "$AIPSPATH" : "\(.*\) .* .* .*"`/`expr "$AIPSPATH" : ".* \(.*\) .* .*"`"
    AIPSPP_INC_PATH="${AIPSPP_INC_PATH} `expr "$AIPSPATH" : "\(.*\) .* .* .*"`/code/include"
  else
    echo
    ]AC_MSG_RESULT([no])[
    ]AC_MSG_ERROR([--with-aipspp=yes has been given, but AIPSPATH has not been set])[
  fi
else
  # the aips path was given manually so look there
  AIPSPP_LIB_PATH="${AIPSPP_LIB_PATH} ${with_aipspp}";
  AIPSPP_INC_PATH="${AIPSPP_INC_PATH} `dirname $with_aipspp`/code/include";
fi

# Do we have enough info?
if test "$AIPSPP_LIB_PATH" = ""; then
  if test "$lfr_option" != "0"; then
    ]AC_MSG_ERROR([AIPS++ is needed, but casapath and AIPSPATH have not been set and  --with-aipspp=path and --with-casa=path have not been given])[
  fi
else
  # Now we have the paths and we can start searching for the aips installation

  # I don't know a nicer way to do a for loop over two lists
  INDEX=0
  for dummy in $AIPSPP_LIB_PATH; do
    let "INDEX += 1"
    AIP=`echo $AIPSPP_INC_PATH | awk '{print $'$INDEX'}'`
    ALP=`echo $AIPSPP_LIB_PATH | awk '{print $'$INDEX'}'`

    ALL_FOUND="yes"
    ]AC_CHECK_FILE([$AIP],, [ALL_FOUND="no"])[
    ]AC_CHECK_FILE([$ALP/lib],, [ALL_FOUND="no"])[
    if test "$ALL_FOUND" = "yes"; then
      # we have a winner
      break
    fi
  done

  if test "$ALL_FOUND" = "no"; then 
    # the paths do not contain the needed files/dirs
    if test "$lfr_option" != "0"; then
      ]AC_MSG_ERROR([AIPS++ installation not found])[
    fi
  else
    case `uname -s` in
      SunOS)  arch=SOLARIS;;
      Linux)  arch=LINUX;;
      IRIX32) arch=IRIX;;
      IRIX64) arch=IRIX;;
      *)      arch=UNKNOWN;;
    esac


    # Define the CPP flags.
    AIPSPP_CPPFLAGS="-I$AIP"
    if [ "$with_wcs" != "no" ]; then
      if [ "$with_wcs" = "yes" ]; then
        with_wcs=`echo $AIP | sed -e 's%/include.*%/casa%'`
      fi
      ]AC_CHECK_FILE([$with_wcs/wcslib/wcs.h],
            [lfr_wcs=yes], [lfr_wcs=no])[
      if test $lfr_wcs = no; then
        ]AC_MSG_ERROR([WCS directory not found])[
      fi
      AIPSPP_CPPFLAGS="$AIPSPP_CPPFLAGS -I$with_wcs"
    fi      
    AIPSPP_CPPFLAGS="$AIPSPP_CPPFLAGS -DAIPS_$arch -DAIPS_NO_TEMPLATE_SRC"
    # Check if we're on a x86-64 bit system.
    if test "`arch`" = "x86_64"; then
      AIPSPP_CPPFLAGS="$AIPSPP_CPPFLAGS -DAIPS_64B"
    fi
    if test "$lofar_compiler" = "kcc"; then
      AIPSPP_CPPFLAGS="$AIPSPP_CPPFLAGS -DAIPS_KAICC"
    fi
    AIPSPP_LDFLAGS="-L$ALP/lib"
    if test "$lofar_compiler" != "pgi"; then
      AIPSPP_LDFLAGS="$AIPSPP_LDFLAGS -Wl,-rpath,$ALP/lib"
    fi
    AIPSPP_LIBS="$ALP/lib/version.o $lfr_aipslibs"

    # If we're using GCC 4.x, we need to add -lgfortran to AIPSPP_LIBS. 
    # This should be handled by the AIPS++ build environment IMHO.
    version=`$CXX -v 2>&1 | tail -1`
    # A typical version strings looks like this:
    #   gcc version 4.0.2 20050901 (prerelease) (SUSE Linux)
    if echo $version | grep -iq gcc; then
      # We'll assume that the first word starting with one or more digits is
      # the version number, so strip off the rest.
      version=`echo $version | sed -e 's,^[^0-9]*,,' -e 's,[ \t].*$,,'`
      # We only need the major version number.
      major=`echo $version | cut -s -d'.' -f1`
      if test $major -ge 4; then
        AIPSPP_LIBS="$AIPSPP_LIBS -lgfortran"
      fi
    fi

    if test "$with_pgplot" != "no"; then
      ]AC_CHECK_FILE([$with_pgplot],
            [lfr_pg=yes], [lfr_pg=no])[
      if test $lfr_pg = no; then
        ]AC_MSG_ERROR([given PGPLOT directory not found])[
      fi
      AIPSPP_LDFLAGS="$AIPSPP_LDFLAGS -L$with_pgplot"
      AIPSPP_LIBS="$AIPSPP_LIBS -lcpgplot -lpgplot"
    fi

    echo AIPSPP >> pkgext
    echo "$AIPSPP_CPPFLAGS" >> pkgextcppflags

    CPPFLAGS="$CPPFLAGS $AIPSPP_CPPFLAGS"
    if test "$lofar_compiler" = "gnu"; then
      CXXFLAGS="$CXXFLAGS -Wno-non-template-friend"
      echo "-Wno-non-template-friend" >> pkgextcxxflags
    fi
    LDFLAGS="$LDFLAGS $AIPSPP_LDFLAGS"
    LIBS="$LIBS $AIPSPP_LIBS"
    AIPSPP="$ALP"

    ]
dnl
    AC_SUBST(AIPSPP)dnl
    AC_SUBST(CPPFLAGS)dnl
    AC_SUBST(CXXFLAGS)dnl
    AC_SUBST(LDFLAGS)dnl
    AC_SUBST(LIBS)dnl
dnl
    AC_DEFINE(HAVE_AIPSPP, 1, [Define if AIPS++ is installed])dnl
    [
  fi
fi
]
])
