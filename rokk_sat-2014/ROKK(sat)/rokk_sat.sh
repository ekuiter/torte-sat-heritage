#!/bin/bash

mypath=${0%/*}

RS=$mypath/rokk_sat

if [ $# -ge 3 ]; then
  RESULTFILE=$2"/"$3".txt"
  echo "c " $RS $1 $RESULTFILE
  $RS $1 $RESULTFILE
else
  echo "c " $RS $1 $RESULTFILE
  $RS $*
fi

exit 0
