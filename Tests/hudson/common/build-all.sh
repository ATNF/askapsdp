#!/bin/bash -l

function bootstrap
{
    unset ASKAP_ROOT
    cd $WORKSPACE/trunk
    /usr/bin/python2.6 bootstrap.py
    if [ $? -ne 0 ]; then
        exit 1
    fi
}


function build_3rdParty
{
    cd $ASKAP_ROOT/3rdParty
    python autobuild.py -q -x install
    if [ $? -ne 0 ]; then
        exit 1
    fi
}

function build_Code
{
    cd $ASKAP_ROOT/Code
    python autobuild.py -q -x install
    if [ $? -ne 0 ]; then
        exit 1
    fi
}


function build_Code_doc
{
    cd $ASKAP_ROOT/Code
    python autobuild.py -q doc
    if [ $? -ne 0 ]; then
        exit 1
    fi
}

# Run all tests in Code - should not need exit status
function test_Code
{
    cd $ASKAP_ROOT/Code
    python autobuild.py -q test
}

#
# main
#

bootstrap
source initaskap.sh # Setup ASKAP environment.
build_3rdParty
build_Code
