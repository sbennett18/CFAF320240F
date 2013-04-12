#ifndef MSP430_DEV_H_
#define MSP430_DEV_H_

#include <msp430.h>

#define PROGMEM /* empty */
#define pgm_read_byte(x)    (*(x))
#define pgm_read_word(x)    (*(x))

typedef unsigned char   BYTE;
typedef unsigned int    WORD;   // a 16 bit value, processor dependant
typedef unsigned char   PBYTE;

#define DLY __delay_cycles(1)

/************************************/
/* Ports for LCD control & data bus */
/************************************/
/* Control Bits */
#define RD          BIT7
#define WR          BIT6
#define CD          BIT5
#define CS          BIT4
#define RES         BIT3

#define SET(arg1)   CTL |= (arg1)
#define CLR(arg1)   CTL &= ~(arg1)

/* Control Ports */
#define DIR_MD      P4DIR
#define MODE        P4OUT
#define DIR_CTL     P3DIR
#define CTL         P3OUT

/* Data Ports */
#define DIR_HI      P1DIR
#define OUT_HI      P1OUT
#define IN_HI       P1IN
#define DIR_LO      P2DIR
#define OUT_LO      P2OUT
#define IN_LO       P2IN

/* Port Direction States */
#define OUTS        0xFF
#define INS         0x00

/*************************/
/* Miscellaneous Defines */
/*************************/
#define DIR_LED     P6DIR
#define OUT_LED     P6OUT
#define PIN_LED     BIT0

#endif /* MSP430_DEV_H_ */
