#!/bin/bash

function abspath()
{
        case "${1}" in
                [./]*)
                [ ! -d ${1%/*} ] && echo "${1}" && return
                echo "$(cd ${1%/*}; pwd)/${1##*/}"
                ;;
                *)
                echo "${PWD}/${1}"
                ;;
        esac
}

function thisdir()
{
        THISFILE=`abspath $BASH_SOURCE`
        XDIR=`dirname $THISFILE`
        if [ -L ${THISFILE} ];
        then
                target=`readlink $THISFILE`
                XDIR=`dirname $target`
        fi

        THISDIR=$XDIR
        echo $THISDIR
}

this_file_directory=`thisdir`

source ${this_file_directory}/setup.sh

export PATH=${PATH}:${FJPYTHIADIR}/install/bin
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${FJPYTHIADIR}/install/lib
export DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${FJPYTHIADIR}/install/lib

fjpythia_youqi_exe --fjroot $@
