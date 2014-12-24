#!/bin/sh

instdir=/opt/gled

cp logrotate.d/gled-xrdmon /etc/logrotate.d
mkdir -p /var/log/gled

cp xrdmon-wrapper.sh $instdir
cp xrd_*.C $instdir
