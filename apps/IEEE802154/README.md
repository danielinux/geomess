## 6LoWPAN Simulator
---------------

### Introduction
This application simulates a 6LoWPAN-network based on picoTCP's 6LoWPAN-implementation and Geomess. It contains a device-driver that simulates a IEEE802.15.4-capable device that uses Geomess as the medium, and some scripts to initialise nodes on the 6LoWPAN-network.

picoTCP's 6LoWPAN implementation uses a Mesh-Under topology which means there are only **6LBR's** (6LoWPAN Border Router) and **6LN's** (6LoWPAN Node). 

### Usage
#### Startup 6LBR
The 6LBR will always be spawned at geographical position {.x = 0, .y = 0}, and will ping address FE80::2 10 times. The size of the ping request can be set by passing the size as the last command-line variable.

To start a 6LBR
```
./edge_router 70
```

#### Spawning 6LN's
The 6LN nodes will always be spawned on the x-axis and on the y-position passed by the last but one command-line variable. Nodes will send UDP garbage packets to the address passed in the last commind-line variable. And the 16-bit short address can be set by passing it in the first commind-line variable.

To start a 6LN
```
./host.sh 1 4 fe80::ff:fe00:0
```

#### Spawning a sniffer
Currently the sniffer will be spawned at location {.x = 0, .y = 4}, and capture all packets received on the 6LoWPAN-network.

To start a sniffer
```
./pcap.sh
```

### Debugging
The application can be debugged by prepending a commind-line variable before every normal command-line variable described above. The simulator can be either started in a GDB-environment by means of passing '**DEBUG**' or either started in a valgrind-environment by passing '**VALGRIND**'.

Starting a 6LBR in GDB
```
./edge_router DEBUG 70
```

Starting a 6LBR in Valgrind
```
./edge_router VALGRIND 70
```
