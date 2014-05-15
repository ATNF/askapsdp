#  lofar_package.m4
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
#  $Id: lofar_package.m4,v 1.21 2006/01/19 14:19:45 diepen Exp $


# lofar_PACKAGE
#
# Macro to check for the installation of a package
#
#  It creates a symlink to the package include directory and
#  symlinks to the package library.
#  The library name is added to the LIBS list.
#  
#  Note that a package is a directory with its own src subdirectory.
#  E.g. BaseSim or CEP/CPA/PSCF.
#  E.g. BaseSim/src/Corba is not a package and its object files
#  must be part of the main BaseSim library.
#
#  The package source files are part of the LOFAR tree.
#  The package library resides in a build area which can be anywhere.
#  The build area has subdirectories containing the build variant
#  like gnu_debug. This subdirectory is used for the build.
#  It can be found automatically if it resides in a standard build area
#  which can be:
#  1. A subdirectory build of the source tree (possibly using a symlink).
#  2. In a directory structure similar to the currently used build area.
#     E.g. if building in somewhere/MyPackage/build/x, a package BaseSim
#     is automatically found if it resides in somewhere/BaseSim/build/x.
#     Similarly, if building in somewhere/build/x/MyPackage, BaseSim
#     is automatically found if residing in somewhere/build/x/BaseSim.
#
#  The lofar root (see lofar_init.m4) sets the root of the system LOFAR
#  directory to use (e.g. lofar/daily).
#  
#  It is possible that the build area is elsewhere, but in that case
#  it need to be given explicitly using --with-package-libdir.
#
#  --with-package  tries to find the paths automatically.
#                  include path is derived from $srcdir
#                  library path is derived from `pwd`
#  --with-package=PFX  specifies basic package path explicitly.
#                      derived include path gets PFX/src
#                      derived library path gets PFX/build
#  --with-package-libdir=PFX   overrides package derived library path.
#
# lofar_PACKAGE(package_path, recoption, cvs-versiontag, option)
#     where recoption=0 means that makepkglinks descends recursively into
#                       all packages used by this package (default).
#           recoption=1 means not recursively
#           recoption=2 means not recursively and no library in LIBS
#           cvs-versiontag is an optional cvstag telling the package version
#             It is used by rub.
#           option = 0 means that the package is optional
#           option = 1 means that the package is mandatory 
#
# e.g. lofar_PACKAGE(CEP/CPA/PSCF)
# -------------------------
#
AC_DEFUN([lofar_PACKAGE],dnl
[dnl
AC_PREREQ(2.13)dnl
define(LOFAR_PKG_SYM,patsubst([$1], [.*/]))
define(LOFAR_PKG_LIB,m4_tolower(patsubst([$1], [.*/])))
ifelse($2, [], [lfr_recoption=0], [lfr_recoption=$2])
ifelse($4, [], [lfr_option=1], [lfr_option=$4])
AC_ARG_WITH([[lofar-]][LOFAR_PKG_LIB],
	[  --with-lofar-LOFAR_PKG_LIB[[=PFX]]        path to $1 directory],
	[with_package=$withval
         if test "${with_package}" = yes; then
            with_package=
         fi],
	[with_package=])

AC_ARG_WITH([[lofar-]][LOFAR_PKG_LIB][[-libdir]],
  [  --with-lofar-LOFAR_PKG_LIB[-libdir]=PFX   specific library dir for $1 library],
  [lfr_package_libdir="$withval"],
  [lfr_package_libdir=])


[
enable_package=yes
lfr_pkgnam=$1
if test "$with_package" = "no"; then
  if test "$lfr_option" != 0; then]
AC_MSG_ERROR([Cannot configure without package $1])
[ fi
else

# Get full path of build and source directory.
#
  lfr_curwd=`pwd`;
  lfr_curdir=`basename $lfr_curwd`;
  lfr_srcdir=`cd $srcdir && pwd`;
  lfr_variant=;
  lfr_libdir=;

# Handle a version and/or variant in a similar way as done in lofar_init.m4.
  lfr_root="$with_package"
  case "$with_package" in
  *:*)
    lfr_root=`echo ${with_package} | sed -e "s/:.*//"`
    lfr_variant=`echo ${with_package} | sed -e "s/.*://"`
    ;;
  esac
# Remove possible trailing /LOFAR/package/.
  lfr_root=`echo $lfr_root | sed -e 's%/$%%' -e 's%/$lfr_pkgnam$%%' -e 's%/LOFAR$%%'`;

# If no version is given, use root if mandatory or if package is not
# available locally.
  if test "$lfr_root" = ""; then
    if test "$lofar_use_root" = "1"; then
      lfr_root=$lofar_root;
    else
      lfr_root=$lofar_top_srcdir;
      ]AC_CHECK_FILE([$lfr_root/$lfr_pkgnam/configure],
	             [lfr_var=yes], [lfr_var=no])[
      if test $lfr_var = no; then
        lfr_root=$lofar_root;
      fi
    fi
  else
  # Take version as given. Expand a possible tilde.
  # Add /lofar if only a 'symbolic' version name is given.
    case "$lfr_root" in
    root)
      lfr_root=$lofar_root;
      ;;
    ~*)
      lfr_root=`echo $lfr_root | sed -e "s%~%$HOME%"`/LOFAR;
      lfr_libdir=$lfr_root/$lfr_pkgnam/build/$lofar_variant;
      ;;
    */*)
      lfr_root=$lfr_root/LOFAR;
      ;;
    *)
      # something like daily
      lfr_root=/home/lofar/$lfr_root/LOFAR;
      lfr_libdir=$lfr_root/$lfr_pkgnam/build/$lofar_variant;
      ;;
    esac
  fi
# Form the include directory (is src directory of package).
  lfr_include=$lfr_root/$lfr_pkgnam;

# Add compiler type to variant if needed.
  if test "$lfr_variant" != ""; then
    case "$lfr_variant" in
    *_*)
      ;;
    *)
      lfr_variant=${lofar_compiler}_$lfr_variant;
      ;;
    esac
  fi

# Now get the library.
# If explicitly given, there is no problem at all.
  if test "$lfr_package_libdir" != ""; then
    lfr_libdir=$lfr_package_libdir;
  fi
  if test "$lfr_libdir" = ""; then
  # Use root directory if used and if library is given.
    if test "$lfr_root" = "$lofar_root"; then
      if test "$lofar_root_libdir" != ""; then
        lfr_libdir=`echo "$lofar_root_libdir" | sed -e 's%<package>%$lfr_pkgnam%'`;
      fi
    fi
    if test "$lfr_libdir" = ""; then
    # The sources are from the user's tree, so find build area there as well.
    # Try to guess where the package library directory is located.
    # pwd gives the current build directory, which can be something
    # like   something/build/variant/package
    # or     something/package/build/variant
    # It simply replaces package by the actual package name in S1.
      lfr_lib=$lfr_curwd;
      if test "$lfr_variant" != ""; then
        lfr_curvar=`pwd | sed -e "s%.*/build/%%" | sed -e "s%/.*%%g"`;
        lfr_lib=`echo $lfr_lib | sed -e "s%/$lfr_curvar%/$lfr_variant%"`
      fi
      lfr_pkg=`echo $lfr_srcdir | sed -e "s%.*/LOFAR/%%"`
      lfr_libdir=`echo $lfr_lib | sed -e "s%/$lfr_pkg%/$lfr_pkgnam%"`
    fi
  fi

