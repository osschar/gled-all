#!/bin/sh

BUILD_DIR=build

tarballs=`ls gled-xrdmon-*.tar.gz 2>/dev/null`

if [ $? -ne 0 ]; then
    echo "Archive with Gled XrdMon not found."
    echo "Please copy the latest gled-xrdmon-VERSION-DIST.tar.gz to the current directory."
    echo "Tarballs are available at ftp://ftp.gled.org/xrdmon/"
    exit 1
fi

latest_tarball=`echo $tarballs | tail -1`

version=`echo $latest_tarball | sed -rn 's/gled-xrdmon-([^-]*)-?[^-]*.tar.gz/\1/p'`
dist=`echo $latest_tarball | sed -rn 's/gled-xrdmon-[^-]*-?([^-]*).tar.gz/\1/p'`

if [ -z $version ]; then
    echo "Could not extract the version from tarball's name '$latest_tarball'. The format is gled-xrdmon-XXXXXXXX-dist.tar.gz, where version is XXXXXXXX"
    exit 2
fi

if [ -z $dist ]; then
    distopt=""
else
    distopt="--define 'dist .${dist}'"
fi

sed -e "s/_VERSION_/$version/" gled-xrdmon.spec > gled-xrdmon-current.spec

[ -d build ] && rm -rf ${BUILD_DIR}

mkdir build
for d in BUILD  RPMS  SOURCES  SPECS  SRPMS; do
    mkdir ${BUILD_DIR}/$d
done


tar -czf ${BUILD_DIR}/SOURCES/configs.tar.gz configs

# copy all regular files to the SOURCES directory

ls -1 | xargs -I {} sh -c "[ -f '{}' ] && cp '{}' ${BUILD_DIR}/SOURCES/"

rpmbuild ${distopt} --define "_topdir `pwd`/build" -bb gled-xrdmon-current.spec

if [ $? -eq 0 ]; then
    # clean sources directory after successfull build
    rm ${BUILD_DIR}/SOURCES/* gled-xrdmon-current.spec
fi
