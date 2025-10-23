#!/bin/bash
# input $1 should be a filename with a DIMACS cnf theory
# input $2 should be a location of a working folder

randomstring=`date +%s%N`
filename=$(basename "$1")
filename="${filename%.*}$randomstring"

cnf_file=$2/$filename.cnf
broken_file=$2/$filename.broken
output_file=$2/$filename.output

echo "c *** Removing duplicate clauses from source file..."
./cnfdedup $1 > $cnf_file

echo "c *** Running Shatter..."
./shatter $cnf_file > $broken_file

echo "c *** Solving the resulting cnf theory using glucose..."
./glucose $broken_file $output_file > /dev/null

echo "c *** Removing tseitin variables from certificate and formatting output..."
./removeTseitins.sh $cnf_file $output_file

echo "c *** Done :)"