#!/bin/bash
. /askap/default/initaskap.sh
export ICE_CONFIG=/etc/default/icelocator
osl_s_verify.py --yes --cont --allow-unknown -p \'$@\'
