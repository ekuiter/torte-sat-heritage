#!/bin/bash

if [ "x$1" = "x" ]; then
  echo "USAGE: SCSeq.sh INSTANCE -s SEED cutofftime"
  exit 1
fi
echo "c Starting sattime"
./go_sattime.sh $1 $3 $4 
X=$?
if [ $X != 10 ]; then
  echo "c Starting clasp"
./clasp $1 --seed $3 
fi

