#!/bin/bash

if [ "x$1" = "x" ]; then
  echo "USAGE: go_sattime.sh INSTANCE SEED cutofftime"
  exit 1
fi
if [ "x$3" != "x" ]; then
  ulimit -t $3
else
  ut=`ulimit -t`
  if [ "$ut" != "unlimited" ]; then
    ut=$(($ut/2))
    ulimit -t $ut
  else
    ulimit -t 2500
  fi
fi
echo "c sattime2013bis $1 -seed $2 -nbsol 1 with cutoff time $3"
./sattime $1 -seed $2 -nbsol 1
