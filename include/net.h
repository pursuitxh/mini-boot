#ifndef _NET_H
#define _NET_H

extern unsigned char mac_addr[6];
extern unsigned char ip_addr[4];
extern unsigned char host_ip_addr[4];
extern unsigned char host_mac_addr[6];

extern unsigned char send_buffer[1024];
extern unsigned char recv_buffer[1024];
extern unsigned long send_packet_len;
extern unsigned long recv_packet_len;

#define TFTP_DATASIZE         512             		  /* data segment size */

#define TFTP_RRQ     0x01                              /* read request */
#define TFTP_WRQ     0x02                              /* write request */
#define TFTP_DATA    0x03                              /* data packet */
#define TFTP_ACK     0x04                              /* acknowledgement */
#define TFTP_ERROR   0x05                              /* error code */

// Tftp Error Code
#define EUNDEF          0               /* not defined */
#define ENOTFOUND       1               /* file not found */
#define EACCESS         2               /* access violation */
#define ENOSPACE        3               /* disk full or allocation exceeded */
#define EBADOP          4               /* illegal TFTP operation */
#define EBADID          5               /* unknown transfer ID */
#define EEXISTS         6               /* file already exists */
#define ENOUSER         7               /* no such user */


#define PROTO_TCP       0x06
#define PROTO_UDP       0x11
#define PROTO_ARP 		0x0806
#define PROTO_IP  		0x0800


struct eth_hdr
{
	unsigned char d_mac[6];
	unsigned char s_mac[6];
	unsigned short type;
}__attribute__ ((packed));

struct arp_hdr
{
	unsigned short hwtype;
	unsigned short protocol;
	unsigned char hwlen;
	unsigned char protolen;
	unsigned short opcode;
	unsigned char smac[6];
	unsigned char sipaddr[4];
	unsigned char dmac[6];
	unsigned char dipaddr[4];
}__attribute__ ((packed));

struct ip_hdr
{
	unsigned char vhl;
	unsigned char tos;
	unsigned short len;
	unsigned short ipid;
	unsigned short ipoffset;
	unsigned char ttl;
	unsigned char proto;
	unsigned short ipchksum;
	unsigned char srcipaddr[4];
	unsigned char destipaddr[4];
}__attribute__ ((packed));

struct udp_hdr
{
	unsigned short sport;
	unsigned short dport;
	unsigned short len;
	unsigned short udpchksum;
}__attribute__ ((packed));

struct tftp_package
{
    unsigned short opcode;                      /* packet type */
    union
    {
        unsigned short blocknum;                /* block # */
        unsigned short errcode;                 /* error code */
    } u;
    unsigned char  data[0];                     /* data or error string */
}__attribute__ ((packed));


#define ETH_TBUF ((struct eth_hdr *)(&transmit_buffer[0]))
#define ETH_RBUF ((struct eth_hdr *)(&receive_buffer[0]))
#define ARP_TBUF ((struct eth_hdr *)(&transmit_buffer[0]))
#define ARP_RBUF ((struct eth_hdr *)(&receive_buffer[0]))
#define IP_TBUF ((struct eth_hdr *)(&transmit_buffer[0]))
#define IP_RBUF ((struct eth_hdr *)(&receive_buffer[0]))
#define UDP_TBUF ((struct eth_hdr *)(&transmit_buffer[0]))
#define UDP_RBUF ((struct eth_hdr *)(&receive_buffer[0]))

//把2字节的Little Endian转换为Big Endian
#define htons(x) (unsigned short)( \
((((unsigned short)(x) & (unsigned short)0x00FF)) << 8) | \
((((unsigned short)(x) & (unsigned short)0xFF00)) >> 8) )

//把4字节的Little Endian转换为Big Endian
#define htonl(x) (unsigned long)( \
((((unsigned long)(x) & (unsigned long)0x000000FF)) << 24) | \
((((unsigned long)(x) & (unsigned long)0x0000FF00)) <<  8) | \
((((unsigned long)(x) & (unsigned long)0x00FF0000)) >>  8) | \
((((unsigned long)(x) & (unsigned long)0xFF000000)) >> 24) )

#define ntohs htons
#define ntohl htohl

#endif

