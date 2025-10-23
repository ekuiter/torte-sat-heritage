#!/bin/bash
# SparrowToRiss, Norbert Manthey, Adrian Balint 2014
#
# solve CNF formula $1 by simplifying first with coprocessor, then run Sparrow and then run Riss 4.27 on $1 in case the formula was not solved
#
# USAGE: ./SparrowToRiss.sh <input.cnf> <seed> <tmpDir> [DRAT]
#

#
# usage
#
if [ "x$1" = "x" -o "x$2" = "x"  -o "x$3" = "x" ]; then
  echo "USAGE: ./SparrowToRiss.sh <input.cnf> <seed> <tmpDir> [DRAT]"
  exit 1
fi

echo "c"
echo "c SparrowToRiss 2014"
echo "c Adrian Balint, Norbert Manthey"
echo "c"


#
# check if the file in the first parameter exists
#
if [ ! -f $1 ]
then
  # if the input file does not exists, then abort nicely with a message
  echo "c the file does not exist: $1"
  echo "s UNKNOWN"
  exit 0
fi

#
# variables for the script
#

file=$1											# first argument is CNF instance to be solved
shift												# reduce the parameters, removed the very first one. remaining $@ parameters are arguments
seed=$1 #seed
shift
tmpDir=$1 # directory for temporary files
shift
doDRAT=$1   # produce a DRAT proof for UNSAT instances
shift

# binary of the used SAT solver
satsolver=sparrow						# name of the binary (if not in this directory, give relative path as well)

# parameters for preprocessor
cp3params="-enabled_cp3 -cp3_stats -up -subsimp -bve -no-bve_gates -no-bve_strength -bve_red_lits=1 -cp3_bve_heap=1 -bve_heap_updates=1 -bve_totalG -bve_cgrow_t=1000 -bve_cgrow=10 -ee -cp3_ee_it -unhide -cp3_uhdIters=5 -cp3_uhdEE -cp3_uhdTrans -cp3_uhdProbe=4 -cp3_uhdPrSize=3 -cp3_uhdUHLE=0"

# some temporary files 
undo=$tmpDir/cp_undo_$$				# path to temporary file that stores cp3 undo information
tmpCNF=$tmpDir/cp_tmpCNF_$$		# path to temporary file that stores cp3 simplified formula
model=$tmpDir/cp_model_$$			# path to temporary file that model of the preprocessor (stdout)
realModel=$tmpDir/model_$$			# path to temporary file that model of the SAT solver (stdout)
echo "c undo: $undo tmpCNF: $tmpCNF model: $model realModel: $realModel"

ppStart=0
ppEnd=0
solveStart=0
solveEnd=0
rissStart=0
rissEnd=0

#
# If DRAT is not used, start the usual solver
# Otherwise, skip Coprocessor and Sparrow, and start with Riss directly
#

#
# handle DRAT case
#
drup=""
if [ "x$doDRAT" != "x" ]
then
	# disable fm and laHack for level 1, because they do not support DRAT proofs
	drup="-drup=$tmpDir/drat_$$.proof -verb-proof=0 -no-dense -no-fm -no-laHack -proofFormat=DRAT"
fi

#
# run coprocessor with parameters added to this script
# and output to stdout of the preprocessor is redirected to stderr
#

