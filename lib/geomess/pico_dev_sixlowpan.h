#ifndef UNIT_TEST
#include "geomess.h"
#endif
#include "pico_device.h"
#include "pico_stack.h"

#include <sys/poll.h>
#define GM_MTU 127

struct pico_device *pico_sixlowpan_create(uint16_t id, uint32_t x, uint32_t y, uint32_t range_max, uint32_t range_good);
