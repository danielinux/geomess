#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>       

#include <sys/types.h>
#include <sys/poll.h>

#ifndef UNIT_TEST
#include "geomess.h"
#endif

#include "radiodriver.h"

#define MTU 127

#define dbg	printf

#define CHECK_PARAM(a, b)	if(!(a)){ \
                                dbg((b)); \
                                return RADIO_ERR_EINVAL; \
                            }(void)a

typedef struct gm_radio
{
	radio_t radio;
	
	/**
	 *  PHY: Geomess-connection
	 */
	GEOMESS conn;
	
	/**
	 *  LIB: LoWPAN Information Base:
	 *	--------------------------------
	 *
	 *	PSI: Pan Specific Information
	 */
	uint16_t	pan_identifier;
	uint8_t		pan_channel;
	
	/**
	 *  DSI: Device Specific Information
	 */
	uint8_t		address_extended[8];
	uint16_t	address_short;
	
	/**
	 *  16-bit address pool to hand out short 
	 *	addresses from.
	 */
	uint16_t	short_address_pool;
}
gm_radio_t;

static uint16_t calculate_crc16(uint8_t *buf, uint8_t len)
{
    uint16_t crc = 0x0000;
    uint16_t q = 0, i = 0;
    uint8_t c = 0;
    
    for (i = 0; i < len; i++) {
        c = buf[i];
        q = (crc ^ c) & 0x0F;
        crc = (crc >> 4) ^ (q * 0x1081);
        q = (crc ^ (c >> 4)) & 0xF;
        crc = (crc >> 4) ^ (q * 0x1081);
    }
    
    return crc;
}

static radio_rcode_t radio_transmit(radio_t *radio, void *buf, int len)
{
	/* Parse the generic radio structure to the internal Geomess radio-structure */
    gm_radio_t *gm_radio = (gm_radio_t *)radio;
    uint16_t crc = 0;
	
    dbg("[RADIODRIVER] > LEN: %04d bytes.\n", len);
    
    /* Calculate the FCS */
    crc = calculate_crc16(buf + 1, len - 3); /* buf + 1 to skip the length-byte */
    dbg("[RADIODRIVER] > CRC: %#X\n", crc);
    
    memcpy(buf + len - 2, (void *)&crc, 2);
    
	/* Send the payload over this radio's geomesh-connection */
	return geomess_send(gm_radio->conn, buf + 1, (uint32_t)len); /* buf + 1 to skip the length-byte */
}

static int radio_receive(radio_t *radio, void *buf, int len)
{
	/* Parse the generic radio structure to the internal Geomess radio-structure */
	gm_radio_t *gm_radio = (gm_radio_t *)radio;
	
	struct pollfd pfd;
	
	/* Get the file-descriptor of this device's connection to the Geomesh-network */
	pfd.fd = geomess_datafd(gm_radio->conn);
	
	/* Set the events to poll */
	pfd.events = POLLIN;
	
	/* Poll the file-descriptors, and return if nothing is selected */
	if (poll(&pfd, 1, 0) <= 0)
		return -1;
	
	/* If a file-descriptor is selected, retrieve data from it */
	return geomess_recv(gm_radio->conn, buf, len);
}

static radio_rcode_t radio_getEUI64(radio_t *radio, uint8_t buf[8])
{
	/* Parse the generic radio structure to the internal Geomess radio-structure */
	gm_radio_t *gm_radio = (gm_radio_t *)radio;

	memcpy(buf, gm_radio->address_extended, 8);
	
	return 0;
}

static radio_rcode_t radio_setSHORT16(radio_t *radio, uint16_t short_16)
{
    /* Parse the generic radio structure to the internal Geomess radio-structure */
    gm_radio_t *gm_radio = (gm_radio_t *)radio;
    
    /* Just set the short address */
    gm_radio->address_short = short_16;
    
    return RADIO_ERR_NOERR;
}

static uint16_t radio_getSHORT16(radio_t *radio)
{
    /* Parse the generic radio structure to the internal Geomess radio-structure */
    gm_radio_t *gm_radio = (gm_radio_t *)radio;
    
    return gm_radio->address_short;
}

static uint16_t radio_getPAN_ID(radio_t *radio)
{
    /* Parse the generic radio structure to the internal Geomess radio-structure */
    gm_radio_t *gm_radio = (gm_radio_t *)radio;
    
    return gm_radio->address_short;
}

static inline void dbgEUI64(uint8_t buf[8])
{
    int i = 0;
    int n = 8;
    
    printf("EUI-64: ");
    for (i = 0; i < n; i++) {
        printf("%02X", buf[i]);
        if (i != n - 1)
            printf(":");
    }
    printf("\n");
}

/* Based on picoapp MAC-generation */
static void gen_rand_EUI64(uint8_t buf[8])
{
    unsigned char macaddr[6] = { 0, 0, 0, 0xa, 0xb, 0x0 };
    unsigned char enc[2] = { 0xFF, 0xFE };
    uint16_t *macaddr_low = (uint16_t *) (macaddr + 2);
    *macaddr_low = (uint16_t)(*macaddr_low ^ (uint16_t)((uint16_t)getpid() & (uint16_t)0xFFFFU));
    
    memcpy(buf, macaddr, 3);
    memcpy((buf + 3), enc, 2);
    memcpy((buf + 5), (macaddr + 3), 3);
	
	dbg("[RADIODRIVER] > Generated random EUI64: \n", buf);
    
    buf[0] = buf[0] ^ 0x02;
}

/**
 *  <#Description#>
 *
 *  @param id             <#id description#>
 *  @param pan_identifier <#pan_identifier description#>
 *  @param pan_channel    <#pan_channel description#>
 *  @param x              <#x description#>
 *  @param y              <#y description#>
 *  @param range_max      <#range_max description#>
 *  @param range_good     <#range_good description#>
 *
 *  @return <#return value description#>
 */
radio_t *radio_create(uint16_t	id,
					  uint16_t	pan_identifier,
					  uint8_t	pan_channel,
					  uint32_t	x,
					  uint32_t	y,
					  uint32_t	range_max,
					  uint32_t  range_good)
{
	/* Create the radio-instance */
	gm_radio_t *gm_radio = (gm_radio_t *)malloc(sizeof(gm_radio_t));
	if (!gm_radio)
		return NULL;
	
	/* Try to make a connection with the geomess-network */
    if (!(gm_radio->conn = geomess_open(id, x, y, range_max, range_good)))
		return NULL;
	
	/* Set the callbacks of this radio-instance */
	gm_radio->radio.transmit = radio_transmit;
	gm_radio->radio.receive = radio_receive;
    gm_radio->radio.get_EUI64 = radio_getEUI64;
    gm_radio->radio.get_short_16 = radio_getSHORT16;
    gm_radio->radio.set_short_16 = radio_setSHORT16;
    gm_radio->radio.get_pan_id = radio_getPAN_ID;
	
	/* Set the short-ID by command-line options */
	gm_radio->address_short = id;
	
	/* Generate a random EUI64-address */
	gen_rand_EUI64(gm_radio->address_extended);
	
	/* Set the 802.15.4 specific parameters */
	gm_radio->pan_identifier = pan_identifier;
	gm_radio->pan_channel = pan_channel;
	
	return (radio_t *)gm_radio;
}
