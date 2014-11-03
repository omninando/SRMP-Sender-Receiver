
#ifndef __SRMP_H_

#define __SRMP_H_

#define SRMP_MAXWIN    65535 
#define SRMP_MAXPKT     1000
#define SRMP_TIMED_OUT  (-1)

/*
 * Packet types
 */
#define SRMP_DATA   0
#define SRMP_ACK	  1
#define SRMP_SYN    2
#define SRMP_FIN	  3
#define SRMP_RESET  4

/*
 * State types
 */
/* shared */
#define SRMP_ESTABLISHED   0
#define SRMP_CLOSED	       1
/* sender */
#define SRMP_SYN_SENT      2
#define SRMP_FIN_WAIT      3
#define SRMP_CLOSING       4
/* receiver */
#define SRMP_LISTEN	       5
#define SRMP_TIME_WAIT     6

/* A packet buffer has a seqno, a length and data */
typedef struct pktbuf_tag {

  struct pktbuf_tag *next;
  
  unsigned short int seqno;
  int len;
  char data[SRMP_MAXPKT];

} pktbuf;



/* The header's three fields are as described in the handout */
typedef struct {
    unsigned short int type; 
    unsigned short int window; 
    unsigned short int seqno;
} srmphdr;

/*  All of the receiver's state is stored in the following structure,
 *  including the received messages, which have to be delivered to
 *  ReceiveApp in order.
 */
typedef struct {
  int state;		/* protocol state: normally ESTABLISHED */
  int fd;		      /* UDP socket descriptor */

  unsigned short rwnd;		/* latest advertised window */
  
  unsigned short NBE;	/* next byte expected */
  unsigned short LBRead;        /* last byte read */
  unsigned short LBReceived;  /* last byte received */

  unsigned short ISN; /* initial sequence number */

  pktbuf *recvQueue;  /* Pointer to the first node of the receive queue */

}srmp_recv_ctrl_blk;


/* Declarations for SRMP.C */
void sendpkt(int fd, int type, unsigned short window, unsigned short seqno, char* data, int len);
int readpkt(int fd, char* pkt, int len);
void dump(int dir, char *pkt, int len);
unsigned int hostname_to_ipaddr(const char *s);
int readWithTimer(int fd, char *pkt, int ms);

/* Declarations for RECEIVER_LIST.C */
void add_packet(srmp_recv_ctrl_blk *info, unsigned short seqno, int len, char *data);
pktbuf *get_packet(srmp_recv_ctrl_blk *info, unsigned short seqno);
void free_packet(pktbuf *pbuf);

/* Declarations for WRAPAROUND.C */
int greater(unsigned short val1, 
	    unsigned short val2);
unsigned short minus(unsigned short greaterVal, 
		     unsigned short lesserVal);


#endif
