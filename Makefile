PICOTCP?=~/picotcp/build

all: geomess libgm.a picomesh geopcap

geomess: geomess.o libevquick.o
	gcc -o geomess geomess.o libevquick.o -lm


libgm.a: libgm.o
	ar cru libgm.a libgm.o
	ranlib libgm.a

libgm.o: libgm.c
	gcc -c $^ -ggdb

libgm.test: libgm.c
	gcc -o $@ libgm.c -DTEST -ggdb -O0
	

geomess.o: geomess.c
	gcc -c geomess.c -I. -ggdb -O0

geopcap.o: geopcap.c
	gcc -c geopcap.c -I. -ggdb -O0

libevquick.o: libevquick.c
	gcc -c libevquick.c -I. -ggdb -O0


test: libgm.test

picomesh: picomesh.o pico_dev_gm.a $(PICOTCP)/lib/libpicotcp.a 
	gcc -o picomesh $^

geopcap: geopcap.o libgm.o
	gcc -o geopcap $^ -lpcap

pico_dev_gm.a: pico_dev_gm.o libgm.o
	ar cru $@ $^ 
	ranlib $@

picomesh.o: picomesh.c
	gcc -c $^ -ggdb -I $(PICOTCP)/include/

pico_dev_gm.o: pico_dev_gm.c
	gcc -c $^ -ggdb -I $(PICOTCP)/include/


clean: 
	rm -f libgm.test libgm.a geomess *.o tags picomesh geopcap

