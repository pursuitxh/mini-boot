/*
 * =====================================================================================
 *
 *       Filename:  tftp.c
 *
 *    Description:  tftp client application
 *
 *        Version:  1.0
 *        Created:  2009年05月12日 18时28分35秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Weimeng, pursuitxh@163.com
 *        Company:  Sourth East University
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <string.h>
#include <net.h>

struct tftp_package *pGtftp = NULL;
unsigned int g_len = 0;
unsigned short TftpServerPort = 0;

static void tftp_send_request(unsigned char *buf, const char *filename)
{
    int len = 0;
    buf[0] = 0x00;							//读请求
    buf[1] = 0x01;
    len += 2;
    sprintf(&buf[len], "%s", filename);
    len += strlen(filename);
    buf[len] = '\0';
    len += 1;
    sprintf(&buf[len], "%s", "octet");
    len += strlen("octet");
    //sprintf(&buf[len], "%s", "netascii");
    //len += strlen("netascii");
    buf[len] = '\0';
    len += 1;
    udp_send(buf, len, 69);					//tftp采用端口号69,协议规定的
}

static void tftp_send_ack(unsigned char *buf, unsigned short blocknum)
{
    struct tftp_package *A = (struct tftp_package *)buf;
    A->opcode = htons(0x04);									//ACK包
    A->u.blocknum = htons(blocknum);
    udp_send(buf, sizeof(struct tftp_package), TftpServerPort);
}

void tftp_handle( unsigned char *buf, unsigned int len, unsigned short port )	
{
    pGtftp = (struct tftp_package *)buf; 
    g_len = len; 
    TftpServerPort = port;
}

int tftp_download(unsigned char *addr, const char *filename)
{  
    int i=0;
    unsigned short curblock = 1;						//第一块数据block data

    tftp_send_request(&send_buffer[256], filename );    //请求下载文件,为什么是从256开始,因为发送数据时要添加头部

    msdelay(100);

    while (1)
    {
        dm9000_receive_packet();
        if( pGtftp == NULL )										//查询的方式，看有没有收到数据
            continue;
        if ( ntohs(pGtftp->opcode) == TFTP_DATA )					//收到的是数据包
        {
            if (ntohs(pGtftp->u.blocknum) == curblock)  			//是第一个数据包，而不是ACK包
            {
                printf("\rCurrent Block Number = %d", curblock);
                for (i=0; i<g_len-4; i++)							//减4就是减去opcode和block的长度，共4个字节
                {
                    *(addr++) = *(pGtftp->data+i);
                }
                tftp_send_ack( &send_buffer[256], curblock);
                
                if (g_len < TFTP_DATASIZE+4)				//一样的道理，512只是数据的长度，iGLen还包括opcode和block的长度
                {											//数据全部收完，退出
                    break;
                }
                curblock += 1;								//记录传送的数据包数量
            }
            else if (ntohs(pGtftp->u.blocknum) < curblock)	//收到的是ACK包,发一个ACK应答包
            {
                tftp_send_ack( &send_buffer[256], ntohs(pGtftp->u.blocknum)); 
            }
            else
            {
                printf("\n\rBlock Number Not Match.");                
            }
        }
        else if ( ntohs(pGtftp->opcode) == TFTP_ERROR ) //收到的是错误包,进行错误处理
        {
            switch( ntohs(pGtftp->u.errcode) )
            {
                case ENOTFOUND:
                    printf(" Error: File NOT Found.\n\r");
                    break;
                case EBADOP:
                    printf(" Error: Bad TFTP Operation.\n\r");
                    break;
                case EBADID:
                    printf(" Error: Unknown Transfer ID.\n\r");
                    break;
                default:
                    printf(" Error: ..... \n\r");
                    break;
            } 
        }
        else if ( ntohs(pGtftp->opcode) == TFTP_RRQ ) 
        {
            printf(" Get TFTP RRQ Packet.\n\r");
        }
        else if ( ntohs(pGtftp->opcode) == TFTP_WRQ ) 
        {
            printf(" Get TFTP WRQ Packet.\n\r");
        }
        else if ( ntohs(pGtftp->opcode) == TFTP_ACK ) 
        {
            printf(" Get TFTP ACK Packet.\n\r");
        }
        else 
        {
            //printf(" Unknown TFTP Packet.");
            //printf(" Opcode = %d.\n\r", htons(pGtftp->opcode));
        }

        pGtftp = NULL;											//进行下一次包的处理
        g_len = 0;
    }
    
    printf("\n\rTransfer complete: %d Bytes.\n\r", (curblock-1)*TFTP_DATASIZE + g_len-4 );//iGLen是最后一次数据的长度
    
    return 0;

}

