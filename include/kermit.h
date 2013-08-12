#ifndef KERM_H
#define KERM_H

#define MARK_START  0x1
#define MARK_EXIT   0x3

#define KERM_TYPE_DATA  'D'
#define KERM_TYPE_SEND  'S'
#define KERM_TYPE_ACK   'Y'
#define KERM_TYPE_NACK  'N'
#define KERM_TYPE_BREAK 'B'

#define KERM_KEY_SPACE   0x20
#define KERM_KEY_SHARP   0x23
#define KERM_KEY_TERM    0x0d  /* '\n' */

#define ENC_PRINT(c) (c + KERM_KEY_SPACE)
#define DEC_PRINT(c) (c - KERM_KEY_SPACE)

#endif
