#!/bin/sh

echo "Extracting model"

echo tar zxvf ../models/10uJy.model.small.tgz

export LD_LIBRARY_PATH=.:${CONRAD_PROJECT_ROOT}/Code/Components/Synthesis/synthesis/trunk/lib:${CONRAD_PROJECT_ROOT}/Code/Base/scimath/trunk/lib:${CONRAD_PROJECT_ROOT}/3rdParty/gsl/default/lib:${CONRAD_PROJECT_ROOT}/Code/Components/CP/mwcommon/trunk/lib:${CONRAD_PROJECT_ROOT}/3rdParty/LOFAR/Blob/default/lib:${CONRAD_PROJECT_ROOT}/3rdParty/casacore/default/lib:${CONRAD_PROJECT_ROOT}/3rdParty/cfitsio/default/lib:${CONRAD_PROJECT_ROOT}/3rdParty/lapack/default/.:${CONRAD_PROJECT_ROOT}/3rdParty/blas/default:${CONRAD_PROJECT_ROOT}/3rdParty/wcslib/default/lib:${CONRAD_PROJECT_ROOT}/3rdParty/LOFAR/APS/default/lib:${CONRAD_PROJECT_ROOT}/3rdParty/LOFAR/Common/tags/default:/usr/lib:${CONRAD_PROJECT_ROOT}/Code/Base/conrad/Cc/trunk/lib:$LD_LIBRARY_PATH

export DYLD_LIBRARY_PATH=.:${CONRAD_PROJECT_ROOT}/Code/Components/Synthesis/synthesis/trunk/lib:${CONRAD_PROJECT_ROOT}/Code/Base/scimath/trunk/lib:${CONRAD_PROJECT_ROOT}/3rdParty/gsl/default/lib:${CONRAD_PROJECT_ROOT}/Code/Components/CP/mwcommon/trunk/lib:${CONRAD_PROJECT_ROOT}/3rdParty/LOFAR/Blob/default/lib:${CONRAD_PROJECT_ROOT}/3rdParty/casacore/default/lib:${CONRAD_PROJECT_ROOT}/3rdParty/cfitsio/default/lib:${CONRAD_PROJECT_ROOT}/3rdParty/lapack/default/.:${CONRAD_PROJECT_ROOT}/3rdParty/blas/default:${CONRAD_PROJECT_ROOT}/3rdParty/wcslib/default/lib:${CONRAD_PROJECT_ROOT}/3rdParty/LOFAR/APS/default/lib:${CONRAD_PROJECT_ROOT}/3rdParty/LOFAR/Common/tags/default:/usr/lib:${CONRAD_PROJECT_ROOT}/Code/Base/conrad/Cc/trunk/lib:$DYLD_LIBRARY_PATH

echo "Running csimulator"
echo time ${CONRAD_PROJECT_ROOT}/Code/Components/Synthesis/synthesis/trunk/bin/csimulator -inputs stdtest.in

echo "Running cimager"
echo time ${CONRAD_PROJECT_ROOT}/Code/Components/Synthesis/synthesis/trunk/bin/cimager -inputs stdtest.in
