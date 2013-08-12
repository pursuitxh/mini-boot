#include <kermit.h>
#include <s3c2440.h>
#include <stdio.h>

#define TXD0READY   (1<<2)
#define RXD0READY   (1)

inline unsigned char GtUartReadByte()
{
    unsigned char ret; 
    while(!(UTRSTAT0 & RXD0READY));
    ret = URXH0;
    return ret;
}

inline void GtUartWriteByte(unsigned char c)
{
    while (!(UTRSTAT0 & TXD0READY));
    UTXH0 = c;
}

#define KERM_ACK_LEN 0x10

static void send_ack_packet(unsigned int seq)
{
	unsigned char buf[KERM_ACK_LEN];
	int index = 0, check_sum = 0;


	buf[index++] = MARK_START;
	buf[index++] = ENC_PRINT(3);
	buf[index++] = seq + KERM_KEY_SPACE;
	buf[index++] = KERM_TYPE_ACK;

	buf[index] = '\0';

	index = 1;
	while (buf[index])
	{
		check_sum += buf[index];
		index++;
	}

	buf[index++] = (KERM_KEY_SPACE + (0x3f & (check_sum + (0x03 & (check_sum >> 6))))) & 0xff;
	buf[index++] = KERM_KEY_TERM;
	buf[index] = '\0';

#ifdef CONFIG_TOPHALF_KERMIT
	UartPuts((const char *)buf);
#else
	index = 0;
	while (buf[index])
		GtUartWriteByte(buf[index++]);
#endif
}

#define KERM_BUF_LEN   128

int GtSerialLoad(void *pLoadAddr)
{
	unsigned char buf[KERM_BUF_LEN];
	unsigned char curr_char;
	int index;
	int check_sum, len, seq, type;
	unsigned char *pAddr = (unsigned char *)pLoadAddr;


	do {
		while(MARK_START != GtUartReadByte());

		index = 0;
		while(KERM_KEY_TERM != (buf[index] = GtUartReadByte()))
			index++;

		index = 0;

		/* length decode */
		len = buf[index++];
		check_sum = len;
		len -= KERM_KEY_SPACE;

		/* sequence decode */
		seq = buf[index++];
		check_sum += seq;
		seq -= KERM_KEY_SPACE;

		/* get package type */
		type = buf[index++];
		check_sum += type;

		if (len) //fixme: handle extended length
			len -= 2;

		while (len > 1) {
			curr_char = buf[index++];
			check_sum += curr_char;
			len--;
			if (type != KERM_TYPE_DATA)
				continue;
			if (curr_char == KERM_KEY_SHARP) /* '#' */
			{
				curr_char = buf[index++];
				check_sum += curr_char;
				len--;

				if (0x40 == (curr_char & 0x60))
					curr_char = curr_char & (~0x40);
				else if (0x3f == (curr_char & 0x7f))
					curr_char |= 0x40;
			}

			*pAddr++ = curr_char;
		}

		/* checksum */
		curr_char = buf[index++];
		if (curr_char != (KERM_KEY_SPACE + (0x3f & (check_sum + (0x03 & (check_sum >> 6))))))
#ifdef CONFIG_TOPHALF_KERMIT
			goto Loop;
#else
			goto Error;
#endif

		/* terminator */
		curr_char = buf[index++];
		if (curr_char != KERM_KEY_TERM)
#ifdef CONFIG_TOPHALF_KERMIT
			goto Loop;
#else
			goto Error;
#endif

		/* send ack package */
		send_ack_packet(seq);
	}while(type != KERM_TYPE_BREAK);

	return pAddr - (unsigned char *)pLoadAddr;

#ifdef CONFIG_TOPHALF_KERMIT
Loop:
	while (1)
            GtUartWriteByte('E');
#else
Error:
#endif
	printf("Error\n\r");
        return -1;
}


