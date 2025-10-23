#
# use riss to produce a drup proof
#

tmpDrup=$2/proof_$$

./riss3gSimp -hlaevery=1 -hlaLevel=5 -laHack -tabu -hlabound=4096 $1 -drup=$tmpDrup
exitCode=$?

if [ "$exitCode" -eq "20" ]
then
  cat $tmpDrup
fi

rm -f $tmpDrup
exit $exitCode
