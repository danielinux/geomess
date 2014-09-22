x=10

killall picomesh
killall geomess
killall geopcap
./geomess &>/dev/null &
./geopcap 0 0 pcapfile.pcap &

for i in `seq 1 9` ; do
    ./picomesh $i $x $x 16 15 &>/dev/null &
    x=$(expr $x + 10)
    #Create pcap file for each node in the network
    ./geopcap $x $x $i.pcap &
done

# picomesh node that is sending ping to node in routing table with
# provided metric as last argument
#./picomesh 19 0 0 15 15 8
