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

if [ ! -z $(which root-config) ]; then

        pred=$(root-config --prefix)
        bind=$(root-config --bindir)
        ldir=$(root-config --libdir)
        idir=$(root-config --incdir)
        root_major_version=$(root-config --version | cut -f 1 -d .)
        outscript="root${root_major_version}_setup.sh"
cat >${this_file_directory}/${outscript} <<EOL
#!/bin/bash
export ROOT6DIR=${pred}
export ROOTSYS=${pred}
export ROOT_DIR=${pred}
source ${pred}/bin/thisroot.sh
# uncomment only if needed
# export LD_LIBRARY_PATH=\${LD_LIBRARY_PATH}:${ldir}
# export DYLD_LIBRARY_PATH=\${DYLD_LIBRARY_PATH}:${ldir}
# export PATH=\${PATH}:${bind}
EOL

        rm -rf ${this_file_directory}/root_setup.sh
        ln -fs ${this_file_directory}/${outscript} ${this_file_directory}/root_setup.sh

fi
