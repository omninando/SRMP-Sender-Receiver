#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>

#include "srmp.h"

#define  RCVR_MAXWIN 5000     /* feel free to modify */
#define  RAND_MAX 2147483647   /* = 2^31-1 */

/* Global file descriptor for the output file. */

int outFile;

/**********************************************************************
 **********************************************************************
 **********************************************************************/
/*
 * Open a UDP connection.
 */
int udp_open(char *remote_IP_str, int remote_port, int local_port)
{
    int      fd;
    uint32_t dst;
    struct   sockaddr_in sin;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        fprintf(stderr, "Error creating unnamed socket\n");
        exit(1);
    }

    /* Bind the local socket to listen at the local_port. */
    printf ("Binding locally to port %d\n", local_port);
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(local_port);
    if (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return (-1);
    }

    /* Connect, i.e. prepare to accept UDP packets from <remote_host, remote_port>.  */
    /* Listen() and accept() are not necessary with UDP connection setup.            */ 
    dst = hostname_to_ipaddr(remote_IP_str);
    printf ("Establishing UDP connection to <%u, port %d>\n", dst, remote_port);
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(remote_port);
    sin.sin_addr.s_addr = dst;
    if (connect(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("connect");
        return(-1);
    }
    return (fd);
}

/**********************************************************************
 **********************************************************************
 **********************************************************************/
int rmp_close ()
{
 return 2;
}

/**********************************************************************
 **********************************************************************
 **********************************************************************/
void rmp_send (int fd, int type, unsigned short window, unsigned short seqno, char* data, int len)
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

/**********************************************************************
 **********************************************************************
 **********************************************************************/
int rmp_open (char *dst, int sport, int rport, char *fname)
{

    char bytes[1000];
    int i=0;
    int a=1;
    int len=1000;
    int fd = udp_open(dst, sport, rport);

    FILE *p;
    char pkt1[SRMP_MAXPKT];
    char pkt[SRMP_MAXPKT];
    p = fopen(fname, "rb"); 
    int j=0;
    rmp_send(fd, 2, 2, 0, 0, 0); 
    while (feof(p) == 0)
    {
        if(a==0){ 
            fread(pkt1, 1000, 1, p);
            rmp_send (fd, 0, 2, a, pkt1, 1000);
            while ((len = readpkt(fd, pkt, sizeof(pkt))) <= 0) 
            ;
            a=1;
        }else if(a==1){ 
            fread(pkt1, 1000, 1, p);
            rmp_send (fd, 0, 2, a, pkt1, 1000);
            while ((len = readpkt(fd, pkt, sizeof(pkt))) <= 0) 
            ;
            a=0;
        }
    }
    fclose(p);
}

/**********************************************************************
 **********************************************************************
 **********************************************************************/
int main(int argc, char **argv)
{
	char dst[100];
    char fname[100];
    int sport, rport;
    if (argc != 5) { fprintf(stderr, "usage: SenderApp receiver-IP-address recv-port send-port file-name\n"); exit(1); }
    sport = atoi(argv[2]);
    rport = atoi(argv[3]);
    strcpy (fname, argv[4]);
    strcpy (dst, argv[1]);
    rmp_open(dst, sport, rport, fname);
    return 0;
}










