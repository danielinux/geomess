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

$(PREFIX)/radiodriver.o: radiodriver.c
	@mkdir -p $(PREFIX)
	gcc -o $@  -c $^ -ggdb -Ibuild/include/ -I$(GEOMESS) $(CFLAGS)

#############################
# STATIC LIBRARIES
#############################

$(PREFIX)/lib/libgm.a: $(PREFIX)/libgm.o
	@mkdir -p $(PREFIX)/lib
	ar cru $@ $^
	ranlib $@

$(PREFIX)/lib/libpicotcp.a:
	make -C $(PICOTCP) TCP=0 UDP=1 ETH=0 IPV4=1 NAT=0 MCAST=1 DEVLOOP=0 DHCP_CLIENT=0 DHCP_SERVER=0 DNS_CLIENT=0 MDNS=0 DNS_SD=0 SNTP_CLIENT=0 IPFILTER=0 CRC=0 OLSR=0 SLAACV4=0 TFTP=0 AODV=0 MEMORY_MANAGER=0 MEMORY_MANAGER_PROFILING=0 TUN=0 TAP=0 PCAP=0 PPP=0 SIXLOWPAN=1 CYASSL=0 WOLFSSL=0 POLARSSL=0

$(PREFIX)/lib/radiodriver.a: $(PREFIX)/radiodriver.o $(PREFIX)/libgm.o
	@mkdir -p $(PREFIX)/lib
	ar cru $@ $^
	ranlib $@

#############################
# TARGETS
#############################

geopcap: $(PREFIX)/geopcap.o $(PREFIX)/lib/libgm.a
	gcc -o $@ $^ -lpcap

picomesh: $(PREFIX)/picomesh.o $(PREFIX)/lib/radiodriver.a $(PREFIX)/lib/libpicotcp.a
	gcc -o picomesh $^ -I$(GEOMESS)

geomess: $(PREFIX)/geomess.o $(PREFIX)/libevquick.o
	gcc -o $@ $^ -lm

clean:
	rm -f libgm.test tags geomess picomesh geopcap
	rm -f $(PREFIX)/*.o
	rm -rf build/lib/*.a
	make -C $(PICOTCP) clean
