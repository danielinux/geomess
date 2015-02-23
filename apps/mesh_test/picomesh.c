#include <stdio.h>
#include "pico_stack.h"
#include "pico_dev_gm.h"
#include "pico_olsr.h"
#include "pico_ipv4.h"
#include "pico_icmp4.h"
#include "pico_socket.h"

#define NUM_PING 1
#define TEST_INTERVAL 5000
#define TEST_ATTEMPTS 20
static int first_node = 0;
static uint32_t metric = 0;


void cb_ping(struct pico_icmp4_stats *s)
{
    char host[30];
    pico_ipv4_to_string(host, s->dst.addr);
    if (s->err == 0) {
        dbg("%lu bytes from %s: icmp_req=%lu ttl=64 time=%lu ms\n", s->size, host, s->seq, s->time);
        exit(0);
    } else {
        dbg("PING %lu to %s: Error %d\n", s->seq, host, s->err);
        exit(1);
    }
}

void cb(uint16_t ev, struct pico_socket *s)
{
    (void) ev;
    (void) s;
}

void aodv_explore(pico_time now, void *arg)
{
    union pico_address dest;
    struct pico_socket *s;
    char hello[6] = "hello";
    int ret;
    pico_string_to_ipv4("10.42.0.0", &dest.ip4.addr);
    (void)arg;

    dest.ip4.addr += long_be(metric + 1);


    if (first_node) {
        s = pico_socket_open(PICO_PROTO_IPV4, PICO_PROTO_UDP, cb);
        ret = pico_socket_sendto(s, hello, 5, &dest, short_be(5555));
        printf("Sending explore hello packet via udp: %d (%s)\n", ret, strerror(pico_err));
        pico_socket_close(s);
        pico_timer_add(10000, aodv_explore, NULL);
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
    char addr[16], gw[16], nm[16];
    uint16_t interval = 1000;
    uint16_t timeout  = 9999;
    uint8_t size  = 28;
    uint8_t numping  = 1;
    int attempt = 0;


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



    if (id == 1)
        first_node = 1;

    dev = pico_geomess_create(id, x, y, range_max, range_good);
    pico_string_to_ipv4("255.255.255.0", &netmask.addr);
    pico_string_to_ipv4("10.42.0.0", &address.addr);
    address.addr += ntohl((uint32_t)(id & 0xFF));
    address.addr += ntohl((uint32_t)(id & 0xFF00));
    //address.addr += ntohl((uint32_t)(id & 0xFF)<<16);
    pico_ipv4_link_add(dev, address, netmask);

#ifdef OLSR
    pico_olsr_add(dev);
#else
    pico_aodv_add(dev);
    pico_timer_add(1000, aodv_explore, NULL);
#endif

    for(;;) {
        pico_stack_tick();
        if (!first_node) {
            usleep(1000);
            continue;
        }
        if (counter < TEST_INTERVAL) {
            counter++;
            usleep(1000);
            continue;
        }else{
            counter=0;
            if (metric!=0) {
                dbg("timer passed\n");
                attempt++;
                pico_tree_foreach(index, &Routes) {
                    entry = index->keyValue;
                    pico_ipv4_to_string(addr, entry->dest.addr);
                    pico_ipv4_to_string(nm, entry->netmask.addr);
                    pico_ipv4_to_string(gw, entry->gateway.addr);
                    dbg("Entry %s/%s via %s metric %hu\n", addr, nm, gw, entry->metric);
                    if (entry->netmask.addr == 0xFFFFFFFF && (entry->metric == metric)) { 
                        dbg("Ping to %s via %s\n",addr, gw);
                        pico_icmp4_ping(addr, NUM_PING, interval, timeout, size, cb_ping);
                        break;
                    }
                } /* End foreach */
            }
        }
        if (attempt > TEST_ATTEMPTS) {
            printf("Found no host at distance %d\n", metric);
            exit(2);
        }
    }
}
