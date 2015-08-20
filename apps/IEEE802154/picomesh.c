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

static int first_node = 0;

struct pico_device *dev;

void cb(uint16_t ev, struct pico_socket *s)
{
	(void) ev;
	(void) s;
}

void ping(pico_time now, void *arg) {
	
	pico_timer_add(1000, ping, NULL);
}

int main(int argc, const char *argv[]) {
	radio_t *radio;
	
    /* Geomess parameters */
    uint16_t id = 0, pan_identifier = 0, i = 0;
    uint32_t x = 0, y = 0, range_max = 0, range_good = 0;
    uint8_t pan_channel = 0;
    
    /* Too much arguments given? */
    if (argc > 8 || argc < 8)
        exit(1);
    
    /* Parse in command-line variables */
    id = (uint8_t)atoi(argv[1]);
	pan_identifier = (uint16_t)atoi(argv[2]);
	pan_channel = (uint8_t)atoi(argv[3]);
    x = (uint32_t)atoi(argv[4]);
    y = (uint32_t)atoi(argv[5]);
    range_max = (uint32_t)atoi(argv[6]);
    range_good = (uint32_t)atoi(argv[7]);
    
    /* Initialise picoTCP */
    pico_stack_init();
    
    /* Create the 802.15.4-radio instance */
	if (!(radio = radio_create(id,
							   pan_identifier,
							   pan_channel,
							   x,
							   y,
							   range_max,
							   range_good)))
	{
		printf("Could not create radio_t-instance, bailing out..\n");
		exit(1);
	}
	
	/* Check if this is the first node */
	if (0 == id) {
		first_node = 1;
	}
	
    /* Create the sixlowpan-device and register it in picoTCP */
    dev = pico_sixlowpan_create(radio);
    
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