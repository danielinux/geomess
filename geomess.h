#include <stdint.h>
#ifndef GEOMESS_H
#define GEOMESS_H
/* move to geomess.h */
#define GEOMESS_LOGIN   0x00
#define GEOMESS_DATA    0xA0
#define MAXLEN 2048

#define SOCKET_PATH "/tmp/geomess.sock"
#define DATABUF_OFFSET (6)
struct __attribute__((packed)) geomess_msg {
    uint16_t signature;
    uint16_t tot_size;
    uint16_t id;
    uint16_t type;
    union gmsg_info {
        struct gmsg_info_data {
            uint16_t len;
            unsigned char buf[0];
        } data;
        struct gmsg_info_login {
            uint32_t x;
            uint32_t y;
            uint32_t range_max;
            uint32_t range_good;
        } login;
    } info;
};

struct geomess_client;
typedef struct geomess_client* GEOMESS;
GEOMESS geomess_open(uint16_t id, uint32_t x, uint32_t y, uint32_t range_max, uint32_t range_good);
int geomess_send(GEOMESS g, uint8_t *data, int len);
int geomess_recv(GEOMESS g, uint8_t *data, int len);
int geomess_datafd(GEOMESS g);
#endif
