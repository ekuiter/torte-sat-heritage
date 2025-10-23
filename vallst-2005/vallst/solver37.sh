#!/bin/sh

# Usage: ./vallstSAT2005.sh problemfile seed

# Repeatedly calls ./vallst to solve a boolean constraint problem.
# Like vallstlocalseed.sh but conforms to the requirements in the SAT 2005
# competitions.

#set -x

# The directory where temporary files are saved. Any model is also saved here.
tmpdir=/tmp/

# Directory where vallst is installed.
vallstdir=/solvers

# Make sure that tmpdir exists. Is this a good practice??
#mkdir -p $tmpdir 1>&2

# Time limit for subcalls in seconds. Not used.
#timeLimit=600

firstRestart=80

# Every period-th time, we will try evoking the solver with a bit different
# options. Those more rare times we will focus more on simplification than
# search.
period=4

# Number of vallst calls made, also counting the coming one.
n=1

echo "$n:" 1>&2

$vallstdir/solver37  \
    --complete-model=yes \
    --seed=$2 \
    --improve-proof-level=0 \
    --nr-of-vars-set-at-start-is-zero=yes \
    --abort-threshold=60 \
    --no-abort-time=300 \
    --out-theory-file-name=${tmpdir}solver37_sh_out_theory.vnf  \
    --changing-setting-file-name=${tmpdir}solver37_sh_changing_setting.options  \
    --result-file=${tmpdir}solver37_sh.model \
    --file=$1 1>&2
# 2>/dev/null 1>/dev/null

result=$?

# Used to store the value of n%$period (because I suck at script programming).
p=0


while [ $result -eq 2 ];
do
    #cp -p ${tmpdir}vallst_sh_out_theory.vnf ${tmpdir}vallst_sh_out_theory$n.vnf

    let n+=1
    let firstRestart+=40
    let p=n%$period

    echo "" 1>&2
    echo "$n:" 1>&2

    if [ $p -eq 0 ];
    then
        # nice \

	$vallstdir/solver37  \
	    --complete-model=yes \
	    --seed=$2 \
	    --improve-proof-level=999999 \
	    --simp-time-limit-fast=3800   --simp-time-limit-slow=100000 \
	    --simp-setting 1-1-4294901498-0   --simp-setting 2-1-400-0 \
	    --simp-setting 2-2-40-0   --simp-setting 3-1-60-0 \
	    --simp-setting 3-2-8-0    --simp-setting 3-3-6-0 \
	    --simp-setting-slow 1-1-4294901501-0 \
	    --simp-setting-slow 2-1-2200-0   --simp-setting-slow 2-2-348-0 \
	    --simp-setting-slow 3-1-200-0   --simp-setting-slow 3-2-16-0 \
	    --simp-setting-slow 3-3-7-0   --simp-setting-slow 4-1-14-0 \
	    --simp-setting-slow 4-2-6-0   --simp-setting-slow 4-3-4-0 \
	    --simp-setting-slow 4-4-3-0 \
	    --time-limit=360 \
	    --nr-of-vars-set-at-start-is-zero=no \
	    --abort-threshold=50 \
	    --no-abort-time=240 \
	    --subst-equs=no \
	    --first-restart=$firstRestart  \
	    --out-theory-file-name=${tmpdir}solver37_sh_out_theory.vnf  \
	    --changing-setting-file-name=${tmpdir}solver37_sh_changing_setting.options  \
	    --result-file=${tmpdir}solver37_sh.model \
	    --file=${tmpdir}solver37_sh_out_theory.vnf  \
	    $(head --lines=1 ${tmpdir}solver37_sh_changing_setting.options) 1>&2
# 2>/dev/null 1>/dev/null

	result=$?
    else

        # nice \

	$vallstdir/solver37  \
	    --complete-model=yes \
	    --seed=$2 \
	    --improve-proof-level=0 \
	    --nr-of-vars-set-at-start-is-zero=no \
	    --abort-threshold=50 \
	    --no-abort-time=240 \
	    --subst-equs=no \
	    --first-restart=$firstRestart  \
	    --out-theory-file-name=${tmpdir}solver37_sh_out_theory.vnf  \
	    --changing-setting-file-name=${tmpdir}solver37_sh_changing_setting.options  \
	    --result-file=${tmpdir}solver37_sh.model \
	    --file=${tmpdir}solver37_sh_out_theory.vnf  \
	    $(head --lines=1 ${tmpdir}solver37_sh_changing_setting.options) 1>&2
# 2>/dev/null 1>/dev/null

	result=$?
    fi
done


# Exit value.
satresult=0

echo "times:" 1>&2
times 1>&2

if [ $result -eq 0 ];
then
    echo "s UNSATISFIABLE"
    let satresult=20
fi

if [ $result -eq 1 ];
then
    echo "v $(cat ${tmpdir}solver37_sh.model)"
    echo "v 0"
    echo "s SATISFIABLE"
    let satresult=10
fi

exit $satresult
