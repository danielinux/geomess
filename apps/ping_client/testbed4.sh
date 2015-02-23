x=10

killall picomesh
killall geomess
killall geopcap
./geomess &>/dev/null &

for i in `seq 1 4` ; do
    ./picomesh $i $x $x 16 15 &>/dev/null &
    x=$(expr $x + 10)
done
./geopcap 1 1 pings.pcap &
./picomesh 19 0 0 15 15 1

