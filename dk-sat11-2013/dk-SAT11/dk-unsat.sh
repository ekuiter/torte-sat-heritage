size=`cat $1 | grep -v "c " | awk 'BEGIN{max=4} / 0/ {if (NF > max) max = NF} END{print max}'`
echo "c size" $(($size - 1))
if [ $size == 4 ]
then
  echo "c running 3-SAT solver"
  cat $1 | grep -v "c" | sed 's| 0||' | sed 's|-|~|g' | ./dk-3-look x$2
else
  echo "c running k-SAT solver"
  cat $1 | grep -v "c" | sed 's| 0||' | sed 's|-|~|g' | ./dk-k-look x$2
fi 
tail=`tail -n 1 $2 | awk '{print $1}'` 
if [ $tail == 0 ]
then
  echo "o proof BDRUP" 
  cat $2
fi
