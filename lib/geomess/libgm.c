#include <stdio.h>
#include <string.h>
#include "libevquick.h"
#include <sys/socket.h>
#include <linux/un.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#include "geomess.h"

struct geomess_client {
    uint16_t id;
    int fd;
};

static unsigned char pktbuf[MAXLEN];

GEOMESS geomess_open(uint16_t id, uint32_t x, uint32_t y, uint32_t range_max, uint32_t range_good)
{
  struct sockaddr_un addr;
  int s;
  struct geomess_msg msg;
  GEOMESS g;
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, UNIX_PATH_MAX);

  s = socket(AF_UNIX, SOCK_STREAM, 0);

  if (s < 0) {
    return NULL;
  }

  if (connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) != 0) {
      return NULL;
  }

  msg.id = htons(id);
  msg.type = htons(GEOMESS_LOGIN);
  msg.info.login.x = htonl(x);
  msg.info.login.y = htonl(y);
  msg.info.login.range_max = htonl(range_max);
  msg.info.login.range_good = htonl(range_good);
  g = malloc(sizeof(struct geomess_client));
  if (g) {
    g->fd = s;
    g->id = id;
  }
  write(s, &msg, MAXLEN);
  return g;
}

int geomess_send(GEOMESS g, uint8_t *data, int len)
{
    struct geomess_msg *msg = (struct geomess_msg *)pktbuf; 
    msg->id = htons(g->id);
    msg->type = htons(GEOMESS_DATA);
    msg->info.data.len = htons(len);
    memcpy(msg->info.data.buf, data, len);
    return write(g->fd, pktbuf, MAXLEN);
}

int geomess_recv(GEOMESS g, uint8_t *data, int len)
{
    struct geomess_msg *msg = (struct geomess_msg *)pktbuf; 
    int r;
    r = read(g->fd, pktbuf, MAXLEN);
    if (r > DATABUF_OFFSET) {
        int size = ntohs(msg->info.data.len);
        if (size > len)
            return -1;
        memcpy(data, msg->info.data.buf, size);
        return size;
    }
    return -1;
}
int geomess_datafd(GEOMESS g)
{
    return g->fd;
}

#ifdef TEST
int main(void)
{
    GEOMESS g;
    int r;
    char msg[500];
    g = geomess_open(1,2,3,4,5);
    if (g)
        printf("Up and running!\n");
    else {
        perror("Open");
        exit(1);
    }
    r = geomess_send(g, "Hello",5);
    if (r < 0) 
        perror("Send\n");
    else
        printf("send returned %d\n", r);

    while(1) {
        int ret;
        geomess_send(g, "Hello", 5);
        ret = geomess_recv(g, msg, 500);
        if (ret > 0) {
            msg[ret] = (unsigned char)0;
            printf("Received %s\n", msg);
        }
        sleep(1);
    }

}


#endif
