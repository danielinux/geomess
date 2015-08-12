#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>

#include "radiodriver.h"

#define MTU 127

int radio_transmit(radio_t *radio, const void *buf, int len)
{
	/* Parse the generic radio structure to the internal Geomess radio-structure */
	gm_radio_t *gm_radio = (gm_radio_t *)radio;
	
	/* Send the payload over this radio's geomesh-connection */
	return geomess_send(gm_radio->conn, buf, (uint32_t)len);
}

int radio_receive(radio_t *radio, void *buf, int len) {
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

radio_t *radio_create(uint16_t id, uint32_t x, uint32_t y, uint32_t range_max, uint32_t range_good)
{
	/* Create the radio-instance */
	gm_radio_t *gm_radio = (radio_t *)malloc(sizeof(gm_radio_t));
	if (!gm_radio)
		return NULL;
	
	/* Try to make a connection with the geomess-network */
	gm_radio->conn = geomess_open(id, x, y, range_max, range_good);
	if (!(gm_radio->conn))
		return NULL;
	
	/* Set the callbacks of this radio-instance */
	gm_radio->radio.transmit = radio_transmit;
	gm_radio->radio.receive = radio_receive;
	
	return (radio_t *)gm_radio;
}
