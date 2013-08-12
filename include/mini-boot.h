#ifndef LBOOT_H
#define LBOOT_H

int boot_linux_sdram();
int boot_linux_serial();
int boot_linux();

void s3c2440_nand_read(unsigned char *buf, unsigned int start_addr, int size);
void s3c2440_nand_init(void);

/********** Network Related Functions ************/
int tftp_download(unsigned char *addr, const char *filename);
void udp_send( unsigned char *buf, unsigned int len, unsigned short port );
void tftp_handle( unsigned char *buf, unsigned int len, unsigned short port );
void net_receive( unsigned char *buf, unsigned int len );
void arp_request(void);

// timer.c
int timer_init (void);
void msdelay(unsigned long msec);

//dm9000.c
unsigned char dm9000_reg_read(unsigned char reg);
void dm9000_init(void);
void dm9000_send_packet(unsigned char * datas, unsigned int length);
unsigned int dm9000_receive_packet();

// Serial
void init_uart(void);
void putc(unsigned char c);
unsigned char getc(void);
int GtSerialLoad(void *pLoadAddr);

#endif

