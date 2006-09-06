#!/bin/sh

AXIS_LIB=$GLEDSYS/AliEnViz/java
LIB_JARS=`ls -1 $AXIS_LIB/*.jar`
LIST=$AXIS_LIB:$CLASSPATH
for lib in $LIB_JARS; do
    LIST=${LIST}:${1}${lib}
done

echo $LIST
