#!/bin/bash

if [ "x$1" = "x" ]; then
	echo "USAGE: hyperplane_minisat.sh <input CNF>"
	exit 1
fi

# to set in evaluation environment
#mypath=.

# To set in a no#rmal envirnement
mypath=.
TMPDIR=/tmp

TMP=$TMPDIR/hpms_$$ #set this to the location of temporary files
SE=$mypath/SatELite_release           #set this to the executable of SatELite
RS=$mypath/minisat_static              #set this to the executable of minisat
BH=$mypath/best_hyperplane              #set this to the executable of hyperplane reducer
INPUT=$1;
shift 
echo "c"
echo "c Starting SatElite Preprocessing $TMP"
echo "c"
$SE $INPUT $TMP.cnf $TMP.vmap $TMP.elim
X=$?
if [ $X == 0 ]; then
	#SatElite te#rminated correctly
	#Run Hyperplane reduction
	echo "c"
	echo "c Starting hyperplane reduction"
	echo "c"
	$BH $TMP.cnf 10 2>> $TMP.err
	X=$?
	if [ $X == 30 ]; then
		#Hyperplane reduction unsat or indet, run minisat on original
		#rm -f $TMP.pre.cnf $TMP.pre.vmap $TMP.pre.elim $TMP.pre.result $TMP.cnf.1
		echo "c"
		echo "c Starting minisat on original reduction"
		echo "c"
		$RS $TMP.cnf $TMP.result "$@" > $TMP.null
		X=$?
		if [ $X == 20 ]; then
			echo "s UNSATISFIABLE"
			#rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result $TMP.null
			exit 20
			#Don't call SatElite for model extension.
		elif [ $X != 10 ]; then
			#timeout/unknown, nothing to do, just clean up and exit.
			#rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result $TMP.null
			exit $X
		fi 
		#SATISFIABLE, call SatElite for model extension
		$SE +ext $INPUT $TMP.result $TMP.vmap $TMP.elim  "$@" 
		#rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result $TMP.null
		X=$?
		exit 10
	fi
	$SE $TMP.cnf.1 $TMP.pre.cnf $TMP.pre.vmap $TMP.pre.elim 2>> $TMP.err
	X=$?
	echo "c"
	echo "c SE returned $X...Starting minisat on hyperplane reduction for 10 minutes"
	echo "c"
	# Run minisat on hyperplane reduction for 10 minutes
	echo "c $RS -cpu-lim=600 $TMP.pre.cnf $TMP.pre.result"
	$RS -cpu-lim=600 $TMP.pre.cnf $TMP.pre.result "$@" > $TMP.null
	X=$?
	echo "c Minisat returned $X"
	if [ $X != 10 ]; then
		#Hyperplane reduction unsat or indet, run minisat on original
		#rm -f $TMP.pre.cnf $TMP.pre.vmap $TMP.pre.elim $TMP.pre.result $TMP.cnf.1
		echo "c"
		echo "c Starting minisat on original reduction"
		echo "c"
		$RS $TMP.cnf $TMP.result "$@" > $TMP.null
		X=$?
		if [ $X == 20 ]; then
			echo "s UNSATISFIABLE"
			#rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result $TMP.null
			exit 20
			#Don't call SatElite for model extension.
		elif [ $X != 10 ]; then
			#timeout/unknown, nothing to do, just clean up and exit.
			#rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result $TMP.null
			exit $X
		fi 
		#SATISFIABLE, call SatElite for model extension
		$SE +ext $INPUT $TMP.result $TMP.vmap $TMP.elim  "$@"
		#rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result $TMP.null
		X=$?
		exit 10
	fi
	#Hyperplane reduction is SATISFIABLE, extend model 
	echo "c $SE +ext $TMP.cnf.1 $TMP.pre.result $TMP.pre.vmap $TMP.pre.elim"
	$SE +ext $TMP.cnf.1 $TMP.pre.result $TMP.pre.vmap $TMP.pre.elim "$@" > $TMP.hp
	cut -c3- $TMP.hp | sed '1d' | sed "1iSAT" > $TMP.result
	echo "c $SE --verbosity=0 +ext $INPUT $TMP.result $TMP.vmap $TMP.elim"
	$SE --verbosity=0 +ext $INPUT $TMP.result $TMP.vmap $TMP.elim "$@"
	#rm -f $TMP.pre.cnf $TMP.pre.vmap $TMP.pre.elim $TMP.pre.result $TMP.hp $TMP.result $TMP.null
elif [ $X == 11 ]; then
	#SatElite died, glucose must take care of the rest
	echo "c"
	echo "c Starting minisat on original problem"
	echo "c"
	$RS $INPUT
	X=$?
elif [ $X == 12 ]; then
	#SatElite prints out usage message
	#There is nothing to do here.
	X=0
fi

#rm -f $TMP.cnf $TMP.vmap $TMP.elim $TMP.result $TMP.cnf.1
exit $X
