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
echo ${this_file_directory}

[ -e ${this_file_directory}/root6_setup.sh ] && source ${this_file_directory}/root6_setup.sh
[ -e ${this_file_directory}/fastjet3_setup.sh ] && source ${this_file_directory}/fastjet3_setup.sh
[ -e ${this_file_directory}/pythia8_setup.sh ] && source ${this_file_directory}/pythia8_setup.sh

export FJPYTHIADIR="${this_file_directory}/.."
