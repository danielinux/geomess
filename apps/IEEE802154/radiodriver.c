#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>       

#include <sys/types.h>
#include <sys/poll.h>

#ifndef UNIT_TEST
#include "geomess.h"
#endif

#include "radiodriver.h"

//#define dbg do {} while (0);
#define dbg	printf

#define MTU 127

#define CHECK_PARAM(a, b)   if(!(a)){ \
                                dbg("[RADIODRIVER]$ %s %d", __FUNCTION__, (b)); \
                                return RADIO_ERR_EINVAL; \
                            }(void)a

/**
 *  Geomess specific radio-instance definition
 */
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

/**
 *  Simulated CRC16-CITT Kermit generation
 *
 *  @param buf uint8_t *, buffer to generate FCS for.
 *  @param len uint8_t, len of the buffer
 *
 *  @return CITT Kermit CRC16 of the buffer
 */
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

/**
 *  Radio driver's TX function
 *
 *  @param radio radio_t *, radio-driver instance
 *  @param buf   void *, payload to TX
 *  @param len   int, length of the payload
 *
 *  @return radio_rcode_t, see radio_rcode_t in 'pico_dev_sixlowpan.h'
 */
static radio_rcode_t radio_transmit(radio_t *radio, void *buf, int len)
{
    gm_radio_t *gm_radio = NULL;
    uint16_t crc = 0;
    
    CHECK_PARAM(radio, __LINE__);
    CHECK_PARAM(buf, __LINE__);

	/* Parse the generic radio structure to the internal Geomess radio-structure */
    gm_radio = (gm_radio_t *)radio;
	
    dbg("[RADIODRIVER]$ Payload: %04d bytes.\n", len);
    
    /* Calculate the FCS */
    crc = calculate_crc16(buf + 1, len - 3); /* buf + 1 to skip the length-byte */
    dbg("[RADIODRIVER]$ FCS: %#X\n", crc);
    
    memcpy(buf + len - 2, (void *)&crc, 2);
    
	/* Send the payload over this radio's geomesh-connection */
	return geomess_send(gm_radio->conn, buf + 1, (uint32_t)len - 1); /* buf + 1 to skip the length-byte, len - 1 to don't cause overflow */
}

/**
 *  Radio driver's RX function
 *
 *  @param radio radio_t *, radio-driver instance
 *  @param buf   void *, buffer to fill with received data
 *  @param len   int, len
 *
 *  @return
 */
static int radio_receive(radio_t *radio, void *buf, int len)
{
	/* Parse the generic radio structure to the internal Geomess radio-structure */
	gm_radio_t *gm_radio = (gm_radio_t *)radio;
	struct pollfd pfd;
    int ret = 0;
	
	/* Get the file-descriptor of this device's connection to the Geomesh-network */
	pfd.fd = geomess_datafd(gm_radio->conn);
	
	/* Set the events to poll */
	pfd.events = POLLIN;
	
	/* Poll the file-descriptors, and return if nothing is selected */
	if (poll(&pfd, 1, 0) <= 0)
		return -1;
    
    /* 6LoWPAN excepts a 128-byte buffer with len + payload + fcs */
    ret = geomess_recv(gm_radio->conn, buf + 1, len - 1);
    ((uint8_t *)buf)[0] = (uint8_t)ret;
	
	/* If a file-descriptor is selected, retrieve data from it */
	return ret;
}

static radio_rcode_t radio_getEUI64(radio_t *radio, uint8_t buf[8])
{
	/* Parse the generic radio structure to the internal Geomess radio-structure */
	gm_radio_t *gm_radio = (gm_radio_t *)radio;

	memcpy(buf, gm_radio->address_extended, 8);
	
	return 0;
}

static uint16_t radio_getSHORT16(radio_t *radio)
{
    /* Parse the generic radio structure to the internal Geomess radio-structure */
    gm_radio_t *gm_radio = (gm_radio_t *)radio;
    
    return gm_radio->address_short;
}

static radio_rcode_t radio_setSHORT16(radio_t *radio, uint16_t short_16)
{
    /* Parse the generic radio structure to the internal Geomess radio-structure */
    gm_radio_t *gm_radio = (gm_radio_t *)radio;
    
    /* Just set the short address */
    gm_radio->address_short = short_16;
    
    return RADIO_ERR_NOERR;
}

static uint16_t radio_getPAN_ID(radio_t *radio)
{
    /* Parse the generic radio structure to the internal Geomess radio-structure */
    gm_radio_t *gm_radio = (gm_radio_t *)radio;
    
    return gm_radio->pan_identifier;
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
static void gen_addr_ext(uint8_t buf[8])
{
    unsigned char macaddr[6] = { 0, 0, 0, 0xa, 0xb, 0x0 };
    unsigned char enc[2] = { 0xAA, 0xAB };
    uint16_t *macaddr_low = (uint16_t *) (macaddr + 2);
    *macaddr_low = (uint16_t)(*macaddr_low ^ (uint16_t)((uint16_t)getpid() & (uint16_t)0xFFFFU));
    
    memcpy(buf, macaddr, 3);
    memcpy((buf + 3), enc, 2);
    memcpy((buf + 5), (macaddr + 3), 3);
    
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
    gm_radio->radio.get_addr_ext = radio_getEUI64;
    gm_radio->radio.get_addr_short = radio_getSHORT16;
    gm_radio->radio.set_addr_short = radio_setSHORT16;
    gm_radio->radio.get_pan_id = radio_getPAN_ID;
	
	/* Set the short-ID by command-line options */
	gm_radio->address_short = id;
	
	/* Generate a random EUI64-address */
	gen_addr_ext(gm_radio->address_extended);
	
	/* Set the 802.15.4 specific parameters */
	gm_radio->pan_identifier = pan_identifier;
	gm_radio->pan_channel = pan_channel;
	
	return (radio_t *)gm_radio;
}
