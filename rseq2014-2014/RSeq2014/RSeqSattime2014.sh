#!/bin/bash

if [ "x$1" = "x" ]; then
  echo "USAGE: RSeqSattime2014.sh INSTANCE SEED cutofftime"
  exit 1
fi
ulimit -t $3
echo "c sattime $1 -seed $2 -nbsol 1 with cutoff time $3"
./sattime $1 -seed $2 -nbsol 1