## Check if the build directory is valid.
## The library might not be there yet, but it must have been configured,
## so a libtool should be there.
##
## Check for Makefile.am header file in src dir.
##
  if test $lfr_recoption != 2; then
    ]AC_CHECK_FILE([$lfr_libdir/libtool],
			[lfr_cv_lib_package=yes],
			[lfr_cv_lib_package=no])
  [else
    lfr_cv_lib_package=yes;
  fi
  ]AC_CHECK_FILE([$lfr_include/src/Makefile.am],
			[lfr_cv_hdr_package=yes],
			[lfr_cv_hdr_package=no])dnl
  [
  if test $lfr_cv_hdr_package = yes  &&  test $lfr_cv_lib_package = yes; then
  # Turn possible relative paths into absolute paths, because
  # relative paths can miss some .. parts.
    lfr_include=`cd $lfr_include && pwd`

  # Two new variables for use in Makefile.am's
    ]LOFAR_PKG_LIB[_top_srcdir="$lfr_include"

    if test $lfr_recoption != 2; then
      lfr_libdir=`cd $lfr_libdir && pwd`
      ]LOFAR_PKG_LIB[_top_builddir="$lfr_libdir"
      LDFLAGS="$LDFLAGS -L$lfr_libdir/src"
      LIBS="$LIBS -l"]LOFAR_PKG_LIB[
    fi

  # Create symlink to the source and build directory
  # Do the same (if needed recursively) for all packages used by this package.
  # Also assemble all new external packages and flags this package depends on.
    rm -f libnames_depend
    touch libnames_depend
    $lofar_sharedir/makepkglinks $lfr_pkgnam $lfr_include $lfr_libdir pkginc pkgbldinc libnames_depend lofar_config.h-pkg $lfr_recoption 0
  # Get the libraries this package is dependent on.
  # Use echo to remove the possible newlines.
    lfr_depend=`cat libnames_depend`
    lfr_depend=`echo $lfr_depend`
    rm libnames_depend
    LOFAR_DEPEND="$LOFAR_DEPEND $lfr_depend"
  # Get all new external packages used by this package and their flags.
    $lofar_sharedir/makeext pkgext $lfr_libdir;
    $lofar_sharedir/makeext pkgextcppflags $lfr_libdir;
    $lofar_sharedir/makeext pkgextcxxflags $lfr_libdir;
    $lofar_sharedir/makeext pkgextobjs $lfr_libdir;
  # Define which external packages are used by this package.
    lfr_pkgext=`cat pkgext_diff`
    lfr_pkgext=`echo $lfr_pkgext`
    for pkgnm in $lfr_pkgext
    do
      echo "" >> lofar_config.h-pkg;
      echo "#if !defined(HAVE_$pkgnm)" >> lofar_config.h-pkg
      echo "# define HAVE_$pkgnm 1" >> lofar_config.h-pkg;
      echo "#endif" >> lofar_config.h-pkg;
    done
  # Do the finalization (in case no lofar_package is used).
    cp lofar_config.h-pkg lofar_config.h
    echo "" >> lofar_config.h
    echo "#endif" >> lofar_config.h
  # If the current lofar_config.h is the same as the old one, move the
  # old one back and create it again.
    diff lofar_config.h lofar_config.old-h > /dev/null 2>&1
    if [ $? = 0 ]; then
      mv lofar_config.old-h lofar_config.h
      touch lofar_config.old-h
    fi

  # Add possible CPPFLAGS and CXXFLAGS.
    lfr_pkgext=`cat pkgextcppflags_diff`
    lfr_pkgext=`echo $lfr_pkgext`
    if [ "$lfr_pkgext" != "" ]; then
      CPPFLAGS="$CPPFLAGS $lfr_pkgext"
    fi
    lfr_pkgext=`cat pkgextcxxflags_diff`
    lfr_pkgext=`echo $lfr_pkgext`
    if [ "$lfr_pkgext" != "" ]; then
      CXXFLAGS="$CXXFLAGS $lfr_pkgext"
    fi
  # Add object files to libraries
    lfr_pkgext=`cat pkgextobjs_diff`
    lfr_pkgext=`echo $lfr_pkgext`
    if [ "$lfr_pkgext" != "" ]; then
      LIBS="$LIBS $lfr_pkgext"
    fi
]
dnl
AC_SUBST(LIBS)dnl
AC_SUBST([LOFAR_PKG_LIB][[_top_srcdir]])dnl
AC_SUBST([LOFAR_PKG_LIB][[_top_builddir]])dnl
AC_SUBST(LOFAR_DEPEND)dnl
AC_SUBST(CPPFLAGS)dnl
AC_SUBST(CXXFLAGS)dnl
dnl
[
  else
    if test $lfr_cv_hdr_package = no; then]
      AC_MSG_ERROR([Could not find Makefile.am in $lfr_include])
[
      enable_package=no
    else]
      AC_MSG_WARN([Could not find libtool in $lfr_libdir])
      AC_MSG_ERROR([Probably package $lfr_pkgnam has not been configured yet])
[
    fi
  fi
fi
]
dnl NOT NEEDED: AM_CONDITIONAL(HAVE_LOFAR_PKG, test "$enable_package" = "yes")
])
