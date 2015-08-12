#ifndef INCLUDE_DEVICE_DRIVER
#define INCLUDE_DEVICE_DRIVER

#ifndef UNIT_TEST
#include "geomess.h"
#endif

#include "pico_dev_sixlowpan.h"

typedef struct gm_radio {
	radio_t radio;
	char *sock;
	GEOMESS conn;
} gm_radio_t;

radio_t *radio_create(uint16_t id, uint32_t x, uint32_t y, uint32_t range_max, uint32_t range_good);

int radio_transmit(radio_t *radio, const void *buf, int len);
int radio_receive(radio_t *radio, void *buf, int len);

#endif /* INCLUDE_DEVICE_DRIVER */