#ifndef UNIT_TEST
#include "geomess.h"
#endif
#include "pico_device.h"
#include "pico_stack.h"

#include <sys/poll.h>
#define GM_MTU 127

struct pico_device_sixlowpan {
    struct pico_device dev;
    char *sock;
    GEOMESS conn;
};

static int pico_sixlowpan_send(struct pico_device *dev, void *buf, int len)
{
    struct pico_device_sixlowpan *sixlowpan = (struct pico_device_sixlowpan *)dev;
    return geomess_send(sixlowpan->conn, buf, (uint32_t)len);
}

static int pico_sixlowpan_poll(struct pico_device *dev, int loop_score)
{
    struct pico_device_sixlowpan *sixlowpan = (struct pico_device_sixlowpan *) dev;
    struct pollfd pfd;
    unsigned char buf[GM_MTU];
    int len;
	int i = 0;
    pfd.fd = geomess_datafd(sixlowpan->conn);
    pfd.events = POLLIN;
    do  {
        if (poll(&pfd, 1, 0) <= 0)
            return loop_score;
        
        len = geomess_recv(sixlowpan->conn, buf, GM_MTU);
        if (len > 0) {
			printf("RCVD:\n");
			for (i = 0; i < len; i++) {
				printf("%c", (char)(*(buf + i)));
			}
			printf("\n");
            pico_stack_recv(dev, buf, (uint32_t)len);
        }
    } while(loop_score > 0);
    return 0;
}


struct pico_device *pico_sixlowpan_create(uint16_t id, uint32_t x, uint32_t y, uint32_t range_max, uint32_t range_good)
{
    struct pico_device_sixlowpan *sixlowpan = PICO_ZALLOC(sizeof(struct pico_device_sixlowpan));
    char name[250];
    snprintf(name, 250, "sixlowpan%04x", id);
    
    if( 0 != pico_device_init((struct pico_device *)sixlowpan, name, NULL)) {
        dbg ("Device init failed.\n");
        return NULL;
    }
    
    sixlowpan->dev.overhead = 0;
    sixlowpan->conn = geomess_open(id, x, y, range_max, range_good);
    if (!sixlowpan->conn) {
        return NULL;
    }
    
    sixlowpan->dev.send = pico_sixlowpan_send;
    sixlowpan->dev.poll = pico_sixlowpan_poll;
    dbg("Device created.\n");
    return (struct pico_device *)sixlowpan;
}
