echo "c CONVERSION"
./brup2drup $2 $3
echo "c CHECKING"
./drup-check $1 $3
