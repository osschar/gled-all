#!/bin/bash

MACRO=xrd_suck_udp.C

MAIL_SUBJ="Message for the action man"
MAIL_TO="mtadel@ucsd.edu mtadel@physics.ucsd.edu"

cd /opt/gled
. gled-env.sh

while true; do

    saturn $MACRO

    if [ $? -ne 0 ]; then
        mail -s "$MAIL_SUBJ" $MAIL_TO << "FNORD"
Saturn died.
FNORD
    fi

done
