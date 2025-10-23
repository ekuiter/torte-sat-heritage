#!/bin/bash

# Top-level script for manysat 

if [ "x$1" = "x" ]; then
  echo "USAGE: manysat.sh <input CNF>"
  exit 1
fi

mypath=$PWD 
TMP=$mypath/$$_many  #set this to the location of temporary files
SE=$mypath/SatElite           #set this to the executable of SatELite
MS=$mypath/manysat_static               #set this to the executable 
INPUT=$1;
shift 

$SE $INPUT $TMP.cnf $TMP.vmap $TMP.elim


X=$?

if [ $X == 0 ]; then
  #SatElite terminated correctly

  $MS -verbosity=0 $TMP.cnf  $TMP.result "$@" 
   
    #more $TMP.result
  X=$?

  if [ $X == 20 ]; then
    #manysat must not print out result!
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
  $SE +ext $INPUT $TMP.result $TMP.vmap $TMP.elim  "$@" -s
  X=$?
elif [ $X == 11 ]; then
  #SatElite died, manysat must take care of the rest

 $MS $INPUT "$@" #but we must force manysat to print out result here!!!
  X=$?
elif [ $X == 12 ]; then
  #SatElite prints out usage message
  #There is nothing to do here.
  X=0
fi

#rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result
exit $X
