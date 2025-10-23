#!/bin/bash
FILE=$2
TMPDIR=$1
java -DUNSATPROOF=$TMPDIR/proof.txt -Xms5g -Xmx5g -jar sat4j2013.jar UNSAT $FILE
rm -f $TMPDIR/proof.txt
