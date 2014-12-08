/*********************************************************************
   PicoTCP. Copyright (c) 2012 TASS Belgium NV. Some rights reserved.
   See LICENSE and COPYING for usage.

   Authors: Daniele Lacamera
 *********************************************************************/

#ifndef UNIT_TEST
#include "geomess.h"
#endif
#include "pico_device.h"
#include "pico_stack.h"

#include <sys/poll.h>
#define GM_MTU 2048

struct pico_device_geomess {
    struct pico_device dev;
    char *sock;
    GEOMESS conn;
};

static int pico_geomess_send(struct pico_device *dev, void *buf, int len)
{
    struct pico_device_geomess *geomess = (struct pico_device_geomess *) dev;
    return geomess_send(geomess->conn, buf, (uint32_t)len);
}

static int pico_geomess_poll(struct pico_device *dev, int loop_score)
{
    struct pico_device_geomess *geomess = (struct pico_device_geomess *) dev;
    struct pollfd pfd;
    unsigned char buf[GM_MTU];
    int len;
    pfd.fd = geomess_datafd(geomess->conn);
    pfd.events = POLLIN;
    do  {
        if (poll(&pfd, 1, 0) <= 0)
            return loop_score;

        len = geomess_recv(geomess->conn, buf, GM_MTU);
        if (len > 0) {
            pico_stack_recv(dev, buf, (uint32_t)len);
        }
    } while(loop_score > 0);
    return 0;
}

/* Public interface: create/destroy. */

struct pico_device *pico_geomess_create(uint16_t id, uint32_t x, uint32_t y, uint32_t range_max, uint32_t range_good)
{
    struct pico_device_geomess *geomess = PICO_ZALLOC(sizeof(struct pico_device_geomess));
    char name[250];
    snprintf(name, 250, "picomesh%04x", id);

    if( 0 != pico_device_init((struct pico_device *)geomess, name, NULL)) {
        dbg ("Device init failed.\n");
        return NULL;
    }

    geomess->dev.overhead = 0;
    geomess->conn = geomess_open(id, x, y, range_max, range_good);
    if (!geomess->conn) {
        return NULL;
    }

    geomess->dev.send = pico_geomess_send;
    geomess->dev.poll = pico_geomess_poll;
    dbg("Device created.\n");
    return (struct pico_device *)geomess;
}

