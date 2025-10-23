#!/bin/bash

if [ "x$1" = "x" ]; then
  echo "USAGE: relback.sh <input CNF>"
  exit 1
fi


# to set in evaluation environment
#mypath=.

# To set in a normal envirnement
mypath=.
TMPDIR=$3

TMP=$TMPDIR/relback_$$ #set this to the location of temporary files
SE=$mypath/SatELite_release           #set this to the executable of SatELite
RS=$mypath/relback_static              #set this to the executable of RSat
INPUT=$1;
shift 
echo "c"
echo "c Starting SatElite Preprocessing"
echo "c"
$SE $INPUT $TMP.cnf $TMP.vmap $TMP.elim
X=$?
echo "c"
echo "c Starting relback"
echo "c"
if [ $X == 0 ]; then
  #SatElite terminated correctly
    $RS $TMP.cnf $TMP.result 
    #more $TMP.result
  X=$?
  if [ $X == 20 ]; then
    echo "s UNSATISFIABLE"
    rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result
    exit 20
    #Don't call SatElite for model extension.
  elif [ $X != 10 ]; then
    #timeout/unknown, nothing to do, just clean up and exit.
    rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result
    exit $X
  fi 
  #SATISFIABLE, call SatElite for model extension
  $SE +ext $INPUT $TMP.result $TMP.vmap $TMP.elim
  X=$?
elif [ $X == 11 ]; then
  #SatElite died, relback must take care of the rest
    echo 2- $RS $INPUT
    $RS $INPUT #but we must force relback to print out result here!!!
  X=$?
elif [ $X == 12 ]; then
  #SatElite prints out usage message
  #There is nothing to do here.
  X=0
fi

rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result
exit $X
