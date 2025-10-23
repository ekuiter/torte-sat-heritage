#!/bin/bash

# Extract the original number of literals
maxlit=0
word_nr=0
while read line; do      
	if [[ $line == p* ]]; then
			for word in $line; do
				word_nr=`expr $word_nr + 1`
				if [ $word_nr -eq 3 ]; then
					maxlit=$word
				fi
			done
		fi
	if [ $maxlit -ne 0 ]; then
		break
	fi
done < $1

# Read output
line_nr=1
firstline=""
secondline=""
while read line; do
	if [ $line_nr -eq 1 ]; then
		line_nr=2
		firstline=$line
	elif [ $line_nr -eq 2 ]; then
		line_nr=3
		secondline=$line
	fi
done < $2

# echo result
if [[ "$firstline" == *UNS* ]]; then
	echo "s UNSATISFIABLE"
elif [[ "$firstline" == *SAT* ]]; then
	if [[ ${#secondline} -le 3 ]]; then
		# Avoid false positives
		echo "s UNKNOWN"
	else
		echo "s SATISFIABLE"
		echo -n "v "
		for word in $secondline; do
			# Remove any tseitin literals introduced by the symmetry breaking preprocessor
			if [ ${word#-} -le $maxlit ]; then  # the stuf between accolades takes the absolute value
				echo -n "$word "
			fi
		done
		echo ""
	fi
else
	echo "s UNKNOWN"
fi

