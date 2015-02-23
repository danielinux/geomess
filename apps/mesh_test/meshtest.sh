x=10

killall picomesh
killall geomess
sleep 1
rm -rf /tmp/msg

./geomess &

sleep 1
./geopcap 5 5 test2.pcap &

for i in `seq 2 8` ; do
    ./picomesh $i $x $x 16 15 &>/dev/null &
    x=$(expr $x + 10)
done
./picomesh 1 0 0 16 15 7
RET=$?

killall picomesh
killall geopcap 
killall geomess
exit $RET
    
#./picomesh 10 $x $x 16 15 5 &

#./picomesh 11 $x $x 16 15
#./geopcap 85 85 test2.pcap 
#./geopcap 45 45 test2.pcap 
