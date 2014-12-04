#include <stdio.h>
#include "pico_stack.h"
#include "pico_dev_gm.h"
#include "pico_olsr.h"
#include "pico_ipv4.h"
#include "pico_icmp4.h"

#define NUM_PING 1


void cb_ping(struct pico_icmp4_stats *s)
{
    char host[30];
    pico_ipv4_to_string(host, s->dst.addr);
    if (s->err == 0) {
        dbg("%lu bytes from %s: icmp_req=%lu ttl=64 time=%lu ms\n", s->size, host, s->seq, s->time);
    } else {
        dbg("PING %lu to %s: Error %d\n", s->seq, host, s->err);
        //exit(1);
    }
}

int main(int argc, char *argv[])
{
    struct pico_device *dev;
    uint16_t counter= 0;
    uint16_t id;
    uint32_t x,y, range_max, range_good;
    struct pico_ip4 address, netmask;
    struct pico_tree_node *index;
    struct pico_ipv4_route * entry;
    char addr[16];
    uint16_t interval = 1000;
    uint16_t timeout  = 9999;
    uint8_t size  = 28;
    uint32_t metric  = 0;
    uint8_t numping  = 1;


    if (argc > 7)
        exit(1);
    pico_stack_init();
    id = (uint16_t)atoi(argv[1]);
    x = (uint32_t)atoi(argv[2]);
    y = (uint32_t)atoi(argv[3]);
    range_max = (uint32_t)atoi(argv[4]);
    range_good = (uint32_t)atoi(argv[5]);
    if (argc == 7)
        metric = (uint32_t)atoi(argv[6]);
    else
        metric = 0;


    dev = pico_geomess_create(id, x, y, range_max, range_good);
    pico_string_to_ipv4("255.255.0.0", &netmask.addr);
    pico_string_to_ipv4("10.42.0.0", &address.addr);
    address.addr += ntohl((uint32_t)(id & 0xFF));
    address.addr += ntohl((uint32_t)(id & 0xFF00));
    pico_ipv4_link_add(dev, address, netmask);
    pico_olsr_add(dev);

    for(;;) {
        pico_stack_tick();
        if (counter < 10000) {
            counter++;
            usleep(1000);
            continue;
        }else{
            counter=0;
            if (metric!=0) {
                dbg("timer passed\n");
                pico_tree_foreach(index, &Routes) {
                    dbg("Entry\n");
                    entry = index->keyValue;
                    if (entry->metric == metric) {
                        pico_ipv4_to_string(addr, entry->dest.addr);
                        dbg("Ping to %s\n",addr);
                        pico_icmp4_ping(addr, NUM_PING, interval, timeout, size, cb_ping);
                    }
                }
            }
        }
    }
}
