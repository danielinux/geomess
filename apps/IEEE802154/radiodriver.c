#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>       

#include <sys/types.h>
#include <sys/poll.h>

#ifndef UNIT_TEST
#include "geomess.h"
#endif

#include "radiodriver.h"

#define DEBUG

#ifdef DEBUG
#define IEEE_DBG(s, ...) printf("[IEEE802.14.4]$ " s, ##__VA_ARGS__);
#define RADIO_DBG(s, ...) printf("[RADIODRIVER]$ " s, ##__VA_ARGS__);
#define dbg	printf
#else
#define dbg do {} while (0);
#define IEEE_DBG do {} while (0);
#define RADIO_DBG do {} while (0);
#endif

#define CHECK_PARAM(a, b)   if(!(a)){ \
                                dbg("[RADIODRIVER]$ %s %d\n", __FUNCTION__, (b)); \
                                return RADIO_ERR_EINVAL; \
                            }(void)a

/**
 *  Geomess specific radio-instance definition
 */
struct gm_radio
{
	struct ieee_radio radio;
	
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
	
	/**
	 *  DSI: Device Specific Information
	 */
	uint8_t		address_extended[8];
	uint16_t	address_short;
	
	/**
	 *  16-bit address pool to hand out short 
	 *	addresses from.
     *
     *  NOTE:
     *  Feel free to implement simulation of IEEE802.15.4
     *  PAN commissioning by simulating PAN association- and
     *  disassociation-events.
	 */
	//uint16_t	short_address_pool;
};

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
 *  @return int, how many bytes are sent
 */
static int radio_transmit(struct ieee_radio *radio, void *buf, int len)
{
    struct gm_radio *gm = NULL;
    uint16_t crc = 0;
    size_t ret = 0;
    
    CHECK_PARAM(radio, __LINE__);
    CHECK_PARAM(buf, __LINE__);

	/* Parse the generic radio structure to the internal Geomess radio-structure */
    gm = (struct gm_radio *)radio;
    
    /* Calculate the FCS */
    crc = calculate_crc16(buf + 1, len - 3); /* buf + 1 to skip the length-byte */
    
    memcpy(buf + len - 2, (void *)&crc, 2);
    
    /* buf + 1 to skip the length-byte, len - 1 to don't cause overflow */
    if ((ret = geomess_send(gm->conn, buf + 1, (uint32_t)(len - 1))) > 0) {
        /* Do nothing, frame is sent */
    }
    
    /* Send the payload over this radio's geomesh-connection */
    return ret;
}

/**
 *  Simulates an Address Filter of an IEEE802.15.4. Performs address filtering
 *  based on the Radio's PAN ID, IEEE802.15.4 16-bit short address or the 64-bit
 *  extended address. Broadcast frames will pass through the Address Filter.
 *
 *  @param buf Received MAC frame, stripped of the PHY header and FCS
 *  @param len Length of the received MAC frame
 *
 *  @return 1 when the frame is indeed filtered and didn't pass through, 0 when
 *          the frame is not filtered and can be send on to the higher layer.
 */
static int radio_filter_frame(struct ieee_radio *radio, uint8_t buf[IEEE_MAC_MTU], uint8_t len)
{
    struct gm_radio *gm = (struct gm_radio *)radio;
    struct ieee_hdr *hdr = (struct ieee_hdr *)buf;
    struct pico_ieee_addr addr;
    
    /* Parse in the destination address */
    addr = pico_ieee_addr_from_hdr(hdr, 0);
    
    /* But first, check the destination PAN-ID */
    if (hdr->pan == gm->pan_identifier) {
        /* If the PAN-ID matches, check for broadcast */
        if (IEEE_AM_SHORT == addr._mode && (0xFFFF == addr._short.addr || addr._short.addr == gm->address_short)) {
            /* Pass broadcast frame as well as frame for which the destination address is the same as this device's short */
            return 0;
        } else if (IEEE_AM_EXTENDED == addr._mode && (0 == memcmp(addr._ext.addr, gm->address_extended, PICO_SIZE_IEEE_EXT))) {
            /* Pass frame for which the extended destination address matches this device's extended address */
            return 0;
        } else {
            /* None of the addresses matches this device's addresses and the frame isn't broadcast, do nothing */
        }
    } else {
        /* PAN-ID doesn't match, do nothing */
    }
    
    /* Filter and bail out */
    memset(buf, 0, IEEE_MAC_MTU);
    return 1;
}

/**
 *  Radio driver's RX function
 *
 *  @param radio radio_t *, radio-driver instance
 *  @param buf   uint8_t[128], buffer to fill with received data
 *
 *  @return
 */
static enum radio_rcode radio_receive(struct ieee_radio *radio, uint8_t buf[IEEE_PHY_MTU])
{
	/* Parse the generic radio structure to the internal Geomess radio-structure */
	struct gm_radio *gm = (struct gm_radio *)radio;
	struct pollfd pfd;
    int ret = 0;
	
	/* Get the file-descriptor of this device's connection to the Geomesh-network */
	pfd.fd = geomess_datafd(gm->conn);
	
	/* Set the events to poll */
	pfd.events = POLLIN;
	
	/* Poll the file-descriptors, and return if nothing is selected */
	if (poll(&pfd, 1, 0) <= 0)
		return -1;
    
