x=0

killall picomesh
killall geomess
sleep 1
rm -rf /tmp/msg

./geomess &

sleep 1

./picomesh 1 $x $x 16 15 5 &
x=$(expr $x + 10)

for i in `seq 2 9` ; do
    ./picomesh $i $x $x 16 15 &>/dev/null &
    x=$(expr $x + 10)
done
    
#./picomesh 10 $x $x 16 15 5 &

#./picomesh 11 $x $x 16 15
./geopcap 85 85 test2.pcap 
#./geopcap 5 5 test2.pcap 
#./geopcap 45 45 test2.pcap 
