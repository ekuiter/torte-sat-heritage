#!/bin/bash
# input $1 should be a filename with a DIMACS cnf theory
# input $2 should be a location of a working folder
limit_saucy_soft=150
limit_saucy=180
limit_saucy_hard=200

limit_BreakID=200
mem_limit_BreakID=6000

randomstring=`date +%s%N`
filename=$(basename "$1")
filename="${filename%.*}$randomstring"

cnf_file=$2/$filename.cnf
sym_file=$2/$filename.cnf.sym
broken_file=$2/$filename.broken
output_file=$2/$filename.output
tmp_file=$2/$filename.tmp

echo "c *** Removing duplicate clauses from source file..."
./cnfdedup $1 > $cnf_file

echo "c *** Detecting symmetry using saucy..."
timeout --kill-after=$limit_saucy_hard $limit_saucy ./saucy --timeout=$limit_saucy_soft -c $cnf_file > $sym_file 2> /dev/null
# if saucy timed out, the last line is typically not finished, so needs to be removed
if [ $? -ne 0 ]; then
	touch $tmp_file
	head -n -1 $sym_file > $tmp_file
	mv $tmp_file $sym_file
fi

echo "c *** Adding symmetry breaking clauses using BreakID..."
./BreakID $cnf_file $broken_file $mem_limit_BreakID $limit_BreakID > /dev/null

echo "c *** Solving the resulting cnf theory using ntusat..."
./ntusat $broken_file $output_file > /dev/null

echo "c *** Removing tseitin variables from certificate and formatting output..."
./removeTseitins.sh $cnf_file $output_file


#echo "
rm -f $cnf_file
rm -f $sym_file
rm -f $broken_file
rm -f $output_file


