LIBDIR?=../../lib
PICOTCP?=$(LIBDIR)/picotcp
GEOMESS?=$(LIBDIR)/geomess
LIBEVQUICK?=$(LIBDIR)/libevquick
PREFIX?=./build
CFLAGS=-Wall

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

$(PREFIX)/pico_dev_sixlowpan.o: $(GEOMESS)/pico_dev_sixlowpan.c
	@mkdir -p $(PREFIX)
	gcc -o $@  -c $^ -ggdb -Ibuild/include/ -I$(GEOMESS) $(CFLAGS)

$(PREFIX)/radiodriver.o: radiodriver.c
	@mkdir -p $(PREFIX)
	gcc -o $@  -c $^ -ggdb -Ibuild/include/ -I$(GEOMESS) $(CFLAGS)

$(PREFIX)/lib/libgm.a: $(PREFIX)/libgm.o
	@mkdir -p $(PREFIX)/lib
	ar cru $@ $^
	ranlib $@

test: libgm.test

$(PREFIX)/lib/libpicotcp.a:
	make -C $(PICOTCP)

$(PREFIX)/lib/pico_dev_sixlowpan.a: $(PREFIX)/pico_dev_sixlowpan.o $(PREFIX)/libgm.o
	@mkdir -p $(PREFIX)/lib
	ar cru $@ $^
	ranlib $@

geopcap: $(PREFIX)/geopcap.o $(PREFIX)/lib/pico_dev_sixlowpan.a
	gcc -o $@ $^ -lpcap

picomesh: $(PREFIX)/picomesh.o $(PREFIX)/radiodriver.o $(PREFIX)/lib/pico_dev_sixlowpan.a build/lib/libpicotcp.a
	gcc -o picomesh $^ -I$(GEOMESS)

geomess: $(PREFIX)/geomess.o $(PREFIX)/libevquick.o
	gcc -o $@ $^ -lm

clean:
	rm -f libgm.test tags geomess picomesh geopcap
	rm -f $(PREFIX)/*.o
	rm -rf build/lib/*.a
	make -C $(PICOTCP) clean
