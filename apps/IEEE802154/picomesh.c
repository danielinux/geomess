/* STD-LIBS */
#include <stdio.h>

/* PICOTCP */
#include "pico_stack.h"
#include "pico_ipv6.h"
#include "pico_icmp6.h"
#include "pico_socket.h"

/* GEOMESS */
#include "pico_dev_sixlowpan.h"
#include "radiodriver.h"

#define EVER (;;)
#define NUM_PING 10

static int first_node = 0;

struct pico_device *dev;

void cb(uint16_t ev, struct pico_socket *s)
{
	(void) ev;
	(void) s;
}

void ping(struct pico_icmp6_stats *s)
{
    char host[50];
    pico_ipv6_to_string(host, s->dst.addr);
    if (s->err == 0) {
        dbg("%lu bytes from %s: icmp_req=%lu ttl=%lu time=%lu ms\n", s->size, host, s->seq,
            s->ttl, (long unsigned int)s->time);
        if (s->seq >= NUM_PING)
            exit(0);
    } else {
        dbg("PING %lu to %s: Error %d\n", s->seq, host, s->err);
        exit(1);
    }
}

void udp(pico_time now, void *arg)
{
    struct pico_msginfo info = {.dev = dev, .ttl = 1, .tos = 0};
    union pico_address dst;
    struct pico_socket *s;
    char buf[100] = {0xaa};
    int ret;
    
    pico_string_to_ipv6((char *)arg, dst.ip6.addr);
    
    s = pico_socket_open(PICO_PROTO_IPV6, PICO_PROTO_UDP, cb);
    ret = pico_socket_sendto_extended(s, buf, 70, &dst, short_be(0xF055), &info);
    printf("Sending too large UDP packet: %d (%s)\n", ret, strerror(pico_err));
    pico_socket_close(s);
    
    pico_timer_add(1000, udp, arg);
}


int main(int argc, const char *argv[]) {
    struct pico_ip6 prefix;
	struct ieee_radio *radio;
	
    /* Geomess parameters */
    uint16_t id = 0, pan_id = 0, i = 0;
    uint32_t x = 0, y = 0, range_max = 0, range_good = 0;
    uint8_t size = 0;
    
    /* Too much arguments given? */
    if (argc < 8)
        exit(1);
    
    pico_string_to_ipv6("aaaa:0000:0000:0000:0000:0000:0000:0000", prefix.addr);
    
    /* Parse in command-line variables */
    id = (uint8_t)atoi(argv[1]);
	pan_id = (uint16_t)atoi(argv[2]);
    x = (uint32_t)atoi(argv[3]);
    y = (uint32_t)atoi(argv[4]);
    range_max = (uint32_t)atoi(argv[5]);
    range_good = (uint32_t)atoi(argv[6]);
    if (argc == 9)
        size = (uint8_t)atoi(argv[8]);
    
    /* Initialise picoTCP */
    pico_stack_init();
    
    /* Create the 802.15.4-radio instance */
	if (!(radio = radio_create(id, pan_id, x, y, range_max, range_good))) {
		printf("Could not create radio_t-instance, bailing out..\n");
		exit(1);
	}
	
    /* Create the sixlowpan-device and register it in picoTCP */
    dev = pico_sixlowpan_create(radio);
    
    /* Set the routable prefix of the PAN */
    pico_sixlowpan_set_prefix(dev, prefix);
    
    /* Check if this is the first node */
    if (0 == id) {
        first_node = 1;
        
        /* Enable IPv6 routing on the device */
        //pico_ipv6_dev_routing_enable(dev);
        
        /* Start pinging the remote host */
        if (argc >= 8)
            pico_icmp6_ping((void *)argv[7], NUM_PING, 1000, NUM_PING * 1000, size, ping, dev);
    } else {
        if (argc >= 8)
            pico_timer_add(1000, udp, (void *)argv[7]);
    }
	
    /* Endless loop */
    for EVER {
        pico_stack_tick();
        if (i < 10000) {
            i++;
            usleep(1000);
            continue;
        }
    }
    
    return 0;
}