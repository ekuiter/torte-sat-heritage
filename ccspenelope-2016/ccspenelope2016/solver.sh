#!/bin/bash
timeLim=5000
args="-rnd-phase -no-clean-exit"
solver=ccs_penelope

if [ ! -f ./$solver ]; then
  echo "Building CBPRPeneLoPe"
  make CONF=Release > /dev/null || exit 0
fi

if [ "x$1" = "x" ]; then
  echo "USAGE: solver.sh <input CNF>"
  exit 1
fi

# to set in evaluation environment
mypath="$(dirname "$(readlink -f ${BASH_SOURCE[0]})")"

# To set in a normal envirnement
if [ -z "$TEMPDIR" ]; then
    echo "c setting TEMPDIR to /tmp"
    TEMPDIR=`pwd`/tmp
    mkdir -p $TEMPDIR
fi


TMP=$TEMPDIR/cbpr_penelope_$$ #set this to the location of temporary files
Satelite=./SatELite_release           #set this to the executable of SatELite
INPUT=$1;

echo "c"
echo "c Starting SatElite Preprocessing"
echo "c"
SatOutput=`$Satelite $INPUT $TMP.cnf $TMP.vmap $TMP.elim`
X=$?
echo "$SatOutput"

#if Satelite found the answer, we may stop now
if [ $X == 20 ]
then
  exit $X
fi


echo "c"
echo "c Starting the solver"
echo "c"
echo "c "$args
if [ $X == 0 ]; then

  emptyResult=`more $TMP.cnf | grep "p cnf 0 0"`  
  if [ "$emptyResult" == "p cnf 0 0" ]; then
    ./$solver $INPUT $args "-force-print"
    X=$?
    exit $X
  fi
  #SatElite terminated correctly

  cp $TMP.cnf temp.cnf
  ./$solver $TMP.cnf $args $TMP.result
  exitCode=$?
  if [ $exitCode == 20 ]; then
    echo "s UNSATISFIABLE"
    rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result
    exit 20
    #Don't call SatElite for model extension.
  elif [ $exitCode != 10 ]; then
    #timeout/unknown, nothing to do, just clean up and exit.
    rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result
    exit $exitCode
  fi

  #SATISFIABLE, call SatElite for model extension
  echo "c calling SatElite for model extension of " $INPUT
  $Satelite +ext $INPUT $TMP.result $TMP.vmap $TMP.elim 
  X=$?
elif [ $X == 11 ]; then
  #SatElite died, cbpr_penelope must take care of the rest
  ./$solver $INPUT $args "-force-print" #but we must force glucose to print out result here!!!
  X=$?
elif [ $X == 12 ]; then
  #SatElite prints out usage message
  #There is nothing to do here.
  X=0
else
  echo "unknown return: "$X
fi

rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result
exit $X
