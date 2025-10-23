#!/bin/bash

if [ "x$1" = "x" ]; then
  echo "USAGE: RSeq2014.sh INSTANCE -s SEED cutofftime"
  exit 1
fi
echo "c Starting sattime"
./RSeqSattime2014.sh $1 $3 $4
X=$?
if [ $X != 10 ]; then
  echo "c Starting Relback"
  ./relback_static $1
  exit $X
fi



