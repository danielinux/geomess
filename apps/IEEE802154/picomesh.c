/* STD-LIBS */
#include <stdio.h>

/* PICOTCP */
#include "pico_stack.h"
#include "pico_ipv6.h"
#include "pico_icmp6.h"
#include "pico_socket.h"

/* GEOMESS */
#include "pico_dev_sixlowpan.h"

#define EVER (;;)

static int first_node = 0;

struct pico_device *dev;

void cb(uint16_t ev, struct pico_socket *s)
{
	(void) ev;
	(void) s;
}

void ping(pico_time now, void *arg) {
	char hello[128] = "hello";
	int ret = 0;
	
	(void)now;
	(void)arg;
	
	dev->send(dev, hello, 127);
	
	pico_timer_add(1000, ping, NULL);
}

int main(int argc, const char *argv[]) {
    
    /* Geomess parameters */
    uint16_t id = 0;
    uint32_t x, y, range_max, range_good;
    
    /* Network parameters */
    struct pico_ip6 address, netmask;
    
    uint16_t i = 0;
    
    /* Too much arguments given? */
    if (argc > 6)
        exit(1);
    
    /* Parse in command-line variables */
    id = (uint8_t)atoi(argv[1]);
    x = (uint32_t)atoi(argv[2]);
    y = (uint32_t)atoi(argv[3]);
    range_max = (uint32_t)atoi(argv[4]);
    range_good = (uint32_t)atoi(argv[5]);
    
    /* Initialise picoTCP */
    pico_stack_init();
    
    /* Check if this is the first node */
    if (1 == id)
        first_node = 1;
    
    /* Create the the sixlowpan-device and register it in picoTCP */
    dev = pico_sixlowpan_create((uint16_t)id, x, y, range_max, range_good);
    
    /* Determine the network address and netmask */
    pico_string_to_ipv6("2015:0000:0000:0000:0000:0000:0000:0000", address.addr);
    pico_string_to_ipv6("ffff:ffff:ffff:ffff:0000:0000:0000:0000", netmask.addr);
    
    /* Add the identifier to the IPv6-address */
    address.addr[15] += id;
    
    /* Add the address to the link */
    pico_ipv6_link_add(dev, address, netmask);
	
	pico_timer_add(1000, ping, NULL);
	
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