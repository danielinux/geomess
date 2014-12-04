geomess

---------------

GEOMESS is a Geopositioning Mesh network Simulator - designed to test mesh networking on a controlled (virtual/simulated) environment.

##Setup

- clone the repo

- Initialise the submodules:

    cd geomess/
    git submodule update --init

## Building the simulator

    cd apps/ping_client
    make

## Use the simulator

- Change director to the application

    cd apps/ping_client

- Start the geomess engine

    ./geomess

- Start adding nodes to your simulated mesh

    ./picomesh id x y rangeMax rangeGood [metricToPing]

- Create a plot of your network

    python plot.py

- have fun!
