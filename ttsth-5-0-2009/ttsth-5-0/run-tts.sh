#!/bin/sh
 
TTSP=ttsp-4-2

CHECKPRT=checkprt


#----------------------------------------------------------------------
r=$1
rr=${r%.cnf}
inputcnf=${rr}.cnf
inputprt=${rr}.prt
${TTSP} -rpt-file $inputcnf
TTSST=$?
if [ $TTSST -ne 20 ]  ; then
	exit ${TTSST} ;
fi

exit 20

${CHECKPRT}  -rpt-file $inputcnf
CHECKST=$?
rm -f $inputprt
if [ $CHECKST -ne 20 ]  ; then
	exit ${CHECKST} ;
fi
exit 20
