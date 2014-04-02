#!/bin/bash -l

PATH=/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin

ECH0='' # Can be overidden by -n option.
ARGS=''
PROCESSES='j=2'
SRCDIR='trunk'

LINUX_PYTHON='/usr/bin/python2.6'
#MACOSX_PYTHON='/opt/local/bin/python2.6'
MACOSX_PYTHON='/usr/bin/python2.7'
WHEEZY_PYTHON='/usr/bin/python2.7'

PNAME=`basename $0`
SDATE=`date`
FDATE=`date +"%y%m%d"`

function HelpMessage
{
    printf "\n${PNAME} [-hnq] [-j6] [<targets>]\n"
    printf "\t-d <srcdir>\te.g. -d TOS-0.21 (default='trunk') \n"
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
    ${ECHO} cd ${WORKSPACE}/${SRCDIR}
    ${ECHO} ${PYTHON} bootstrap.py
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
    ${ECHO} rbuild -a -M -S -T -p ${PROCESSES}  ${args} -t ${target}
    if [ $? -ne 0 ]; then
        exit 1
    fi
}

function builddev
{
    target=$1
    ${ECHO} cd ${ASKAP_ROOT}/Tools/Dev/rbuild
    ${ECHO} python setup.py ${target}
    ${ECHO} cd ${ASKAP_ROOT}/Tools/Dev/templates
    ${ECHO} python setup.py ${target}
}

#
# Get options
#


while getopts d:hj:nq OPT
do
    case $OPT in
        d)  SRCDIR="$OPTARG"
            ;;
        h)  HelpMessage
            ;;
        j)  PROCESSES="j=$OPTARG"
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

if [ `uname` == "Darwin" ]; then
    PYTHON=${MACOSX_PYTHON}
else
    PYTHON=${LINUX_PYTHON}
    grep '7.' /etc/debian_version >/dev/null 2>&1
    if [ $? -eq 0 ]; then
	PYTHON=${WHEEZY_PYTHON}
    fi
fi

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
    builddev         ${target}
    build "Tools" "" ${target}
    build "Code"  "" ${target}
done

DATE=`date`
printf "\n==> Finished ${PNAME} at ${DATE}\n"
