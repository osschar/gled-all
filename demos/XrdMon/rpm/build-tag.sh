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
    echo "Usage: $0 [--publish] <tag> <distro>"
    echo "  . for --publish to work, user must have ssh access to gled.org"
    echo "  . tag should exist in gled svn and in external software ftp on gled.org"
    echo "  . distro is appended after version and used by rpm, too; e.g. el5 or el6"
    if [[ $help = "yes" ]]; then
        exit 0;
    else
        exit 1;
    fi
fi

tag=$1
dist=$2

# Getting root-5.34 to build on EL5 is fun.
if [[ $dist = "el5" ]]; then
    export GLED_ROOT_CONFIG_OPTS="--disable-xrootd --disable-hdfs"
fi

dir=`pwd`
ncpu=`cat /proc/cpuinfo  | grep -cP '^processor'`
svn co https://svn.gled.org/gled/trunk/gled-builder
cd gled-builder/
autoconf
./configure RELEASE=$tag CACHE_TAG=$tag SVN_DIR=tags/$tag MAKE_J_OPT="-j${ncpu}" LIBSETS="GledCore Net1 XrdMon" --prefix=${dir}/gled
make

# To make tarball:
cd ${dir}
mv gled/gled/demos/XrdMon/rpm .
(cd gled; ../rpm/cleanup-install-for-rpm.sh)
tarfile=gled-xrdmon-${tag}-${dist}.tar.gz
tar czf ${tarfile} gled/

if [[ $publish = "yes" ]]; then
    scp ${tarfile} gled.org:/u/ftp/xrdmon/
    ssh gled.org << FNORD
cd /u/ftp/xrdmon/
rm -f gled-xrdmon-current-${dist}.tar.gz
ln -s ${tarfile} gled-xrdmon-current-${dist}.tar.gz
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
