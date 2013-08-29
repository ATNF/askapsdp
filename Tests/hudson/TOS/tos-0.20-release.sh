#!/bin/bash -l
#
# Repository URL
# https://svn.atnf.csiro.au/askapsoft/Src/releases/TOS-0.20
#
# Slaves - Node/Label - aksc01, aktos01, mtos1, mtos2, ptbdr1
#

# Bootstrap
unset ASKAP_ROOT
cd $WORKSPACE/TOS-0.20
SVNREV=`svn info |grep 'Changed Rev'|awk '{print $NF;}'`
REV=0.20_r${SVNREV}
if [ ! -f initaskap.sh ]
then
    /usr/bin/python2.6 bootstrap.py
    if [ $? -ne 0 ]; then
       exit 1
    fi
fi
# Setup ASKAP environment
source initaskap.sh
TOSNAME="TOS$REV"
OUTDIR="/var/tmp/tos-releases/"
trap 'rm -rf /tmp/release-* /tmp/TOS*' EXIT

if [ -f "$OUTDIR/$TOSNAME/$TOSNAME.tgz" ]
then
   echo "$TOSNAME built already"
   exit 0
fi
# clean Code
#rbuild -S -t clean Code/Components/EPICS

#Build TOS
cd Code/Systems/TOS

rbuild -S -t release --release-name=$TOSNAME
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi
mkdir -p /var/tmp/tos-releases/$TOSNAME
rfiles="install_release.sh fabfile.py $TOSNAME.tgz"
for f in $rfiles
do
    if [ ! -f $f ]; then exit 1;fi
    cp $f $OUTDIR/$TOSNAME/
done

# build TOS-CSS
cd $ASKAP_ROOT/Code/Systems/TOS-CSS
CSSNAME="TOS-CSS$REV"
rbuild -n -t release --release-name=$CSSNAME
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

mkdir -p /var/tmp/tos-releases/$CSSNAME/
rfiles="install_release.sh fabfile.py $CSSNAME.tgz"
for f in $rfiles
do
    if [ ! -f $f ]; then exit 1;fi
    cp $f $OUTDIR/$CSSNAME/
done

#clean up left overs
RTMP="/tmp/release-${SVNREV}"
if [ -e $RTMP ]
then
    rm -rf $RTMP
fi
