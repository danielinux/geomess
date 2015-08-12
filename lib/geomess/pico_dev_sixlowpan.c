/* Custom includes */
#include "pico_dev_sixlowpan.h"

/* --- DEFINES --- */
#define MTU 127

struct pico_device_sixlowpan {
	struct pico_device dev;
	
	/* Interface between pico_device-structure & 802.15.4-device driver */
	radio_t *radio;
};

static int pico_sixlowpan_send(struct pico_device *dev, void *buf, int len)
{
	/* Parse the pico_device structure to the internal sixlowpan-structure */
    struct pico_device_sixlowpan *sixlowpan = (struct pico_device_sixlowpan *)dev;
	
	/* Call the transmit-callback on this sixlowpan's specific radio-instance */
	return sixlowpan->radio->transmit(sixlowpan->radio, buf, len);
}

static int pico_sixlowpan_poll(struct pico_device *dev, int loop_score)
{
	/* Parse the pico_device structure to the internal sixlowpan-structure */
	struct pico_device_sixlowpan *sixlowpan = (struct pico_device_sixlowpan *) dev;
	
    unsigned char buf[MTU];
	int i = 0;
    int len = 0;
    
    do {
		/* Try to receive data from radio-interface */
		len = sixlowpan->radio->receive(sixlowpan->radio, buf, MTU);
		if (len < 0)
			return loop_score;
		else if (len > 0) {
			printf("RCVD:\n");
			for (i = 0; i < len; i++)
				printf("%02X", (char)(*(buf + i)));
			printf("<<<< EOF\n");
			pico_stack_recv(dev, buf, (uint32_t)len);
		}
	} while (loop_score > 0);
	
    return 0;
}

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
int pico_sixlowpan_set_radio(struct pico_device *dev, radio_t *radio)
{
	/* Parse the pico_device structure to the internal sixlowpan-structure */
	struct pico_device_sixlowpan *sixlowpan = (struct pico_device_sixlowpan *)dev;
	
	/* Check if the structure is correctly initialised */
	if (!radio || !(radio->transmit) || !(radio->receive))
		return -1;
	
	/* Assign the radio-instance to the pico_device-instance */
	sixlowpan->radio = radio;
	
	return 0;
}

/**
 *  Custom pico_device creation function.
 *
 *  @param name Name to give to interface.
 *
 *  @return Generic pico_device structure.
 */
struct pico_device *pico_sixlowpan_create(const char *name)
{
    struct pico_device_sixlowpan *sixlowpan = PICO_ZALLOC(sizeof(struct pico_device_sixlowpan));
	
	/* Try to init & register the device to picoTCP */
    if (0 != pico_device_init((struct pico_device *)sixlowpan, name, NULL))
	{
        dbg("Device init failed.\n");
        return NULL;
    }
	
    /* Set the device-parameters*/
    sixlowpan->dev.overhead = 0;
    sixlowpan->dev.send = pico_sixlowpan_send;
    sixlowpan->dev.poll = pico_sixlowpan_poll;
	
    dbg("Device created.\n");
	
	/* Cast internal 6LoWPAN-structure to picoTCP-device structure */
    return (struct pico_device *)sixlowpan;
}