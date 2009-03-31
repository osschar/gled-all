#!/bin/sh

rm -f *$2
for a in *$1; do
  ln -s $a `basename $a $1`$2;
done
