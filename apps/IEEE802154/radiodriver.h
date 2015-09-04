#ifndef INCLUDE_DEVICE_DRIVER
#define INCLUDE_DEVICE_DRIVER

#include "pico_dev_sixlowpan.h"

/**
 *  Creates a radio-driver instance to pass to the 6LoWPAN layer.
 *  In this specific case this will create a gm_radio_t and set up
 *  a connection with the Geomess-network. The gm_radio_t will get
 *  casted to a proper radio_t * that you can give to the 6LoWPAN
 *  adaption layer.
 *
 *  [TODO]: Simulate IEEE802.15.4 network commissioning and joining
 *          by means of
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
struct ieee_radio *radio_create(uint16_t id, uint16_t pan_identifier, uint32_t x, uint32_t y, uint32_t range_max, uint32_t range_good);

#endif /* INCLUDE_DEVICE_DRIVER */