#!/bin/bash
if [ "x$1" = "x" ]; then
  echo "USAGE: glucose.sh <input CNF>"
 exit 1
fi
# to set in evaluation environment
mypath=.
# To set in a normal envirnement
mypath=.
TMP=/tmp/tinisat_$$

SE=$mypath/SatELite_release           #set this to the executable of SatELite
RS=$mypath/tinisat              #set this to the executable of RSat
INPUT=$1;
shift
echo "c"
echo "c Starting SatElite Preprocessing"
echo "c"
echo "$SE $INPUT $TMP.cnf $TMP.vmap $TMP.elim"
$SE $INPUT $TMP.cnf $TMP.vmap $TMP.elim
X=$?
echo "c"
echo "c Starting tinisat"
echo "c"
if [ $X == 0 ]; then
  #SatElite terminated correctly
    $RS $TMP.cnf -verbosity=0 $TMP.result "$@"
    #more $TMP.result
  X=$?
  if [ $X == 20 ]; then
    echo "s UNSATISFIABLE"
    rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result
    exit 20
    #Dont call SatElite for model extension.
  elif [ $X != 10 ]; then
    #timeout/unknown, nothing to do, just clean up and exit.
    rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result
    exit $X
  fi
  echo "s SATISFIABLE"
  X=10
elif [ $X == 11 ]; then
  #SatElite died, tinsat must take care of the rest
    $RS $INPUT -verbosity=0 #but we must force tinisat to print out result here!!!
  X=$?
elif [ $X == 12 ]; then
  #SatElite prints out usage message
  #There is nothing to do here.
  X=0
fi
rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result
exit $X
