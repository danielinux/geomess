#ifndef INCLUDE_DEVICE_DRIVER
#define INCLUDE_DEVICE_DRIVER

#include "pico_dev_sixlowpan.h"

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
					  uint32_t  range_good);

#endif /* INCLUDE_DEVICE_DRIVER */