exitCode=0
if [ "x$doDRAT" == "x" ]
then

	ppStart=`date +%s`
	./coprocessor $file $realModel -enabled_cp3 -undo=$undo -dimacs=$tmpCNF $cp3params 1>&2
	exitCode=$?
	ppEnd=`date +%s`
	echo "c preprocessed $(( $ppEnd - $ppStart)) seconds" 1>&2
	echo "c preprocessed $(( $ppEnd - $ppStart)) seconds with exit code $exitCode"

	# solved by preprocessing
	if [ "$exitCode" -eq "10" -o "$exitCode" -eq "20" ]
	then 
		echo "c solved by preprocessor"
		winningSolver="Coprocessor"
	else
		echo "c not solved by preprocessor -- do search"
		if [ "$exitCode" -eq "0" ]
		then
			#
			# exit code == 0 -> could not solve the instance
			# dimacs file will be printed always
			# exit code could be 10 or 20, depending on whether coprocessor could solve the instance already
			#
	
			#
			# run your favorite solver (output is expected to look like in the SAT competition, s line and v line(s) )
			# and output to stdout of the sat solver is redirected to stderr
			#
			echo "c starting sparrow solver" 1>&2
			solveStart=`date +%s`
			./$satsolver -a -l -k -r1 --timeout 900 --maxflips=500000000 $tmpCNF $seed > $model
			exitCode=$?
			solveEnd=`date +%s`
			echo "c solved $(( $solveEnd - $solveStart )) seconds" 1>&2
	
			#
			# undo the model
			# coprocessor can also handle "s UNSATISFIABLE"
			#
			echo "c post-process with coprocessor"
			./coprocessor -post -undo=$undo -model=$model > $realModel
	
			#
			# verify final output if SAT?
			#
			if [ "$exitCode" -eq "10" ]
			then
				echo "c verify model ..."
				winningSolver="sparrow"
				# ./verify SAT $realModel $file
			fi
		else
			#
			# preprocessor returned some unwanted exit code
			#
			echo "c preprocessor has been unable to solve the instance"
			#
			# run sat solver on initial instance
			# and output to stdout of the sat solver is redirected to stderr
			#
			solveStart=`date +%s`
			./$satsolver -a -l -k -r1 --timeout 900 --maxflips=500000000 $file $seed > $realModel
			exitCode=$?
			solveEnd=`date +%s`
			echo "c solved $(( $solveEnd - $solveStart )) seconds" 1>&2
		fi
	fi
else
	echo "c skipped Coprocessor and Sparrow due to required DRAT proof"
fi # end DRAT if
#
# 
#


if [ "$exitCode" -ne "10" -a "$exitCode" -ne "20" ]
then 
	echo "c use Riss 4.27" 1>&2
	# lets use Riss 4.27 for everything that could not be solved within the limits
	rissStart=`date +%s`
	#
	# use Riss
	# If DRAT should be used, the variable $drup contains the location to the proof, and disables FM in the solver (all other techniques work with DRAT)
	#
	./rissDRAT $file $realModel -R=1.2 -szLBDQueue=60 -szTrailQueue=4000 -lbdIgnL0 -quickRed -keepWorst=0.001 -var-decay-b=0.85 -var-decay-e=0.99 -var-decay-d=10000 -rnd-freq=0.005 -init-act=1 -init-pol=2 -rlevel=1 -alluiphack=2 -clsActB=2 -dontTrust -lhbr=3 -lhbr-sub -actIncMode=2 -laHack -dyn -laEEl -hlaLevel=1 -hlaevery=32 -hlabound=-1 -hlaTop=512 -sInterval=1 -learnDecP=80 -er-size=16 -er-lbd=12 -sUhdProbe=1 -no-sUhdPrRb -sUHLEsize=30 -sUHLElbd=12 -cp3_ee_bIter=400000000 -card_maxC=7 -card_max=2 -pr-uips=0 -pr-keepI=0 -no-pr-nce  -enabled_cp3 -cp3_stats -bve -bve_red_lits=1 -fm -no-cp3_fm_vMulAMO -unhide -cp3_uhdIters=5 -cp3_uhdEE -cp3_uhdTrans -bce -bce-cle -no-bce-bce $drup
	exitCode=$?
	rissEnd=`date +%s`
	echo "c Riss 4.27 used $(( $rissEnd - $rissStart)) seconds with exit code $exitCode" 1>&2
	if [ "$exitCode" -eq "10" -o "$exitCode" -eq "20" ]
	then 
		winningSolver="Riss 4.27" 
	fi
fi


#
# print times
#
echo "c pp-time: $(( $ppEnd - $ppStart)) SLS-time: $(( $solveEnd - $solveStart ))  CDCL-time: $(( $rissEnd - $rissStart))" 1>&2
echo "c solved with: $winningSolver" 1>&2

#
# print solution
#
cat $realModel

#
# print the DRAT proof, if answer is UNSAT, and if DRAT is requested
#
if [ "x$doDRAT" != "x" -a "$exitCode" -eq "20" ]
then
	cat $tmpDir/drat_$$.proof
fi
# remove the file, if it exists or not
rm -f $tmpDir/drat_$$.proof

#
# remove tmp files
#
rm -f $undo $undo.map $tmpCNF $model $realModel $ageFile $actFile

#
# return with correct exit code
#
exit $exitCode

