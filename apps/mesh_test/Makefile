LIBDIR?=../../lib
PICOTCP?=$(LIBDIR)/picotcp
GEOMESS?=$(LIBDIR)/geomess
LIBEVQUICK?=$(LIBDIR)/libevquick
PREFIX?=./build
ROUTING?=OLSR
CFLAGS=-Wall -D$(ROUTING)

all: geomess picomesh geopcap


$(PREFIX)/libgm.o: $(GEOMESS)/libgm.c
	@mkdir -p $(PREFIX)
	gcc -o $@ -c $^ -ggdb -I$(LIBEVQUICK) $(CFLAGS)

$(PREFIX)/libgm.test: libgm.c
	@mkdir -p $(PREFIX)
	gcc -o $@ $^ -DTEST -ggdb -O0 $(CFLAGS) 

$(PREFIX)/geomess.o: $(GEOMESS)/geomess.c
	@mkdir -p $(PREFIX)
	gcc -o $@  -c $^ -I$(LIBEVQUICK) -I$(GEOMESS) -ggdb -O0 $(CFLAGS) 

$(PREFIX)/geopcap.o:  $(GEOMESS)/geopcap.c
	@mkdir -p $(PREFIX)
	gcc -o $@  -c $^ -I$(LIBEVQUICK) -I$(GEOMESS) -ggdb -O0 $(CFLAGS)

$(PREFIX)/libevquick.o: $(LIBEVQUICK)/libevquick.c
	@mkdir -p $(PREFIX)
	gcc -o $@ -c $^ -I. -ggdb -O0 $(CFLAGS)

$(PREFIX)/picomesh.o: $(PREFIX)/lib/libpicotcp.a picomesh.c
	@mkdir -p $(PREFIX)
	gcc -o $@  -c $^ -ggdb -Ibuild/include/ -I$(GEOMESS) $(CFLAGS)

$(PREFIX)/pico_dev_gm.o: $(GEOMESS)/pico_dev_gm.c
	@mkdir -p $(PREFIX)
	gcc -o $@  -c $^ -ggdb -Ibuild/include/ -I$(GEOMESS) $(CFLAGS)

$(PREFIX)/lib/libgm.a: $(PREFIX)/libgm.o
	@mkdir -p $(PREFIX)/lib
	ar cru $@ $^
	ranlib $@

test: libgm.test

$(PREFIX)/lib/libpicotcp.a:
	make -C $(PICOTCP) OLSR=1 AODV=1

$(PREFIX)/lib/pico_dev_gm.a: $(PREFIX)/pico_dev_gm.o $(PREFIX)/libgm.o
	@mkdir -p $(PREFIX)/lib
	ar cru $@ $^
	ranlib $@

geopcap: $(PREFIX)/geopcap.o $(PREFIX)/lib/pico_dev_gm.a
	gcc -o $@ $^ -lpcap

picomesh: $(PREFIX)/picomesh.o $(PREFIX)/lib/pico_dev_gm.a $(PREFIX)/lib/libpicotcp.a
	gcc -o picomesh $^ -I$(GEOMESS)

geomess: $(PREFIX)/geomess.o $(PREFIX)/libevquick.o
	gcc -o $@ $^ -lm

clean:
	rm -f libgm.test tags geomess picomesh geopcap
	rm -f $(PREFIX)/*.o
	rm -rf build/lib/*.a
	make -C $(PICOTCP) clean
