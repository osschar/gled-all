#!/bin/sh

BUILD_DIR=build

tarballs=`ls gled-xrdmon-*.tar.gz 2>/dev/null`

if [ $? -ne 0 ]; then
    echo "Archive with Gled XrdMon not found."
    echo "Please copy the latest gled-xrdmon-YYYYMMDD.tar.gz to the current directory."
    echo "Tarballs are available at ftp://ftp.gled.org/xrdmon/"
    exit 1
fi

latest_tarball=`echo $tarballs | tail -1`

revision=`echo $latest_tarball | sed -rn 's/gled-xrdmon-slc5-(.*).tar.gz/\1/p'`

if [ -z $revision ]; then
    echo "Could not extract the revision from tarball's name '$latest_tarball'. The format is gled-xrdmon-XXXXXXXX.tar.gz, where revision is XXXXXXXX"
    exit 2
fi

sed -e "s/^\(%global rev \).*$/\1 $revision/" gled-xrdmon.spec > gled-xrdmon-current.spec

[ -d build ] && rm -rf ${BUILD_DIR}

mkdir build
for d in BUILD  RPMS  SOURCES  SPECS  SRPMS; do
    mkdir ${BUILD_DIR}/$d
done


tar -czf ${BUILD_DIR}/SOURCES/configs.tar.gz configs

# copy all regular files to the SOURCES directory

ls -1 | xargs -I {} sh -c "[ -f '{}' ] && cp '{}' ${BUILD_DIR}/SOURCES/"

rpmbuild --define "_topdir `pwd`/build" -bb gled-xrdmon-current.spec

if [ $? -eq 0 ]; then
    # clean sources directory after successfull build
    rm ${BUILD_DIR}/SOURCES/* gled-xrdmon-current.spec
fi
