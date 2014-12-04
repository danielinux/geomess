geomess

---------------

GEOMESS is a Geopositioning Mesh network Simulator - designed to test mesh networking on a controlled (virtual/simulated) environment.

How to build/use:

- compile a recent PicoTCP with "make OLSR=1"

- run "make PICOTCP=/path/to/picotcp/build/dir"

- Start the geomess engine ("./geomess")

- Start adding nodes to your simulated mesh (pico nodes are added via ./picomesh id x y rangeMax rangeGood)

- have fun!


