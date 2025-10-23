#!/bin/bash

if [ "x$1" = "x" ]; then
  echo "USAGE: $0 <input CNF> [<temp dir>]"
  echo "If <temp dir> is not given, it will use '/tmp/gluH_\$RANDOM'."
  exit 1
fi

TEMPDIR=$2
if [ "x$TEMPDIR" = "x" ]; then
    echo "c \$<temp dir> not set!"
    echo "c Will use '/tmp/gluH_\$RANDOM'."
    TEMPDIR=/tmp/gluH_$RANDOM
fi
mkdir -p $TEMPDIR
EXEDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

TMP=$TEMPDIR/gluH_$$
SE=$EXEDIR/SatELite_release
RS=$EXEDIR/gluH_static
INPUT=$1
shift

echo "c"
echo "c Starting SatELite Preprocessing"
echo "c"
echo "c $SE $INPUT $TMP.cnf $TMP.vmap $TMP.elim"
$SE $INPUT $TMP.cnf $TMP.vmap $TMP.elim
X=$?
echo "c"
echo "c Starting gluH (hacked addition to glucose 2.1)"
echo "c"

if [ $X == 0 ]; then # SatELite terminated correctly.

  $RS $TMP.cnf $TMP.result "$@" 
  X=$?
  if [ $X == 20 ]; then
    echo "s UNSATISFIABLE"
  elif [ $X == 10 ]; then
    # SATISFIABLE; call SatELite for model extension.
    #echo "s SATISFIABLE"
    $SE +ext $INPUT $TMP.result $TMP.vmap $TMP.elim "$@"
  fi

elif [ $X == 11 ]; then # SatELite died; gluH must take care of the rest.
  $RS $INPUT
fi

rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result
