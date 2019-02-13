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

source ${this_file_directory}/setup.sh

build_dir=${FJPYTHIADIR}/build
mkdir -p ${build_dir}
cd ${build_dir}
if [ "x${1}" == "xclean" ]; then
        make clean
else
        cmake -DCMAKE_INSTALL_PREFIX=${FJPYTHIADIR}/install -DCMAKE_BUILD_TYPE=Release ${FJPYTHIADIR}/src
        make -j && make install
fi


