#include <dm9000.h>
#include <string.h>
#include <net.h>
#include <mini-boot.h>

unsigned short checksum(unsigned char *ptr, int len)
{
    unsigned long sum = 0;
    unsigned short *p = (unsigned short *)ptr;

    while (len > 1)
    {
        sum += *p++;
        len -= 2;
    }
    
    if(len == 1)
        sum += *(unsigned char *)p;
    
    while(sum>>16)
        sum = (sum&0xffff) + (sum>>16);
    
    return (unsigned short)((~sum)&0xffff);
}

void mac_send( unsigned char *buf, unsigned int len, unsigned short proto)
{
    buf -= sizeof(struct eth_hdr);
    len += sizeof(struct eth_hdr);
    
    struct eth_hdr *p = (struct eth_hdr *)(buf);
    
    memcpy (p->s_mac, mac_addr, 6);
    memcpy (p->d_mac, host_mac_addr, 6); 
    
    p->type = htons(proto);
     
    dm9000_send_packet( buf, len );
}

void ip_send( unsigned char *buf, unsigned int len )
{
    buf -= sizeof(struct ip_hdr);					//增加IP头部
    len += sizeof(struct ip_hdr);
    
    struct ip_hdr *p = (struct ip_hdr*)(buf);
    
    p->vhl = 0x45;                         				 
    p->tos = 0x00;                              		
    p->len = htons(len);       							
    p->ipid = htons(0x00);           					
    p->ipoffset = htons(0x4000);               
    p->ttl = 0xFF;                             
    p->proto = 17;        							//上层协议采用UDP                  	
	memcpy(p->srcipaddr,ip_addr,4);
	memcpy(p->destipaddr,host_ip_addr,4);
    p->ipchksum = 0x0;                          
    p->ipchksum = checksum( buf, sizeof(struct ip_hdr) );
    mac_send( buf, len, PROTO_IP );
}

void udp_send( unsigned char *buf, unsigned int len, unsigned short port )
{
    buf -= sizeof(struct udp_hdr);						//增加udp头部
    len += sizeof(struct udp_hdr);

	struct udp_hdr *p = (struct udp_hdr*)(buf);
    p->sport  = htons(0x3000);    						// 2 Byte,suppose we use 0x8da4 port
    p->dport = htons(port);   							// 2 Byte
    p->len = htons(len);       							// 2 Byte 
    p->udpchksum  = 0x00;     							// Do Not Checksum, 2 Byte 

    ip_send( buf, len );
}

void arp_request(void) 								//send a ARP request packet
{
	struct arp_hdr *arpbuf;
	arpbuf = (struct arp_hdr *)&send_buffer[256];
	
	arpbuf->hwtype = htons( 1 );					//the type of send hardware address,if 1 means yi tai wang di zhi
	arpbuf->protocol = htons( 0x0800 );				//means ip------>arp
	arpbuf->hwlen = 6;
	arpbuf->protolen = 4;
	arpbuf->opcode = htons( 1 );					//arp request
	memcpy(arpbuf->smac, mac_addr, 6);
	memcpy(arpbuf->sipaddr, ip_addr, 4);
	memcpy(arpbuf->dipaddr, host_ip_addr, 4);
	send_packet_len = 28;
	mac_send(&send_buffer[256], send_packet_len ,PROTO_ARP );
}

static int udp_handle( unsigned char *buf, unsigned int len )
{
    struct udp_hdr *p;
    p = (struct udp_hdr *)buf;
       
    buf += sizeof(struct udp_hdr);
    len -= sizeof(struct udp_hdr);

    tftp_handle( buf, len, htons(p->sport) );
    
    return 0;
}

unsigned long ip_handle(unsigned char *buf, unsigned int len)
{
	struct ip_hdr *p;
	p = (struct ip_hdr *)buf;    
		
	if((p->destipaddr[0] != ip_addr[0]) || \
	   (p->destipaddr[1] != ip_addr[1]) || \
	   (p->destipaddr[2] != ip_addr[2]) || \
	   (p->destipaddr[3] != ip_addr[3]))
	   {
	   		//printf("it is not my ip packet!\n\r");
	   		return -1;
	   }
	   
	   if(p->vhl != 0x45)
	   	{	
	   		printf("it is not a IPV4 packet!\n\r");
	   		return -1;
	   	}
	   	
    switch ( p->proto )
    {	
    	
        case PROTO_UDP:
            buf += sizeof(struct ip_hdr);
            len -= sizeof(struct ip_hdr);
            udp_handle( buf, len );
            break;
        case PROTO_TCP:
        	printf("we don't handle the TCP packets!\n\r");
            break;
        default:
        	printf("IP_PROTO NOT MACTHED\n\r");
            break;
    }

    return 0;   	
}

unsigned char arp_handle(unsigned char *buf, unsigned int len)
{
	struct arp_hdr *pRx, *pTx;	
    pRx = (struct arp_hdr *)(buf);
    pTx = (struct arp_hdr *)&send_buffer[512];

	if(len<28)
		{
			printf("\narp packet length is not right!\n\r");
			return 0;
		}
	switch (htons(pRx->opcode))
	{
		case 1:													//处理ARP请求
			if((pRx->dipaddr[0] == ip_addr[0]) && \
			   (pRx->dipaddr[1] == ip_addr[1]) && \
			   (pRx->dipaddr[2] == ip_addr[2]) && \
			   (pRx->dipaddr[3] == ip_addr[3]) )
			   
			   {
			 
			   	pTx->hwtype = htons(1);
				pTx->protocol = htons(0x0800);					//代表IP协议
				pTx->hwlen = 6;
				pTx->protolen = 4;
				pTx->opcode = htons(2);							//ARP应答报文
				memcpy(pTx->sipaddr,ip_addr,4);					//将我们的IP，MAC地址发给请求方
				memcpy(pTx->smac,mac_addr,6);
				memcpy(pTx->dipaddr,pRx->sipaddr,4);			//将收到的源方的IP地址，MAC地址变为目标地址
			   	memcpy(pTx->dmac, pRx->smac,6);

			   	mac_send( (unsigned char*)pTx, sizeof(struct arp_hdr), PROTO_ARP);
   				}
			   break;
		case 2:													//处理arp应答
			if((pRx->dipaddr[0] == ip_addr[0]) && \
			   (pRx->dipaddr[1] == ip_addr[1]) && \
			   (pRx->dipaddr[2] == ip_addr[2]) && \
			   (pRx->dipaddr[3] == ip_addr[3]) )
			   			   
			   memcpy(host_mac_addr,pRx->smac,6);
			   break;
		default:
				printf("\n\r arp_opcode Not Support.\n");
				break;
	}
}


void net_receive( unsigned char *buf, unsigned int len )
{
    struct eth_hdr *p = (struct eth_hdr *)(buf);

    switch ( htons(p->type) )
    {
        case PROTO_ARP:
            buf += sizeof(struct eth_hdr);
            len -= sizeof(struct eth_hdr);
            arp_handle( buf, len );
            break;
        case PROTO_IP:
            buf += sizeof(struct eth_hdr);
            len -= sizeof(struct eth_hdr);
            ip_handle( buf, len ); 
            break;
        default:
            printf("\r\tMAC_opcode type error (=0x%04X)!\n", p->type);
            break;
    }
}

