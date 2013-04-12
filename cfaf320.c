#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include "typedefs.h"
#include "GrLCD.h"
#include "cfaf320.h"

String_T strg = { 0, 0, 0, 0, 0, 0, 0 };    // string structure for text display
char txtbuf[40];

/*************************************************/
void delay(WORD t)
{
    BYTE t1;

    while(t--)
    {
        for(t1 = 11; t1 > 0; t1--)
        {
            asm("nop");
        }
    }
}

/*************************************************/
void display_rgb(WORD data)
{
    WORD i,j;

    LCDSetGCur(0, 0);
    for(i = 0; i < NR_ROWS; i++)
    {
        for(j = 0; j < NR_COLS; j++)
        {
            LCDData(data);
        }
    }
}

/*************************************************/
int main(void)
{
    DIR_MD = OUTS;          // Mode Port, Output
    CTRL = 0xFF;            // all control bits high
    DIR_CT = OUTS;          // Control Port, Output
    DIR_HI = OUTS;          // Data 8-15, Output
    DIR_LO = OUTS;          // Data 0-7, Output

#ifdef BIT16
    MODE = M80_16;          // 8080 16 bit mode
#else
    MODE = M80_8;           // 8080 8 bit mode
#endif

    LCDInit();

    display_rgb(BLUE);      // paint background color

    strg.font = FONT_G;
    strg.cj_code = C50J + NOBG;
    strg.row = 5;
    strg.col = 0;
    strg.fg = WHITE;
    strg.bg = BLUE;
    strg.text = txtbuf;
    sprintf(txtbuf, "Now is the");
    LCDText(&strg);         // display text

    strg.cj_code = C50J + ROT270 + NOBG;
    strg.row = 5;
    sprintf(txtbuf, "time for");
    LCDText(&strg);         // display text

    strg.cj_code = C50J + ROT180 + NOBG;
    sprintf(txtbuf, "all good men");
    LCDText(&strg);         // display text

    strg.cj_code = C50J + ROT90 + NOBG;
    sprintf(txtbuf, "to come");
    LCDText(&strg);         // display text

    strg.font = FONT_L;
    strg.cj_code = C50J + NOBG;
    strg.row = 50;
    strg.fg = YELLOW;
    sprintf(txtbuf, "to the aid of");
    LCDText(&strg);         // display text

    strg.cj_code = C50J + ROT270 + NOBG;
    sprintf(txtbuf, "their party");
    LCDText(&strg);         // display text

    strg.cj_code = C50J + ROT180 + NOBG;
    sprintf(txtbuf, "1234567890.");
    LCDText(&strg);         // display text

    strg.cj_code = C50J + ROT90 + NOBG;
    sprintf(txtbuf, "Fun with");
    LCDText(&strg);         // display text

    strg.font = FONT_H;
    strg.cj_code = C50J + NOBG;
    strg.row = 90;
    strg.fg = GREEN;
    sprintf(txtbuf, "Fonts !");
    LCDText(&strg);         // display text

    while (1)
    {
        PORTD ^= 2;         // blink the led
        delay(0xFFFF);
    }
    return 0;
}
/*************************************************/
