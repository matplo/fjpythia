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

wget_command=wget
[ -z $(which wget) ] && wget_command="curl -O"
echo "wget command is: ${wget_command}"

this_file_directory=`thisdir`
echo ${this_file_directory}

cd ${this_file_directory}

fastjet_directory_src="${this_file_directory}/../downloads"
mkdir ${fastjet_directory_src}
cd ${fastjet_directory_src}

fastjet_version="3.3.2"
fastjet_src_dir="fastjet-${fastjet_version}"
fastjet_tgz="fastjet-${fastjet_version}.tar.gz"

if [ ! -e ${fastjet_tgz} ]; then
        echo "downloading fj..."
        ${wget_command} http://fastjet.fr/repo/${fastjet_tgz}
fi

if [ -d ${fastjet_src_dir} ]; then
        echo "fj src directory exists - we are at: ${PWD}"
else
        tar zxvf ${fastjet_tgz}
fi

fastjet_installation_directory="${this_file_directory}/../external/fastjet-${fastjet_version}"
if [ -d ${fastjet_src_dir} ]; then
        cd ${fastjet_src_dir}
        ./configure --prefix=${fastjet_installation_directory}
        make -j && make install
        echo "#!/bin/bash" > ${this_file_directory}/fastjet_setup.sh
        echo "export FASTJETDIR=${fastjet_installation_directory}" >> ${this_file_directory}/fastjet_setup.sh
        echo "export PATH=${PATH}:${fastjet_installation_directory}/bin" >> ${this_file_directory}/fastjet_setup.sh
else
        echo "error! fj src directory not found"
fi

# now fj contrib

cd ${this_file_directory}

fastjet_contrib_version="1.041"
fastjet_contrib_src_dir="fjcontrib-${fastjet_contrib_version}"
fastjet_contrib_tgz="fjcontrib-${fastjet_contrib_version}.tar.gz"

cd ${fastjet_directory_src}
if [ ! -e ${fastjet_contrib_tgz} ]; then
        echo "downloading fj conributed algorithms..."
        wget http://fastjet.hepforge.org/contrib/downloads/${fastjet_contrib_tgz}
fi

if [ -d ${fastjet_contrib_src_dir} ]; then
        echo "fj contrib src directory exists - we are at: ${PWD}"
else
        tar zxvf ${fastjet_contrib_tgz}
fi

if [ -d ${fastjet_contrib_src_dir} ]; then
        cd ${fastjet_contrib_src_dir}
        ./configure --fastjet-config=${fastjet_installation_directory}/bin/fastjet-config
        make -j && make check && make install
else
        echo "error! fj src directory not found"
fi
