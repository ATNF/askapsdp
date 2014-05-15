#!/bin/bash
#
# Pin svn:externals to the current revision for release branches
#

# Work on Code only
cd $ASKAP_ROOT/Code
# exit if not a release branch
test -n "$(svn info |grep URL | grep 'Src/releases')" ||  exit 1

IFS=$'\n'
for i in $(svn propget svn:externals -R)
do
    # ignore comments
    if [[ "$i" == *\#* ]]; then
	continue
    fi
    # ignore already pinned
    if [[ "$i" == *-r* ]]; then
	continue
    fi
    bdir=$(echo $i | awk '{print $1;}')
    tdir=$(echo $i | awk '{print $3;}')
    url=$(echo $i | awk '{print $4;}')
    rev=$(svn info "$bdir/$tdir" | grep Revision |cut -d ' ' -f2)
    prop="$tdir -r $rev $url"
    svn propset svn:externals "$prop" $bdir
done
unset IFS;
