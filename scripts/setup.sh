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
# echo ${this_file_directory}

[ -z ${ROOTSYS} ] && [ -e ${this_file_directory}/root_setup.sh ] && source ${this_file_directory}/root_setup.sh
[ -z ${FASTJET_DIR} ] && [ -e ${this_file_directory}/fastjet3_setup.sh ] && source ${this_file_directory}/fastjet3_setup.sh
[ -z ${PYTHIA8_DIR} ] && [ -e ${this_file_directory}/pythia8_setup.sh ] && source ${this_file_directory}/pythia8_setup.sh

export FJPYTHIADIR="${this_file_directory}/.."

installed_bin_dir="${this_file_directory}/../install/bin"
installed_lib_dir="${this_file_directory}/../install/lib"
if [ -d ${installed_bin_dir} ]; then
        if [[ ":$PATH:" == *":${installed_bin_dir}:"* ]]; then
                echo "Your path already contains ${installed_bin_dir}"
        else
                # echo "Your path is missing ${installed_bin_dir}"
                export PATH=${PATH}:${installed_bin_dir}
                export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${installed_lib_dir}
                export DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${installed_lib_dir}
        fi
fi
