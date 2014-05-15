#  lofar_external.m4
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
#  $Id: lofar_external.m4,v 1.23 2007/03/07 14:56:58 loose Exp $


# lofar_EXTERNAL
#
# Macro to check for installation of external packages (e.g. FFTW)
#
# lofar_EXTERNAL(package, [option], [headerfile], [libraries], [searchpath].
#            [extra_cppflags],[extra_cxxflags],[extra_ldflags],[extra_libs]
#     package is the name of the external package (e.g. BLITZ)
#     option 0 means that package is optional, otherwise mandatory.
#         default option=0
#     headerfile is the header file to be looked for
#     libraries are the package libraries to be looked for and to be
#         added to the LD command
#         separate by blanks if multiple libraries
#         default is package name in lowercase
#     searchpath is the path to look for header files and libraries
#         the path must be separated by blanks
#         +prefix is a special name; it is replaced by the --prefix value
#           which can be used to find a package in the install directory.
#         +root is a special name; it is replaced by the --with-lofar value
#           which can be used to find a package in the lofar system wide
#           install directory.
#         +pkg is a special name; it is replaced by the package name.
#         +comp is a special name; it is replaced by the compiler name.
#         +vers is a special name; it is replaced by the package version
#               which can be given as e.g.  --with-python-version=2.2
#         The default is taken from the option --with-searchpath.
#         If empty, the default is
#          "+prefix +root /usr/local/+pkg+vers/+comp /usr/local/+pkg+vers
#           /usr/local /usr"
#         The header and libraries are looked up in each directory of the
#         search path and in include/lib subdirectories of them.
#         The first match is taken.
#         Note that at configure time the user can specify the directory
#         for header and library which overrides the searchpath.
#     extra_cppflags, extra_cxxflags, extra_ldflags, and extra_libs
#         are extra options for the preprocessor, compiler, and linker.
#         It is a blank separated list of options. An option can be preceeded
#         by the compiler type and a colon indicating that the flag should
#         only be used for that compiler.
#         E.g. gnu:-Wno-unused
#             could be used for the blitz package to avoid too many warnings.
#         
#
# For example:
#  lofar_EXTERNAL (blitz,1,blitz/blitz.h,,,,"gnu:-Wno-unused",,-lm)
#    configures the blitz package.
#    When compiling with gnu3, the default search path is
#      /usr/local/blitz/gnu3 /usr/local/blitz/gnu /usr/local/blitz
#      /usr/local /usr
#
# -------------------------
#
AC_DEFUN([lofar_EXTERNAL],dnl
[dnl
AC_PREREQ(2.13)dnl
lfr_pkgnam=$1
define(LOFAR_EXT_SYM,m4_toupper(patsubst([$1], [.*/])))
define(LOFAR_EXT_LIB,m4_tolower(patsubst([$1], [.*/])))
ifelse($2, [], [lfr_option=0], [lfr_option=$2])
ifelse($3, [], [lfr_hdr=""], [lfr_hdr=$3])
ifelse($4, [], [lfr_libs=LOFAR_EXT_LIB], [lfr_libs=$4])
ifelse($5, [], [lfr_search=""], [lfr_search=$5])
AC_ARG_WITH([LOFAR_EXT_LIB],
	[  --with-LOFAR_EXT_LIB[[=PFX]]        path to $1 directory],
	[with_external=$withval
         if test "${with_external}" = yes; then
            with_external=
         fi],
	[with_external=])

AC_ARG_WITH([LOFAR_EXT_LIB][[-libdir]],
  [  --with-LOFAR_EXT_LIB[-libdir]=PFX   specific library dir for $1 library],
  [lfr_external_libdir="$withval"],
  [lfr_external_libdir=])

AC_ARG_WITH([LOFAR_EXT_LIB][[-version]],
  [  --with-LOFAR_EXT_LIB[-version]=PFX  specific version for $1],
  [lfr_ext_version="$withval"],
  [lfr_ext_version=])

AC_ARG_WITH([[searchpath]],
  [  --with-searchpath="+prefix +root /usr/local/+pkg+vers/+comp /usr/local/+pkg+vers /usr/local /usr"   package searchpath],
  [lfr_ext_searchp="$withval"],
  [lfr_ext_searchp=])

[
##
## Set default searchpath if not given.
## Append to search path given in 'call'.
##
if test "$lfr_ext_searchp" = ""  -o  "$lfr_ext_searchp" = "yes"  -o  "$lfr_ext_searchp" = "no"; then
  lfr_ext_searchp="+prefix +root /usr/local/+pkg+vers/+comp /usr/local/+pkg+vers /usr/local /usr"
fi
lfr_search="$lfr_search $lfr_ext_searchp"
##
## Set version to blank if it is yes or no.
##
if test "$lfr_ext_version" = "no"  -o  "$lfr_ext_version" = "yes"; then
  lfr_ext_version=;
fi
##
## Look if an external package is used.
## It is if mandatory or if given by user.
## Also determine the given search path.
##
lfr_ext_name=]LOFAR_EXT_LIB[
enable_external=no
if test "$lfr_option" = "1"; then
  enable_external=yes
fi
if test "$with_external" = "no"; then
  if test "$enable_external" = "yes"; then
]
    AC_MSG_ERROR(--with-LOFAR_EXT_LIB=no cannot be given; LOFAR_EXT_SYM package is mandatory)
[
  fi
else
  if test "$with_external" = ""; then
    external_search=
    if test "$with_external_libdir" != ""; then
      enable_external=yes
    fi
  else
    if test "$with_external" = "yes"; then
      external_search=
    else
      external_search=$with_external
    fi
    enable_external=yes
  fi
##
## Get build compiler and type
##
  lfr_buildcomp=`echo $lofar_variant | sed -e "s/_.*//"`
  lfr_buildtype=`echo $lofar_variant | sed -e "s/.*_//"`
##
## Get the extra flags, possibly compiler dependent.
##
  lfr_cpp=$6
  lfr_cxx=$7
  lfr_ld=$8
  lfr_lb=$9

  lfr_extra_cpp=
  for flag in $lfr_cpp
  do
    flagv=`echo $flag | sed -e "s/.*://"`
    flagc=`echo $flag | sed -e "s/:.*//"`
    if [ "$flagc" = "$flagv" ]; then
      flagc="";
    fi
    flagcc=`echo $flagc | sed -e "s/_.*//"`
    flagct=`echo $flagc | sed -e "s/.*_//"`
    if [ "$flagct" = "$flagcc" ]; then
      flagct="";
    fi
    if [ "$flagcc" = "" -o "$flagcc" = "$lfr_buildcomp" ]; then
      if [ "$flagct" = "" -o "$flagct" = "$lfr_buildtype" ]; then
        lfr_extra_cpp="$lfr_extra_cpp $flagv";
      fi
    fi
  done

  lfr_extra_cxx=
  for flag in $lfr_cxx
  do
    flagv=`echo $flag | sed -e "s/.*://"`
    flagc=`echo $flag | sed -e "s/:.*//"`
    if [ "$flagc" = "$flagv" ]; then
      flagc="";
    fi
    flagcc=`echo $flagc | sed -e "s/_.*//"`
    flagct=`echo $flagc | sed -e "s/.*_//"`
    if [ "$flagct" = "$flagcc" ]; then
      flagct="";
    fi
    if [ "$flagcc" = "" -o "$flagcc" = "$lfr_buildcomp" ]; then
      if [ "$flagct" = "" -o "$flagct" = "$lfr_buildtype" ]; then
        lfr_extra_cxx="$lfr_extra_cxx $flagv";
      fi
    fi
  done

  lfr_extra_ld=
  for flag in $lfr_ld
  do
    flagv=`echo $flag | sed -e "s/.*://"`
    flagc=`echo $flag | sed -e "s/:.*//"`
    if [ "$flagc" = "$flagv" ]; then
      flagc="";
    fi
    flagcc=`echo $flagc | sed -e "s/_.*//"`
    flagct=`echo $flagc | sed -e "s/.*_//"`
    if [ "$flagct" = "$flagcc" ]; then
      flagct="";
    fi
    if [ "$flagcc" = "" -o "$flagcc" = "$lfr_buildcomp" ]; then
      if [ "$flagct" = "" -o "$flagct" = "$lfr_buildtype" ]; then
        lfr_extra_ld="$lfr_extra_ld $flagv";
      fi
    fi
  done

  lfr_extra_libs=
  for flag in $lfr_lb
  do
    flagv=`echo $flag | sed -e "s/.*://"`
    flagc=`echo $flag | sed -e "s/:.*//"`
    if [ "$flagc" = "$flagv" ]; then
      flagc="";
    fi
    flagcc=`echo $flagc | sed -e "s/_.*//"`
    flagct=`echo $flagc | sed -e "s/.*_//"`
    if [ "$flagct" = "$flagcc" ]; then
      flagct="";
    fi
    if [ "$flagcc" = "" -o "$flagcc" = "$lfr_buildcomp" ]; then
      if [ "$flagct" = "" -o "$flagct" = "$lfr_buildtype" ]; then
        lfr_extra_libs="$lfr_extra_libs $flagv";
      fi
    fi
  done

##
## Replace +prefix, +root, +pkg, +vers and +comp in search list.
##
  external_slist=$external_search;
  if test "$external_slist" = ""; then
    external_slist="$lfr_search";
  fi
  if test "$external_slist" = ""; then
    external_slist="/usr/local /usr";
  fi
  lfr_slist=
  for bdir in $external_slist
  do
    lfr_a0=`echo $bdir | sed -e "s%+prefix%$prefix%g"`
    lfr_a0=`echo $lfr_a0 | sed -e "s%+root%$LOFARROOT%g"`
    lfr_a0=`echo $lfr_a0 | sed -e "s%+pkg%$lfr_ext_name%g"`
    lfr_a=`echo $lfr_a0 | sed -e "s%+vers%$lfr_ext_version%g"`
    if test "$lfr_buildcomp" != ""; then
      lfr_b=`echo $lfr_a | sed -e "s%+comp%$lfr_buildcomp%"`
      lfr_slist="$lfr_slist $lfr_b"
    fi
    if test "$lfr_a" != "$lfr_b"; then
      if test "$lfr_buildcomp" != "$lofar_compiler"; then
        lfr_b=`echo $lfr_a | sed -e "s%+comp%$lofar_compiler%"`
        lfr_slist="$lfr_slist $lfr_b"
      fi
    fi
  done

## Look for the header file or first library in directories of the search list
## and in its include or lib subdirectories.
## Assume that libraries are in similar directory structure as headers.
## (thus in lib subdirectory if header is in include subdirectory)

## Use different lib directory on 64 bit systems.
  lfr_libdext=lib
  if test "`arch`" = "x86_64"; then
    lfr_libdext=lib64;
  fi
## Put possible version into the library names.
  lfr_libsc=`echo $lfr_libs | sed -e "s%+vers%$lfr_ext_version%g"`
## Search for header if given, otherwise for library (.a or .so).
  lfr_searchfil=
  if test "$lfr_hdr" != ""; then
    lfr_searchfil=$lfr_hdr
    lfr_searchext=include
  else
    if test "$lfr_libsc" != ""; then
      lfr_sfil=lib`echo "$lfr_libsc" | sed -e 's/ .*//'`
      lfr_searchfil="$lfr_sfil.so $lfr_sfil.a"
      lfr_searchext=$lfr_libdext
    fi
  fi
## Now fo the actual search.
## Stop if a file is found.
  lfr_ext_dir=
  lfr_pkg_rootdir=
  for bdir in $lfr_slist
  do
    for bfil in $lfr_searchfil
    do
      ]AC_CHECK_FILE([$bdir/$lfr_searchext/$bfil],
			[lfr_ext_dir=$bdir/$lfr_searchext],
			[lfr_ext_dir=no])[
      if test "$lfr_ext_dir" != "no" ; then
        if test "$lfr_external_libdir" = ""; then
          lfr_external_libdir=$bdir/$lfr_libdext;
        fi
        break;
      fi
      ]AC_CHECK_FILE([$bdir/$bfil],
			[lfr_ext_dir=$bdir],
			[lfr_ext_dir=no])[
      if test "$lfr_ext_dir" != "no" ; then
        if test "$lfr_external_libdir" = ""; then
          lfr_external_libdir=$bdir;
        fi
        break;
      fi
    done
    if test "$lfr_ext_dir" != "no" ; then
      lfr_pkg_rootdir=$bdir
      break
    fi
  done
# Now search for the libraries.
  lfr_depend=
  lfr_ext_lib=
  if test "$lfr_external_libdir" != ""; then
    for lib in $lfr_libsc
    do
      if test "$lfr_ext_lib" != "no" ; then
        ]AC_CHECK_FILE([$lfr_external_libdir/lib$lib.so],
			[lfr_ext_lib=$lfr_external_libdir],
			[lfr_ext_lib=no])[
        if test "$lfr_ext_lib" == "no" ; then
          ]AC_CHECK_FILE([$lfr_external_libdir/lib$lib.a],
			[lfr_ext_lib=$lfr_external_libdir],
			[lfr_ext_lib=no])[
          lfr_depend="$lfr_depend $lfr_external_libdir/lib$lib.a"
        else
          lfr_depend="$lfr_depend $lfr_external_libdir/lib$lib.so"
        fi
      fi
    done
  fi
  if test "$lfr_ext_dir" != "no"  -a  "$lfr_ext_lib" != "no" ; then
    EXTERNAL_CPPFLAGS=
    EXTERNAL_CXXFLAGS=
    EXTERNAL_LDFLAGS=
    EXTERNAL_LIBS=
    if test "$lfr_hdr" != ""; then
      if test "$lfr_ext_dir" != "/usr/include" -a \
              "$lfr_ext_dir" != "/usr/local/include" ; then
        EXTERNAL_CPPFLAGS="-I$lfr_ext_dir"
      fi
    fi
    if test "$lfr_ext_lib" != "" ; then
      EXTERNAL_LDFLAGS="-L$lfr_ext_lib"
      if test "$lofar_compiler" != "pgi"; then
        EXTERNAL_LDFLAGS="$EXTERNAL_LDFLAGS -Wl,-rpath,$lfr_ext_lib"
      fi
    fi
    for lib in $lfr_libsc
    do
      EXTERNAL_LIBS="$EXTERNAL_LIBS -l$lib"
    done

    EXTERNAL_CPPFLAGS="$EXTERNAL_CPPFLAGS $lfr_extra_cpp"
    EXTERNAL_CXXFLAGS="$EXTERNAL_CXXFLAGS $lfr_extra_cxx"
    EXTERNAL_LDFLAGS="$EXTERNAL_LDFLAGS $lfr_extra_ld"
    EXTERNAL_LIBS="$EXTERNAL_LIBS $lfr_extra_libs"

    echo ]LOFAR_EXT_SYM[ >> pkgext
    echo "$EXTERNAL_CPPFLAGS" >> pkgextcppflags
    echo "$EXTERNAL_CXXFLAGS" >> pkgextcxxflags
    echo "$EXTERNAL_LDFLAGS" >> pkgextldflags

    # Get all new external packages used by this package and their flags.
    lfr_pkgcfgdir=$lfr_pkg_rootdir/config/$lfr_pkgnam
    $lofar_sharedir/makeext pkgext $lfr_pkgcfgdir
    $lofar_sharedir/makeext pkgextcppflags $lfr_pkgcfgdir
    $lofar_sharedir/makeext pkgextldflags $lfr_pkgcfgdir

    # Define which external packages are used by this package.
    for pkg in `cat pkgext_diff`
    do
      echo "" >> lofar_config.h-pkg;
      echo "#if !defined(HAVE_$pkg)" >> lofar_config.h-pkg
      echo "# define HAVE_$pkg 1" >> lofar_config.h-pkg;
      echo "#endif" >> lofar_config.h-pkg;
    done

    # Do the finalization
    cp lofar_config.h-pkg lofar_config.h
    echo "" >> lofar_config.h
    echo "#endif" >> lofar_config.h

    # If the current lofar_config.h is the same as the old one, copy the
    # old one back, while preserving creation date and time.
    diff lofar_config.h lofar_config.old-h > /dev/null 2>&1
    if [ $? = 0 ]; then
      cp -p lofar_config.old-h lofar_config.h
    fi

    # Update EXTERNAL_CPPFLAGS and EXTERNAL_LDFLAGS. Do not update
    # EXTERNAL_CXXFLAGS, since we do not want to propagate C++ compiler flags.
    # Use tr to translate any newline into a space.
    EXTERNAL_CPPFLAGS=`cat pkgextcppflags | tr $'\n' " "`
    EXTERNAL_LDFLAGS=`cat pkgextldflags | tr $'\n' " "`

    # Finally, update the flags that will be exported. Remove any remaining
    # duplicate tokens. Note that this will sort the flags.
    # Do not sort LIBS since the order is usually important.
    CPPFLAGS=`echo "$CPPFLAGS $EXTERNAL_CPPFLAGS" | tr " " $'\n' | \
              sort | uniq | tr $'\n' " "`
    CXXFLAGS=`echo "$CXXFLAGS $EXTERNAL_CXXFLAGS" | tr " " $'\n' | \
              sort | uniq | tr $'\n' " "`
    LDFLAGS=`echo "$LDFLAGS $EXTERNAL_LDFLAGS" | tr " " $'\n' | \
             sort | uniq | tr $'\n' " "`
    LIBS="$LIBS $EXTERNAL_LIBS"
    LOFAR_DEPEND="$LOFAR_DEPEND $lfr_depend"

    enable_external=yes
]
dnl
    AC_SUBST(CPPFLAGS)dnl
    AC_SUBST(CXXFLAGS)dnl
    AC_SUBST(LDFLAGS)dnl
    AC_SUBST(LIBS)dnl
    AC_SUBST(LOFAR_DEPEND)dnl
dnl
    AC_DEFINE([HAVE_]LOFAR_EXT_SYM, 1, [Define if ]LOFAR_EXT_SYM[ is installed])dnl
[
  else
    if test "$enable_external" = "yes" ; then
]
      AC_MSG_ERROR([Could not find ]LOFAR_EXT_SYM[ headers or library in $lfr_slist])
[
    fi
    enable_external=no
  fi
fi]
AM_CONDITIONAL([HAVE_]LOFAR_EXT_SYM, [test "$enable_external" = "yes"])
])
