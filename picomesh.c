#include <stdio.h>
#include "pico_stack.h"
#include "pico_dev_gm.h"
#include "pico_olsr.h"
#include "pico_ipv4.h"

int main(int argc, char *argv[]) 
{
    struct pico_device *dev;
    uint16_t id;
    uint32_t x,y, range_max, range_good;
    struct pico_ip4 address, netmask;



    if (argc != 6)
        exit(1);
    pico_stack_init();
    pico_olsr_init();
    id = (uint16_t)atoi(argv[1]);
    x = (uint32_t)atoi(argv[2]);
    y = (uint32_t)atoi(argv[3]);
    range_max = (uint32_t)atoi(argv[4]);
    range_good = (uint32_t)atoi(argv[5]);


    dev = pico_geomess_create(id, x, y, range_max, range_good);
    pico_string_to_ipv4("255.255.0.0", &netmask.addr);
    pico_string_to_ipv4("10.42.0.0", &address.addr);
    address.addr += ntohl((uint32_t)(id & 0xFF));
    address.addr += ntohl((uint32_t)(id & 0xFF00));
    pico_ipv4_link_add(dev, address, netmask);
    pico_olsr_add(dev);

    for(;;) {
        pico_stack_tick();
        usleep(1000);
    }
}
