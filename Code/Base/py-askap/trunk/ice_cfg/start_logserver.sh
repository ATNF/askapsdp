#!/bin/bash

export DYLD_LIBRARY_PATH=${ASKAP_ROOT}/3rdParty/Ice/tags/Ice-3.3.0/install/lib/
export PATH=${ASKAP_ROOT}/3rdParty/Ice/tags/Ice-3.3.0/install/bin:${PATH}
icebox --Ice.Config=config.icebox
