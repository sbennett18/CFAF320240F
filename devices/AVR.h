#ifndef AVR_DEV_H_
#define AVR_DEV_H_

#include <avr/pgmspace.h>
#include <avr/io.h>

typedef unsigned char  BYTE;
typedef unsigned short WORD;    // a 16 bit value, processor dependant
typedef unsigned char  PROGMEM PBYTE;

#define DLY \
asm ("NOP"); \
asm ("NOP");

/************************************/
/* Ports for LCD control & data bus */
/************************************/
/* Control Bits */
#define RD          0x80
#define WR          0x40
#define CD          0x20
#define CS          0x10
#define RES         0x08

#define SET(arg1)   CTL |= (arg1)
#define CLR(arg1)   CTL &= ~(arg1)

/* Control Ports */
#define DIR_MD      DDRD
#define MODE        PORTD
#define DIR_CTL     DDRC
#define CTL         PORTC

/* Data Ports */
#define DIR_HI      DDRA
#define OUT_HI      PORTA
#define IN_HI       PINA
#define DIR_LO      DDRE
#define OUT_LO      PORTE
#define IN_LO       PINE

/* Port Direction States */
#define OUTS        0xFF
#define INS         0x00

/*************************/
/* Miscellaneous Defines */
/*************************/
#define DIR_LED     DDRD
#define OUT_LED     PORTD
#define PIN_LED     0x02

#endif /* AVR_DEV_H_ */