    /* 6LoWPAN excepts a 128-byte buffer with len + payload + fcs */
    if ((ret = geomess_recv(gm->conn, buf + 1, IEEE_PHY_MTU - 1)) < 0)
        return RADIO_ERR_ERX;
    
    buf[0] = (uint8_t)ret;
    if (ret > 0) {
        /* Let the packet pass through the address-filter */
        if (radio_filter_frame(radio, buf + 1, ret - 2)) {
            /* If the frame is filtered make sure the length returned is zero */
            memset(buf, 0, IEEE_PHY_MTU);
        }
    }
    
	/* If a file-descriptor is selected, retrieve data from it */
	return RADIO_ERR_NOERR;
}

/**
 *  Fills a buffer with the extended 64-bit address of a radio-instance
 *
 *  @param radio radio_t *, radio instance to get the address of.
 *  @param buf   uint8_t[8], buffer to fill with the extended address.
 *
 *  @return radio_rcode_t, see "pico_sixlowpan_dev.h"
 */
static enum radio_rcode radio_addr_ext(struct ieee_radio *radio, uint8_t buf[8])
{
	/* Parse the generic radio structure to the internal Geomess radio-structure */
	struct gm_radio *gm = (struct gm_radio *)radio;

	memcpy(buf, gm->address_extended, 8);
	
	return 0;
}

/**
 *  Get the short 16-bit address of a radio-instance
 *
 *  @param radio radio_t *, radio-instance to get the address of.
 *
 *  @return uint16_t, short 16-bit address of the radio-instance.
 */
static uint16_t radio_addr_short(struct ieee_radio *radio)
{
    /* Parse the generic radio structure to the internal Geomess radio-structure */
    struct gm_radio *gm = (struct gm_radio *)radio;
    
    return gm->address_short;
}

/**
 *  Set the short 16-bit address of a radio-instance
 *
 *  @param radio    radio_t *, radio-instnace to set the address of.
 *  @param short_16 uint16_t, new 16-bit address of the radio-instance
 *
 *  @return radio_rcode_t, see "pico_sixlowpan_dev.h"
 */
static enum radio_rcode radio_addr_short_set(struct ieee_radio *radio, uint16_t short_16)
{
    /* Parse the generic radio structure to the internal Geomess radio-structure */
    struct gm_radio *gm = (struct gm_radio *)radio;
    
    /* Just set the short address */
    gm->address_short = short_16;
    
    return RADIO_ERR_NOERR;
}

/**
 *  Get the PAN identifier radio is connected to
 *
 *  @param radio uint16_t, PAN identifier of the radio
 *
 *  @return uint16_t, PAN-identifier of the radio
 */
static uint16_t radio_pan_id(struct ieee_radio *radio)
{
    /* Parse the generic radio structure to the internal Geomess radio-structure */
    struct gm_radio *gm = (struct gm_radio *)radio;
    return gm->pan_identifier;
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
 *  Creates a radio-driver instance to pass to the 6LoWPAN layer.
 *  In this specific case this will create a gm_radio_t and set up
 *  a connection with the Geomess-network. The gm_radio_t will get
 *  casted to a proper radio_t * that you can give to the 6LoWPAN 
 *  adaption layer.
 *
 *  [TODO]: Simulate IEEE802.15.4 network commissioning and joining
 *
 *  @param id             uint16_t, identifier in the Geomess-network,
 *                        will also be the 16-bit short address of the 
 *                        6LoWPAN-node.
 *  @param pan_identifier uint16_t, identification number of the PAN to
 *                        to set up or to join.
 *  @param x              uint32_t, geographical X-coördinate of the radio
 *  @param y              uint32_t, geographical Y-coördinate of the radio
 *  @param range_max      uint32_t, maximum radio-range
 *  @param range_good     uint32_t, radio-range wherein the communication
 *                        never fails.
 *
 *  @return radio_t *, radio-driver instance you can pass to 6LoWPAN.
 */

struct ieee_radio *radio_create(uint16_t id, uint16_t pan_identifier, uint32_t x, uint32_t y, uint32_t range_max, uint32_t range_good)
{
	/* Create the radio-instance */
	struct gm_radio *gm = (struct gm_radio *)PICO_ZALLOC(sizeof(struct gm_radio));
    if (!gm) {
        IEEE_DBG("ERROR: Could not allocate Geomess radio-instance\n");
		return NULL;
    }
	
	/* Try to make a connection with the geomess-network */
    if (!(gm->conn = geomess_open(id, x, y, range_max, range_good))) {
        IEEE_DBG("ERROR: Could not open geomess connection\n");
        return NULL;
    }
	
	/* Set the callbacks of this radio-instance */
    gm->radio.transmit = radio_transmit;
    gm->radio.get_addr_short = radio_addr_short;
    gm->radio.get_pan_id = radio_pan_id;
	gm->radio.receive = radio_receive;
    gm->radio.get_addr_ext = radio_addr_ext;
    gm->radio.set_addr_short = radio_addr_short_set;
	
	/* Set the short-ID by command-line options */
	gm->address_short = id;
	
	/* Generate a random EUI64-address */
	gen_addr_ext(gm->address_extended);
	
	/* Set the 802.15.4 specific parameters */
	gm->pan_identifier = pan_identifier;
	
	return (struct ieee_radio *)gm;
}
