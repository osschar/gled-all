#!/bin/bash

MAIL_SUBJ="Message for the action man"
MAIL_TO="mtadel@ucsd.edu mtadel@physics.ucsd.edu"

if [ $# -lt 1 ]; then
   echo $0 requires at least one argument, a script to execute.
   echo All arguments are just passed to saturn untouched.
   exit 1
fi

args=$*

cd /opt/gled
. gled-env.sh

# I used to run a loop like this within screen.
# It actually works amazingly well.
# while true; do

    saturn $args
    es=$?
    if [ $es -ne 0 ]; then
        now=`date`
        mail -s "$MAIL_SUBJ" $MAIL_TO << FNORD
saturn $args
Died on $now with exit status $es.
FNORD
    fi

# done
