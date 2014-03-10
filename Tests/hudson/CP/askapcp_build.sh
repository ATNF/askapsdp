#!/bin/bash -l

#
# Make a directory to contain the artifacts
#
ARTIFACTS_DIR=$WORKSPACE/artifacts
mkdir -p $ARTIFACTS_DIR
if [ $? -ne 0 ]; then
    echo "Error: failed to make artifacts directory"
    exit 1
fi

#
# Bootstrap
#
unset ASKAP_ROOT
cd $WORKSPACE/trunk
nice python2.7 bootstrap.py -n
if [ $? -ne 0 ]; then
    echo "Error: Bootstrapping failed"
    exit 1
fi

#
# Init ASKAP environment
#
source initaskap.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current
source initaskap.sh

cd Code/Systems/cpapps

#
# Build CP applications
#
nice rbuild -a -M -S -T
if [ $? -ne 0 ]; then
    echo "Error: rbuild -a -M -S -T"
    exit 1
fi

#
# Build a release tarball
#
nice rbuild -n -M -S -T -t release 
if [ $? -ne 0 ]; then
    echo "Error: rbuild -n -M -S -T -t release"
    exit 1
fi

# Need a "standard name" for the artifact
mv release-*.tgz cpapps-release.tgz
mv cpapps-release.tgz $ARTIFACTS_DIR

#
# Build user documentation
#
cd $WORKSPACE/trunk/Code/Components/CP/docs/current
nice rbuild -n -M -S -T -t doc
if [ $? -ne 0 ]; then
    echo "Error: rbuild -n -M -S -T -t doc failed"
    exit 1
fi
cd doc/_build
nice tar zcvf ${ARTIFACTS_DIR}/cp-docs.tgz html
if [ $? -ne 0 ]; then
    echo "Error: tar failed"
    exit 1
fi

#
# Build admin documentation
#
cd $WORKSPACE/trunk/Code/Components/CP/docs_admin/current
nice rbuild -n -M -S -T -t doc
if [ $? -ne 0 ]; then
    echo "Error: rbuild -n -M -S -T -t doc failed"
    exit 1
fi
cd doc/_build
nice tar zcvf ${ARTIFACTS_DIR}/cp-docs-admin.tgz html
if [ $? -ne 0 ]; then
    echo "Error: tar failed"
    exit 1
fi
