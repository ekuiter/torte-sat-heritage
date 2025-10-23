#!/bin/bash

if [ "x$1" = "x" ]; then
	echo "c Usage: ./CCAnr+glucose.sh <instance> <seed> <sls_cutoff_time>"
	exit 1
fi


sls_cutoff=$3

echo "c start CCAnr"
./CCAnr $1 $2 ${sls_cutoff}
R=$?

if [ $R != 0 ]; then
	echo "c start glucose"
	./glucose $1
fi

