#!/bin/bash

# This is a wrapper created solely for the SAT Competition 2014.

if [ -z $1 ]; then
  echo "USAGE: $0 <input CNF> [--DRUP]"
  exit 1
fi

EXEDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
if [ -z $EXEDIR ]; then EXEDIR="."; fi

RS=$EXEDIR/minisat_HACK_999ED_static

echo "c Binary: $RS"
echo "c Input: $1"

if [ ! -x $RS ]; then
    echo "c Cannot execute $RS!"
fi

if [ "x$2" = "x--DRUP" ]; then
    exec $RS $1 DRUP
else
    exec $RS $1
fi
