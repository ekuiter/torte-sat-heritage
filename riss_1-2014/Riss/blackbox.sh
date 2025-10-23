#!/bin/bash
#
# Riss BlackBox SAT Solver
# Enrique Matos Alfonso and Norbert Manthey, 2014
#
#
#
# script that first selects a configuration for the present file and executes a sat solver next
#
# usage: ./blackbox.sh <input.cnf> <tmpDir> [DRAT]
# If DRAT is specified, the solver produces a DRAT proof
#
#
# the script writes temporary files to "tmpDir" and deletes them after successful classification
#

#
# is there an external tmp directory
#
tmpDir="/tmp"
if [ "x$2" != "x" ]
then
	tmpDir=$2
fi

#
# to be adopted after training:
# (should be the weka data file used for training!)
#
features="blackboxFeatures.arff"
classifierTimeout=100   		# match with learning!
train="all"						
params="-no-clausesf -no-resf -no-xor -no-varf"  # need to match with features that are used for learning!
version="complement"
tunetimeout=3600
wekaclass="$version-$tunetimeout--$features"  # match it!
defaultconfig="PRB"												# fall back configuration
drat=""

if [ "x$3" == "xDRAT" ]
then
	wekaclass="$version-$tunetimeout-DRAT-$features"  # match it!
	defaultconfig="Riss3g"														# fall back configuration
	drat="DRAT"																				# set DRAT variable
fi


#
# Solver header
#
echo "c ======================================================="
echo "c                Riss BlackBox SAT Solver"
echo "c     Enrique Matos Alfonso and Norbert Manthey, 2014"
echo "c ======================================================="
echo "c work with models: $wekaclass "
echo "c fall back configuration: $defaultconfig"
echo "c run BlackBox SAT solver with PID $$"

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
# One binary call? ( set to 0 or 1 )
#
oneBinary=1

#
# SELECT THE CONFIGURATION
#


#version="relative"
#version="global"

classifierMemlimit=6024 # the classifier is allowed to consume up to 1G main memory
gainth=0                # there should not be any gain used

#
# select the right models for all the configurations
#

id=""


#
# point to the place where all the data is stored
#
path="." # there is no extra directory!

#
# setup path for the project
#
model="$path/model/model.$train-$wekaclass$id"
cnfinstance=$1
wekatemp="$tmpDir/wekatemp_$$.arff"
attrtmp="$tmpDir/attrInfo_$$.arff"
classifierOutput="$tmpDir/classifyClass_$$"

#
# call the classifier and check which configuration is selected
# call the classifier from the directory level below!
# make sure its only a single configuration
#

#
# one call vs two calls
#
precode=0
clasifyStartTime=`date +%s`
if [ "$oneBinary" -eq "0" ]
then
    #
    # this call computes features only and writes them to a file ready to be classified
    #
    ./classifier -no-xor -preclassify $cnfinstance -dataset="$wekatemp" -classifier="$model" -configInfo="$path/model/$version$drat-configurations.txt" -attrInfo="$attrtmp" -mem-lim=$classifierMemlimit -cpu-lim=$classifierTimeout -weka=weka-3.6.6.jar -predictor=predictor.jar $params -verb=1
    precode=$?

    #
    # if everything worked nicely, then we call the classifier to generate the configuration from the features
    #
    if [ "$precode" == "0" ]
	then
	./classifier -no-xor -classify -dataset="$wekatemp" -classifier="$model" -configInfo="$path/model/$version$drat-configurations.txt" -attrInfo="$attrtmp" -mem-lim=$classifierMemlimit -cpu-lim=$classifierTimeout -weka=weka-3.6.6.jar -predictor=predictor.jar $params -verb=1 > $classifierOutput
	code=$?
	else
	code=1
	fi 

else
    #
    # a single binary call for feature extraction and classification
    #
    ./classifier -no-xor -classify $cnfinstance -dataset="$wekatemp" -classifier="$model" -configInfo="$path/model/$version$drat-configurations.txt" -attrInfo="$attrtmp" -mem-lim=$classifierMemlimit -cpu-lim=$classifierTimeout -weka=weka-3.6.6.jar -predictor=predictor.jar $params -verb=1 > $classifierOutput
    code=$?
fi
clasifyEndTime=`date +%s`

#
# depending on the exit code of the classifier either use the predicted class,
# or use the fall back class
#
selectedConfiguration=$defaultconfig
if [ "$code" == "0" ]
then
    # evaluate exit code
    output=`cat $classifierOutput  | grep "@class" | awk '{print $2}' | tail -n 1`
    echo "c exitCode: $code prediction: $output wall-time: $(( $clasifyEndTime - $clasifyStartTime)) preCode: $precode"
    selectedConfiguration=$output
else
    echo "c exitCode: $code prediction: $defaultconfig wall-time: $(( $clasifyEndTime - $clasifyStartTime)) preCode: $precode tool-output: $output"
fi

# for debugging:
#
#echo "classifierOutput: $classifierOutput"
#echo "wekatempfile: $wekatemp attrtmpfile: $attrtmp"
#exit
#

# clean up classification files
rm -f $wekatemp $attrtmp $classifierOutput



#
#
# USE THE SELECTED CONFIGURATION
#
#

# solve the instance with the correct version of the tool
ec=0		# default exit code
if [ "x$3" != "xDRAT" ]
then
	echo "c execute SAT solver with $cnfinstance -config=$selectedConfiguration"
	./riss $cnfinstance -config=$selectedConfiguration
	# get the exit code
	ec=$?
else
	# use DRAT version
	proof=$tmpDir/drat_$$.proof 

	# use Riss with the selected configuration, and turn off variable renaming	
	echo "c execute SAT solver with $cnfinstance -config=$selectedConfiguration -drup=$proof -proofFormat=DRAT -verb-proof=0 -no-dense"
	./riss $cnfinstance -config=$selectedConfiguration -drup=$proof -proofFormat=DRAT -verb-proof=0 -no-dense -no-symm
	ec=$?
	
	# show proof if UNSAT
	if [ "$ec" -eq "20" ]
	then
		cat $proof
	fi
	
	# could remove the proof file here ...
	
fi

# return the exit code
exit $ec

