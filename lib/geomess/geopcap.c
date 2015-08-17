#include <stdio.h>
#include <pcap/pcap.h>
#include "geomess.h"
#include <stdlib.h>

static char errbuf[PCAP_ERRBUF_SIZE];

pcap_t *pcap_create(const char *source, char *errbuf);

int main(int argc, char *argv[]) 
{
    uint32_t x,y;
    uint16_t id = 0xFFFE; /* Fixed ID for the sniffer thing. */
    pcap_t *pcap;
    pcap_dumper_t *pcapd;
    GEOMESS g;

    if (argc != 4)
        exit(1);
    x = (uint32_t)atoi(argv[1]);
    y = (uint32_t)atoi(argv[2]);

    pcap = pcap_open_dead(DLT_IEEE802_15_4, 65535);
    if (!pcap) {
        perror("pcap");
        exit(2);
    }

    pcapd = pcap_dump_open(pcap, argv[3]);
    if (!pcapd){ 
        perror("pcapd");
        exit(3);
    }
    g = geomess_open(id, x, y, 0, 0);
    if (!g) {
        perror("geomess_open");
        exit(1);
    }

    for(;;) {
        uint8_t msg[2048];
        int ret = geomess_recv(g, msg, 2048);
        struct pcap_pkthdr ph;

        if (ret > 0) {
            printf("Dump one packet to file, %d bytes.\n", ret);
            gettimeofday(&ph.ts, NULL);
            ph.caplen = ret;  
            ph.len =  ret;
            pcap_dump((u_char *)pcapd, &ph, msg);
            printf("done.\n");
            pcap_dump_flush(pcapd);
        }
    }
}
