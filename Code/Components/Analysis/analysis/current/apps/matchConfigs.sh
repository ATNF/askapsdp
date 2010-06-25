#!/bin/sh
#
# ASKAP auto-generated file
#

ASKAP_ROOT=/Users/whi550/PROJECTS/ASKAP/askapsoft
export ASKAP_ROOT

PATH=${ASKAP_ROOT}/3rdParty/fftw/fftw-3.2.2/install/bin:${ASKAP_ROOT}/3rdParty/LOFAR/Blob/Blob-1.2/install/bin:${ASKAP_ROOT}/3rdParty/LOFAR/Common/Common-3.3/install/bin:${ASKAP_ROOT}/Code/Base/askap/current/install/bin:${ASKAP_ROOT}/3rdParty/apr-util/apr-util-1.3.9/install/bin:${ASKAP_ROOT}/3rdParty/apr/apr-1.3.9/install/bin:${ASKAP_ROOT}/3rdParty/casacore/casacore-1.0.0/install/bin:${ASKAP_ROOT}/3rdParty/Duchamp/Duchamp-1.1.9/install/bin:${ASKAP_ROOT}/3rdParty/gsl/gsl-1.10/install/bin:${PATH}
export PATH

if [ "${DYLD_LIBRARY_PATH}" !=  "" ]
then
    DYLD_LIBRARY_PATH=.:${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/install/lib:${ASKAP_ROOT}/Code/Base/scimath/current/install/lib:${ASKAP_ROOT}/3rdParty/fftw/fftw-3.2.2/install/lib:${ASKAP_ROOT}/Code/Base/mwbase/askapparallel/current/install/lib:${ASKAP_ROOT}/Code/Base/mwbase/mwcommon/current/install/lib:${ASKAP_ROOT}/3rdParty/LOFAR/Blob/Blob-1.2/install/lib:${ASKAP_ROOT}/3rdParty/LOFAR/Common/Common-3.3/install/lib:${ASKAP_ROOT}/3rdParty/boost/boost-1.41.0/install/lib:${ASKAP_ROOT}/Code/Base/askap/current/install/lib:${ASKAP_ROOT}/3rdParty/log4cxx/log4cxx-0.10.0/install/lib:${ASKAP_ROOT}/3rdParty/apr-util/apr-util-1.3.9/install/lib:${ASKAP_ROOT}/3rdParty/apr/apr-1.3.9/install/lib:${ASKAP_ROOT}/3rdParty/casacore/casacore-1.0.0/install/lib:${ASKAP_ROOT}/3rdParty/lapack/lapack-3.1.1/install/lib:${ASKAP_ROOT}/3rdParty/blas/blas-2007.02.28/install/lib:${ASKAP_ROOT}/3rdParty/Duchamp/Duchamp-1.1.9/install/lib:${ASKAP_ROOT}/3rdParty/wcslib/wcslib-4.2/install/lib:${ASKAP_ROOT}/3rdParty/gsl/gsl-1.10/install/lib:${ASKAP_ROOT}/3rdParty/cfitsio/cfitsio-3.0.6/install/lib:${ASKAP_ROOT}/3rdParty/wcslib/wcslib-4.2/install/lib:${DYLD_LIBRARY_PATH}
else
    DYLD_LIBRARY_PATH=.:${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/install/lib:${ASKAP_ROOT}/Code/Base/scimath/current/install/lib:${ASKAP_ROOT}/3rdParty/fftw/fftw-3.2.2/install/lib:${ASKAP_ROOT}/Code/Base/mwbase/askapparallel/current/install/lib:${ASKAP_ROOT}/Code/Base/mwbase/mwcommon/current/install/lib:${ASKAP_ROOT}/3rdParty/LOFAR/Blob/Blob-1.2/install/lib:${ASKAP_ROOT}/3rdParty/LOFAR/Common/Common-3.3/install/lib:${ASKAP_ROOT}/3rdParty/boost/boost-1.41.0/install/lib:${ASKAP_ROOT}/Code/Base/askap/current/install/lib:${ASKAP_ROOT}/3rdParty/log4cxx/log4cxx-0.10.0/install/lib:${ASKAP_ROOT}/3rdParty/apr-util/apr-util-1.3.9/install/lib:${ASKAP_ROOT}/3rdParty/apr/apr-1.3.9/install/lib:${ASKAP_ROOT}/3rdParty/casacore/casacore-1.0.0/install/lib:${ASKAP_ROOT}/3rdParty/lapack/lapack-3.1.1/install/lib:${ASKAP_ROOT}/3rdParty/blas/blas-2007.02.28/install/lib:${ASKAP_ROOT}/3rdParty/Duchamp/Duchamp-1.1.9/install/lib:${ASKAP_ROOT}/3rdParty/wcslib/wcslib-4.2/install/lib:${ASKAP_ROOT}/3rdParty/gsl/gsl-1.10/install/lib:${ASKAP_ROOT}/3rdParty/cfitsio/cfitsio-3.0.6/install/lib:${ASKAP_ROOT}/3rdParty/wcslib/wcslib-4.2/install/lib
fi
export DYLD_LIBRARY_PATH

${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/apps/matchConfigs "$@"
