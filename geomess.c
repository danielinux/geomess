#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include "libevquick.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#include "geomess.h"

struct geomess_node {
	int fd; /* When the node is online, this is the fd of its socket */
    uint16_t id;
    uint32_t x;
    uint32_t y;
    uint32_t range_max;
    uint32_t range_good;
    evquick_event *e;
    struct geomess_node *next;
};

static struct geomess_node *NODELIST;


void read_cb(int fd, short int revents, void *_node);
void err_cb(int fd, short int revents, void *_node);

/**** 
 ** LIST UTILITIES
 ****/
void nodelist_add(struct geomess_node *new)
{
	new->next = NODELIST;
	NODELIST = new;
}

void node_connected(int fd)
{
	struct geomess_node *node = NULL;
	node = malloc(sizeof(struct geomess_node));
	if (!node) {
		close(fd);
		return;
	}
    memset(node, 0, sizeof(struct geomess_node));
	
	node->fd = fd;
	node->e = evquick_addevent(fd, EVQUICK_EV_READ, read_cb, err_cb, node);
    if (!node->e) {
        free(node);
        return;
    }
    node->id = 0xFFFF;
	nodelist_add(node);
}


void nodelist_del(struct geomess_node *tgt)
{
	struct geomess_node *cur = NODELIST;
	struct geomess_node *prev = NULL;
	while(cur) {
		if (cur == tgt) {
			if (prev == NULL) {
				NODELIST = cur->next;
			} else {
				prev->next = cur->next;
			}
			break;
		}
		prev = cur;
		cur = cur->next;
	}
	evquick_delevent(tgt->e);
	free(tgt);
}

struct geomess_node *get_node(uint16_t id)
{
	struct geomess_node *cur = NODELIST;
	while(cur) {
        if (id == cur->id)
			return cur;	
		cur = cur->next;
	}
	return NULL;
}

void login(struct geomess_node *from, struct geomess_msg *msg)
{
    int fd = open("map.csv", O_WRONLY|O_CREAT|O_APPEND, 0664);
    from->id = ntohs(msg->id);
    from->x = (double)ntohl(msg->info.login.x);
    from->y = (double)ntohl(msg->info.login.y);
    from->range_max = (double)ntohl(msg->info.login.range_max);
    from->range_good = (double)ntohl(msg->info.login.range_good);
    if (fd >= 0) {
        char line[1000];
        snprintf(line, 1000, "%lu,%lu,%lu,%lu\n", from->x, from->y, from->range_max,from->range_good);
        (void)write(fd, line, strlen(line));
        close(fd);
    }

}

static double getrange(struct geomess_node *from)
{
    double range;
    if (from->range_max <= from->range_good)
        return from->range_max;
    range = 1.0 * from->range_good + (1.0 * from->range_max - 1.0 * from->range_good) * drand48();
    return range;
}

static int inrange(struct geomess_node *from, struct geomess_node *to)
{
    double x1, y1, x2, y2;
    double range = getrange(from);
    x1 = (double)(from->x);
    y1 = (double)(from->y);
    x2 = (double)(to->x);
    y2 = (double)(to->y);

    //printf("Range:%lf, distance %lf\n", range, sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)));

    if ( pow(x1 - x2, 2) + pow(y1 - y2, 2) < pow(range, 2))
        return 1;
    else 
        return 0;

}


void deliver(struct geomess_node *from, struct geomess_msg *msg)
{
    struct geomess_node *to = NODELIST;
    if (from->id != ntohs(msg->id))
        return;

    while(to) {
        if ((to != from) && (inrange(from, to)) ) {
            write(to->fd, msg, MAXLEN);
        }
        to = to->next;
    }
}

int parse_cmd(struct geomess_node *from, void *cmd)
{
    struct geomess_msg *msg = cmd;
    if (from->id == 0xFFFF) {
        if (ntohs(msg->type) == GEOMESS_LOGIN) {
            login(from, msg);
            return 0;
        }
    } else {
        if (ntohs(msg->type) == GEOMESS_DATA) {
            deliver(from, msg);
            return 0;
        }
    }
    return -1;
}

void read_cb(int fd, short int revents, void *_node)
{
	struct geomess_node *node = (struct geomess_node *) _node;
	char cmd[MAXLEN];
	int r;
	
	r = read(fd, cmd, MAXLEN);
	if (r > 0) {
		cmd[r] = (char) 0;
		parse_cmd(node, cmd);
	}
}

void err_cb(int fd, short int revents, void *_node)
{
	struct geomess_node *node = (struct geomess_node *) _node;
	nodelist_del(node);
}


/* This is triggered upon new connections */
void connect_cb(int fd, short revents, void *arg)
{
	struct sockaddr_un client;
	int socklen = sizeof(struct sockaddr_un);
	int new_fd = accept(fd, (struct sockaddr *)&client, &socklen);
	if (new_fd >= 0) {
		node_connected(new_fd);
	}
}

void connect_failure(int fd, short revents, void *arg)
{
	fprintf(stderr, "Error accepting a new connection: %s \n", strerror(errno));
	exit(5);
}

int main(void) 
{
	evquick_init();
    unlink("map.csv");
	
	// Create socket
	int s_server;
	struct sockaddr_un socket_server;

	mkdir("/tmp/msg", 0666);
	s_server = socket(AF_UNIX, SOCK_STREAM, 0);
	
	// Bind socket
	socket_server.sun_family = AF_UNIX;
	strcpy(socket_server.sun_path, SOCKET_PATH);
	unlink(socket_server.sun_path);
	bind(s_server, (struct sockaddr *) &socket_server, sizeof(socket_server));
	
	// listen socket
	listen(s_server, 3);
	evquick_addevent(s_server, EVQUICK_EV_READ, connect_cb, connect_failure, NULL);
	evquick_loop();
	/* This should never return... */
	
	return 0;

}





