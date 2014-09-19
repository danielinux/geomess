x=0

killall picomesh
killall geomess
./geomess &

for i in `seq 1 10` ; do
    ./picomesh $i $x $x 16 15 &>/dev/null &
    x=$(expr $x + 10)
done
    

./picomesh 11 $x $x 16 15
