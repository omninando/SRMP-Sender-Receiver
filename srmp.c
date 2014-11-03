
/*
 *
 * Helper functions for Programming Assignment #1 
 *
 *
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "srmp.h"


/*
 * Convert a DNS name or numeric IP address into an integer value
 * (in network byte order).  This is more general-purpose than
 * inet_addr() which maps dotted pair notation to uint. 
 */
unsigned int
hostname_to_ipaddr(const char *s)
{
    if (isdigit(*s))
        return (unsigned int)inet_addr(s);
    else {
        struct hostent *hp = gethostbyname(s);
        if (hp == 0) {
            /* Error */
            return (0);
        } 
        return *((unsigned int **)hp->h_addr_list)[0];
    }
}

/*
 * Print an srmp packet to standard output.
 *   - dir is either 's'ent or 'r'eceived packet
 */
void
dump(int dir, char *pkt, int len)
{
    srmphdr *srh = (srmphdr*) pkt;
    unsigned short type = ntohs(srh->type);
    unsigned short seqno = ntohs(srh->seqno);
    unsigned short win = ntohs(srh->window);

    printf("%c %s seq %u win %u len %d\n", dir,
           (type == SRMP_DATA) ? "dat" : 
           (type == SRMP_ACK) ? "ack" : 
           (type == SRMP_SYN) ? "syn" : 
           (type == SRMP_FIN) ? "fin" : 
           (type == SRMP_RESET) ? "rst" : "???",
           seqno, win, len);

    /*XXX*/
    fflush(stdout);
}



/*
 * Helper function to send an srmp packet over the network.
 * As a side effect print the packet header to standard output.
 */
void
sendpkt(int fd, int type, unsigned short window, unsigned short seqno, char* data, int len)
{
    char wrk[SRMP_MAXPKT + sizeof(srmphdr)];
    srmphdr *srh = (srmphdr *)wrk;
    srh->type = htons(type);
    srh->window = htons(window);
    srh->seqno = htons(seqno);
    if (data != 0) {
        memcpy((char*)(srh + 1), data, len);
    }

    dump('s', wrk, len + sizeof(srmphdr));
    if (send(fd, wrk, len + sizeof(srmphdr), 0) < 0) {
        perror("write");
        exit(1);
    }
}


/*
 * Helper function to read an srmp packet from the network.
 * As a side effect print the packet header to standard output.
 */
int
readpkt(int fd, char* pkt, int len)
{
    int cc = read(fd, pkt, len);
    if (cc > 0) {
        dump('r', pkt, cc);
    }
    return (cc);
}

/*
 * Reset the network connection by sending an RESET packet, print an error
 * message to standard output, and exit.
 */
void
reset(int fd)
{
    fprintf(stderr, "protocol error encountered... resetting connection\n");
    sendpkt(fd, SRMP_RESET, 0, 0, 0, 0);
    exit(0);
}


/*
 * Read a packet from the network but if "ms" milliseconds transpire
 * before a packet arrives, abort the read attempt and return
 * SRMP_TIMED_OUT.  Otherwise, return the length of the packet read.
 */
int
readWithTimer(int fd, char *pkt, int ms)
{
    int s;
    fd_set fds;
    struct timeval tv;

    tv.tv_sec = ms / 1000;

    tv.tv_usec = (ms - tv.tv_sec * 1000) * 1000;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    s = select(fd + 1, &fds, 0, 0, &tv);
    if (FD_ISSET(fd, &fds)) {
        return (readpkt(fd, pkt, SRMP_MAXPKT));
    }
    else return (SRMP_TIMED_OUT);
}

/*
 * Set an I/O channel (file descriptor) to non-blocking mode.
 */
void nonblock(int fd)
{       
        int flags = fcntl(fd, F_GETFL, 0);
#if defined(hpux) || defined(__hpux)
        flags |= O_NONBLOCK;
#else
        flags |= O_NONBLOCK|O_NDELAY;
#endif
        if (fcntl(fd, F_SETFL, flags) == -1) {
                perror("fcntl: F_SETFL");
                exit(1);
        }
}

