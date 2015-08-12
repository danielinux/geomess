#ifndef INCLUDE_PICO_SIXLOWPAN
#define INCLUDE_PICO_SIXLOWPAN

/* picoTCP includes */
#include "pico_device.h"
#include "pico_stack.h"

/**
 *  Generic radio-structure to provide an interface between the 
 *	IEEE802.15.4-radio specific device driver and the 6LoWPAN-
 *	adaption layer.
 */
typedef struct RADIO{
	int (*transmit)(struct RADIO *radio, const void *buf, int len);
	int (*receive)(struct RADIO *radio, void *buf, int len);
} radio_t;

/**
 *  Creates an interface between the sixlowpan adaption layer and
 *  the IEEE802.15.4-device specific device-driver.
 *
 *  @param dev   Sixlowpan-device structure to assign the radio to.
 *  @param radio Device-driver structure to be assigned
 *
 *  @return Returns 0 on succes, something else when the device-driver
 *			structure isn't correctly initialised
 */
int pico_sixlowpan_set_radio(struct pico_device *dev, radio_t *radio);

/**
 *  Custom pico_device creation function.
 *
 *  @param name Name to give to interface.
 *
 *  @return Generic pico_device structure.
 */
struct pico_device *pico_sixlowpan_create(const char *name);

#endif /* INCLUDE_PICO_SIXLOWPAN */