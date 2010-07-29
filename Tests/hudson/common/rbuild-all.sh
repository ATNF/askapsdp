#!/bin/bash -l

PATH=/usr/bin:/bin:/usr/sbin:/sbin

ECH0='' # Can be overidden by -n option.
ARGS=''

PNAME=`basename $0`
SDATE=`date`
FDATE=`date +"%y%m%d"`

function HelpMessage
{
    printf "\n${PNAME} [-hnq] [<targets>]\n"
    printf "\t-h\thelp\n"
    printf "\t-n\tno execute (debugging)\n"
    printf "\t-q\tbuild Code quietly\n"
    printf "\t<targets> whitespace separated list of ASKAPsoft Code targets\n"
    printf "\t\te.g. doc, test, functest\n"
    printf "\n"
}


function bootstrap
{
    ${ECHO} unset ASKAP_ROOT
    ${ECHO} cd $WORKSPACE/trunk
    ${ECHO} /usr/bin/python2.6 bootstrap.py
    if [ $? -ne 0 ]; then
        exit 1
    fi
}


function build
{
    directory=$1
    args=$2
    target=$3
    ${ECHO} cd ${ASKAP_ROOT}/${directory}
    ${ECHO} rbuild -a -M -S -T ${args} -t ${target}
    if [ $? -ne 0 ]; then
        exit 1
    fi
}


#
# Get options
#


while getopts hnq OPT
do
    case $OPT in
        h)  HelpMessage
            ;;
        n)  ECHO="echo"
            WORKSPACE="WORKSPACE"
            ASKAP_ROOT="ASKAP_ROOT"
            ;;
        q)  ARGS+=" -q"
            ;;
        ?)  HelpMessage
            exit 1
    esac
done

shift $(($OPTIND - 1))
TARGETS=$@
NENV=$#

#
# Main
#

# Basic build
printf "==> Running ${PNAME}\n"
printf "==> Starting standard build at ${SDATE}\n\n"
bootstrap
${ECHO} source initaskap.sh # Setup ASKAP environment.
build "3rdParty" "-q" "install"  # Always be quiet and fail immediately.
build "Code" "${ARGS}" "install"
DATE=`date`
printf "\n==> Finished standard build at ${DATE}\n"

# Additional targets
for target in ${TARGETS};
do
    DATE=`date`
    printf "\n==> Starting ${target} target at ${DATE}\n\n"
    build "Code" "" ${target}
done

DATE=`date`
printf "\n==> Finished ${PNAME} at ${DATE}\n"
