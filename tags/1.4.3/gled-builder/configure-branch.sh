#!/bin/bash

# Runs configure script with arguments appropriate for building of a
# given brach.
#
# NOTE: It is assumed that rsync server with external packages has a
# directory with the provided branch name.

if [ $# -le 1 ]; then
  echo "Usage: $0 <branch> [other options and variable assignments]"
  exit 1
fi

if [ ! -e "configure" ]; then
  echo "configure script not found, try running autoconf."
  exit 1
fi

BRANCH=$1
shift

./configure RELEASE=$BRANCH-branch CACHE_TAG=$BRANCH SVN_DIR=branches/$BRANCH $@
