#!/bin/bash

# Args: [--publish] tag os

if [[ $1 = "--help" || $1 = "-h" ]]; then
  help=yes
  shift
fi

if [[ $1 = "--publish" ]]; then
  publish=yes
  shift
fi

if [[ $# != 2 || $help = "yes" ]]; then
    echo "Usage: $0 [--publish] tag os"
    if [[ $help = "yes" ]]; then
        exit 0;
    else
        exit 1;
    fi
fi

tag=$1
os=$2


echo "publish $publish, tag '$tag', os '$os'"

exit 0;

dir=`pwd`
ncpu=`cat /proc/cpuinfo  | grep -cP '^processor'`
svn co https://svn.gled.org/gled/trunk/gled-builder
cd gled-builder/
autoconf
./configure-tag.sh $tag MAKE_J_OPT="-j${ncpu}" LIBSETS="GledCore Net1 XrdMon" --prefix=${dir}/gled
make

# To make tarball:
cd ${dir}
mv gled/gled/demos/XrdMon/rpm .
(cd gled; ../rpm/cleanup-install-for-rpm.sh)
tarfile=gled-xrdmon-${tag}-${os}.tar.gz
tar czf ${tarfile} gled/

if [[ $publish = "yes" ]]; then
    scp ${tarfile} gled.org:/u/ftp/xrdmon/
    ssh gled.org << FNORD
cd /u/ftp/xrdmon/
rm -f gled-xrdmon-current-${os}.tar.gz
ln -s ${tarfile} gled-xrdmon-current-${os}.tar.gz
FNORD
fi

# To make RPM:
cd rpm
ln -s ../${tarfile} .
./make-rpm.sh
rpmdir=build/RPMS/x86_64
rpm=`ls ${rpmdir}`

if [[ $publish = "yes" ]]; then
    scp ${rpmdir}/${rpm} gled.org:/u/ftp/xrdmon/
fi
