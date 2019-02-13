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

function get_fj_setup()
{
        if [ -z $(which fastjet-config) ]; then
                if [ -e ${this_file_directory}/fastjet3_setup.sh ]; then
                        source ${this_file_directory}/fastjet3_setup.sh
                        if [ ! -z $(which fastjet-config) ]; then
                                echo "--with-fastjet3=$(fastjet-config --prefix)"
                        fi
                fi
        else
                echo "--with-fastjet3=$(fastjet-config --prefix)"
        fi
}

function get_root_setup()
{
        if [ -z $(which root-config) ]; then
                if [ -e ${this_file_directory}/root_setup.sh ]; then
                        source ${this_file_directory}/root_setup.sh
                        if [ ! -z $(which root-config) ]; then
                                echo "--with-root=$(root-config --prefix)"
                        fi
                fi
        else
                echo "--with-root=$(root-config --prefix)"
        fi
}

wget_command=wget
[ -z $(which wget) ] && wget_command="curl -O"
echo "wget command is: ${wget_command}"

this_file_directory=`thisdir`
echo ${this_file_directory}

pythia_directory_src="${this_file_directory}/../downloads"
mkdir ${pythia_directory_src}
cd ${pythia_directory_src}

pythia_version=8240
pythia_src_dir="pythia${pythia_version}"
pythia_tgz="${pythia_src_dir}.tgz"

if [ ! -e ${pythia_tgz} ]; then
        echo "downloading pythia..."
        ${wget_command} http://home.thep.lu.se/~torbjorn/pythia8/${pythia_tgz}
fi

if [ -d ${pythia_src_dir} ]; then
        echo "pythia src directory exists - we are at: ${PWD}"
else
        tar zxvf ${pythia_tgz}
fi

pythia_installation_directory="${this_file_directory}/../external/pythia-${pythia_version}"
if [ -d ${pythia_src_dir} ]; then
        cd ${pythia_src_dir}
        fjsetup=$(get_fj_setup)
        root6setup=$(get_root_setup)
        echo "with flags: ${fjsetup} ${root6setup}"
        ./configure --prefix=${pythia_installation_directory} ${fjsetup} ${root6setup}
        make && make install
cat>${this_file_directory}/pythia8_setup.sh << EOL
#!/bin/bash
export PYTHIA8DIR=${pythia_installation_directory}
export PATH=\${PATH}:${pythia_installation_directory}/bin
export LD_LIBRARY_PATH=\${LD_LIBRARY_PATH}:\$(pythia8-config --libdir)
export DYLD_LIBRARY_PATH=\${DYLD_LIBRARY_PATH}:\$(pythia8-config --libdir)
EOL
else
        echo "error! pythia src directory not found"
fi
