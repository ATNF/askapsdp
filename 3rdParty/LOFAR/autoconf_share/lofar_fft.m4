#  lofar_fftw.m4
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
#  $Id: lofar_fft.m4,v 1.3 2006/08/28 09:12:24 romein Exp $


# lofar_FFTW2
#
# Macro to check for FFTW version 2.x installation.
#
# lofar_FFTW2(option)
#     option 0 means that FFTW2 is optional, otherwise mandatory.
#
# e.g. lofar_FFTW2(1)
# -------------------------
#
AC_DEFUN([lofar_FFTW2],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], [lfr_option=0], [lfr_option=$1])
lofar_EXTERNAL(FFTW2,[$lfr_option],fftw.h,"rfftw fftw")
])

# lofar_FFTW3
#
# Macro to check for FFTW version 3.x installation.
#
# lofar_FFTW3(option)
#     option 0 means that FFTW3 is optional, otherwise mandatory.
#
# e.g. lofar_FFTW3(1)
# -------------------------
#
AC_DEFUN([lofar_FFTW3],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], [lfr_option=0], [lfr_option=$1])
lofar_EXTERNAL(FFTW3,[$lfr_option],fftw3.h)
])

# lofar_FFTW3F(option) checks the single-precision library
AC_DEFUN([lofar_FFTW3F],dnl
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], [lfr_option=0], [lfr_option=$1])
lofar_EXTERNAL(FFTW3,[$lfr_option],fftw3.h,fftw3f)
])

