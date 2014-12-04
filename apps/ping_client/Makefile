LIBDIR?=../../lib
PICOTCP?=$(LIBDIR)/picotcp
GEOMESS?=$(LIBDIR)/geomess
LIBEVQUICK?=$(LIBDIR)/libevquick
PREFIX?=./build

all: geomess picomesh geopcap

$(PREFIX)/libgm.o: $(GEOMESS)/libgm.c
	@mkdir -p $(PREFIX)
	gcc -o $@ -c $^ -ggdb -I$(LIBEVQUICK)

$(PREFIX)/libgm.test: libgm.c
	@mkdir -p $(PREFIX)
	gcc -o $@ $^ -DTEST -ggdb -O0

$(PREFIX)/geomess.o: $(GEOMESS)/geomess.c
	@mkdir -p $(PREFIX)
	gcc -o $@  -c $^ -I$(LIBEVQUICK) -I$(GEOMESS) -ggdb -O0

$(PREFIX)/geopcap.o:  $(GEOMESS)/geopcap.c
	@mkdir -p $(PREFIX)
	gcc -o $@  -c $^ -I$(LIBEVQUICK) -I$(GEOMESS) -ggdb -O0

$(PREFIX)/libevquick.o: $(LIBEVQUICK)/libevquick.c
	@mkdir -p $(PREFIX)
	gcc -o $@ -c $^ -I. -ggdb -O0

$(PREFIX)/picomesh.o: $(PREFIX)/lib/libpicotcp.a picomesh.c
	@mkdir -p $(PREFIX)
	gcc -o $@  -c $^ -ggdb -Ibuild/include/ -I$(GEOMESS)

$(PREFIX)/pico_dev_gm.o: $(GEOMESS)/pico_dev_gm.c
	@mkdir -p $(PREFIX)
	gcc -o $@  -c $^ -ggdb -Ibuild/include/ -I$(GEOMESS)

$(PREFIX)/lib/libgm.a: $(PREFIX)/libgm.o
	@mkdir -p $(PREFIX)/lib
	ar cru $@ $^
	ranlib $@

test: libgm.test

$(PREFIX)/lib/libpicotcp.a:
	make -C $(PICOTCP) TFTP=0 UDP=1 TCP=1 MCAST=0 IPFRAG=1 IPV6=0 OLSR=1 DNS_CLIENT=0 DHCP_CLIENT=0 HTTP_CLIENT=0 DHCP_SERVER=0 HTTP_SERVER=0 ICMP4=1 SLAACV4=1 ZMQ=0 ETH=1 DEVLOOP=1 NAT=1 IPFILTER=1 SIMPLE_HTTP=0 DEBUG=1 ROUTE_DEBUG=1

$(PREFIX)/lib/pico_dev_gm.a: $(PREFIX)/pico_dev_gm.o $(PREFIX)/libgm.o
	@mkdir -p $(PREFIX)/lib
	ar cru $@ $^
	ranlib $@

geopcap: $(PREFIX)/geopcap.o $(PREFIX)/lib/pico_dev_gm.a
	gcc -o $@ $^ -lpcap

picomesh: $(PREFIX)/picomesh.o $(PREFIX)/lib/pico_dev_gm.a build/lib/libpicotcp.a
	gcc -o picomesh $^ -I$(GEOMESS)

geomess: $(PREFIX)/geomess.o $(PREFIX)/libevquick.o
	gcc -o $@ $^ -lm

clean:
	rm -f libgm.test tags geomess picomesh geopcap
	rm -f $(PREFIX)/*.o
	rm -rf build/lib/*.a
	make -C $(PICOTCP) clean
