#include <stdlib.h>
#include "typedefs.h"
#include "cfaf320.h"
#include "GrLCD.h"
#include "Font8.h"
#include "Font16.h"
#include "helvn6.h"
#include "helvb10.h"
#include "timesb12.h"


/***************************************
  LCD display constants
 ***************************************/
static const WORD JustHrz[8] = {    // text location table
    0, NR_COLS/4,                   // L00J,L25J
    NR_COLS/3, NR_COLS/2,           // L33J,C50J
    (NR_COLS*2)/3, (NR_COLS*3)/4,   // R66J,R75J
    NR_COLS, 0                      // R100J,ABSJ
};
static const WORD JustVrt[8] = {    // text location table
    0, NR_ROWS/4,                   // L00J,L25J
    NR_ROWS/3, NR_ROWS/2,           // L33J,C50J
    (NR_ROWS*2)/3, (NR_ROWS*3)/4,   // R66J,R75J
    NR_ROWS, 0                      // R100J,ABSJ
};

BYTE font;                          // text font in use
BYTE CJ_code = 0;                   // control/justify code for text display
BYTE dsrow;                         // starting row for text
WORD dscol;                         // last column written to
WORD fgcolor, bgcolor;              // colors
WORD gr_x = 0;                      // X origin of pixel/line
WORD gr_y = 0;                      // Y origin of pixel/line

static const WORD LCDIni[] =
{                                   // all page #s refer to SSD2119 datasheet
    0x0028, 0x0006,                 // VCOM OTP Page 55-56
    0x0000, 0x0001,                 // start Oscillator Page 36
    0x0010, 0x0000,                 // Sleep mode Page 49
    0x0001, 0x72EF,                 // Driver Output Control Page 36-39
    0x0002, 0x0600,                 // LCD Driving Waveform Control Page 40-42
    0x0003, 0x6A38,                 // Power Control 1 Page 43-44
    0x0011, 0x6870,                 // Entry Mode Page 50-52
    0x000F, 0x0000,                 // Gate Scan Position Page 49
    0x000B, 0x5308,                 // Frame Cycle Control Page 45
    0x000C, 0x0003,                 // Power Control 2 Page 47
    0x000D, 0x000A,                 // Power Control 3 Page 48
    0x000E, 0x2E00,                 // Power Control 4 Page 48
    0x001E, 0x00BE,                 // Power Control 5 Page 53
    0x0025, 0x8000,                 // Frame Frequency Control Page 53
    0x0026, 0x7800,                 // Analog setting Page 54
    0x0044, 0xEF00,                 // Vertical RAM address position Page 57
    0x0045, 0x0000,                 // Horizontal RAM address position	Page 57
    0x0046, 0x013F,                 // Horizontal RAM address position Page 57
    0x004E, 0x0000,                 // Ram Address Set Page 58
    0x004F, 0x0000,                 // Ram Address Set Page 58
    0x0012, 0x08D9,                 // Sleep mode Page 49

    0x0030, 0x0000,                 // Gamma Control (R30h to R3Bh)
    0x0031, 0x0104,                 // Page 56
    0x0032, 0x0100,
    0x0033, 0x0305,
    0x0034, 0x0505,
    0x0035, 0x0305,
    0x0036, 0x0707,
    0x0037, 0x0300,
    0x003A, 0x1200,
    0x003B, 0x0800,

    0x0007, 0x0033,                 // Display Control  Page 45
    0xFFFF, 0xFFFF
};


/***********************************************************
  Initialize display
 ***********************************************************/
void LCDInit(void)
{
    const WORD * ptr;

    SET(CS + RD + WR + CD);
    CLR(RES);                       // assert hardware reset
    delay(100);
    SET(RES);                       // de-assert reset

    ptr = &LCDIni[0];
    while (*ptr < 0xFFFF)
    {
        LCDCmd(*(ptr++));
        LCDData(*(ptr++));
    }
    LCDClear(0, NR_ROWS);           // clear data mamory
}


/***********************************************************
  Write command to LCD controller
 ***********************************************************/
void LCDCmd(WORD arg)
{
    CLR(CS);
    CLR(CD);

#ifdef BIT16
    OUT_HI = arg >> 8;
    OUT_LO = arg & 0xFF;
    CLR(WR);
    SET(WR);
#else
    OUT_HI = arg >> 8;
    CLR(WR);
    SET(WR);

    OUT_HI = arg & 0xFF;
    CLR(WR);
    SET(WR);
#endif
    SET(CS);
}

/***********************************************************
  Write data to LCD controller
 ***********************************************************/

void LCDData(WORD arg)
{
    CLR(CS);
    SET(CD);

#ifdef BIT16
    OUT_HI = arg >> 8;
    OUT_LO = arg & 0xFF;
    CLR(WR);
    SET(WR);
#else
    OUT_HI = arg >> 8;
    CLR(WR);
    SET(WR);

    OUT_HI = arg & 0xFF;
    CLR(WR);
    SET(WR);
#endif
    SET(CS);
}

/***********************************************************
  Read pixel from LCD controller
 ***********************************************************/

WORD LCDRead(void)
{
    BYTE argh, argl;

    CLR(CS);
    SET(CD);
    DIR_HI = INS;                   // set hi port to inputs
#ifdef BIT16
    DIR_LO = INS;                   // set lo port to inputs
    CLR(RD);                        // assert read
    DLY;
    DLY;
    argl = IN_LO;
    argh = IN_HI;                   // get word from the bus
    SET(RD);                        // de-assert read
    DIR_LO = OUTS;                  // set lo port to outputs
#else
    CLR(RD);                        // assert read
    DLY;
    DLY;
    argl = IN_HI;                   // get byte from the bus
    SET(RD);                        // de-assert read
    DLY;
    DLY;
    CLR(RD);                        // assert read
    DLY;
    DLY;
    argh = IN_HI;                   // get byte from the bus
    SET(RD);                        // de-assert read
#endif
    DIR_HI = OUTS;                  // set hi port to outputs
    SET(CS);
    return (WORD)((argh << 8) | argl);
}


/***********************************************************
  Clear the display data RAM
  row = starting row
  hgt = number of rows
 ***********************************************************/
void LCDClear(WORD row, WORD hgt)
{
    WORD a, b;

    LCDSetGCur(row, 0);
    OUT_HI = 0;                     // put black on the bus
    OUT_LO = 0;
    CLR(CS);
    SET(CD);
    for (a = 0; a < hgt; a++) {     // each row
        for (b = 0; b < NR_COLS; b++) { // all columns
            CLR(WR);
            SET(WR);
#ifndef BIT16
            DLY;
            CLR(WR);
            SET(WR);
#endif
        }
    }
    SET(CS);
}

/***********************************************************
  Write text string to display.
Sample: String_T str1 = { FONT_M, C50J, 2, 0, RED, BLACK, "Hello World" };

strg = pointer to String_T structure
 ***********************************************************/

void LCDText(String_T *strg)
{
    WORD a, b;
    char *cp;

    font = strg->font;
    CJ_code = strg->cj_code;            // save CJ code
    dsrow = strg->row;                  // save starting row number
    fgcolor = strg->fg;
    bgcolor = strg->bg;
    b = CJ_code & JUSTIFY;              // isolate justify mode

    if (b == L00J) dscol = 0;
    else if (b == ABSJ) dscol = strg->col;
    else {
        a = LCDPixLen(strg->text);      // get display length in pixels
        if (b != R100J)                 // if not right justify
        {
            a /= 2;                     // use half length for centering
        }
        dscol = ((CJ_code & ROT90) ? JustVrt[b] : JustHrz[b]) - a;  // set starting column
    }

    if (dscol >= ((CJ_code & ROT90) ? NR_ROWS : NR_COLS)) { // if over/underflow
        dscol = 0;                      // use left justify
    }
    cp = strg->text;                    // get pointer to the text

    b = (CJ_code & ROTN);               // get rotation angle
    if (b == 0) a = WRTDWN;             // get entry mode command
    else if (b == ROT90) a = WRTRGT;    // for SSD2119 register 11h
    else if (b == ROT180) a = WRTUP;    // to set writing direction
    else a = WRTLFT;                    // 270

    LCDCmd(0x11);                       // set entry mode, to write
    LCDData(a);                         // pixels, column by column

    while (*cp) {
        LCDChar(*cp++);                 // output chars to screen
    }

    LCDCmd(0x11);
    LCDData(WRTRGT);                    // set default entry mode
}


/***********************************************************
  Write character bitmap to display. Font data is expected
  to be in columns, a byte for every 8 rows, LSB is top row.
  Font data and tables are stored in flash (program memory).
  "pgm_read_byte()" and "pgm_read_word()" are macros in
  <pgmspace.h> to fetch a byte from flash instead of SRAM.
 ***********************************************************/
void LCDChar(char chr)
{
    BYTE wid, a, b, c, d, rw;
    BYTE flag, size;
    const PBYTE * bitp;                     // pointer to flash memory space
    BYTE bits, mask;

    switch (font) {                         // get the font information
        case FONT_M:                        // med font
            if (chr < firstchr_M) return;   // char code out of range
            a = chr - firstchr_M;
            if (a >= nr_chrs_M) return;     // char code out of range

            wid = pgm_read_byte(&widtbl_M[a]);              // number of columns
            bitp = (PBYTE *)pgm_read_word(&chrtbl_M[a]);    // bitmap data pointer
            size = chr_hgt_M;               // cell height
            break;

        case FONT_L:                        // large font
            if (chr < firstchr_L) return;   // char code out of range
            a = chr - firstchr_L;
            if (a >= nr_chrs_L) return;     // char code out of range

            wid = pgm_read_byte(&widtbl_L[a]);              // number of columns
            bitp = (PBYTE *)pgm_read_word(&chrtbl_L[a]);    // bitmap data pointer
            size = chr_hgt_L;               // cell height
            break;

        case FONT_G:                        // great font
            if (chr < firstchr_G) return;   // char code out of range
            a = chr - firstchr_G;
            if (a >= nr_chrs_G) return;     // char code out of range

            wid = pgm_read_byte(&widtbl_G[a]);              // number of columns
            bitp = (PBYTE *)pgm_read_word(&chrtbl_G[a]);    // bitmap data pointer
            size = chr_hgt_G;               // cell height
            break;

        case FONT_H:                        // huge font
            if (chr < firstchr_H) return;   // char code out of range
            a = chr - firstchr_H;
            if (a >= nr_chrs_H) return;     // char code out of range

            wid = pgm_read_byte(&widtbl_H[a]);              // number of columns
            bitp = (PBYTE *)pgm_read_word(&chrtbl_H[a]);    // bitmap data pointer
            size = chr_hgt_H;               // cell height
            break;

        default:                            // small font
            if (chr < firstchr_S) return;   // char code out of range
            a = chr - firstchr_S;
            if (a >= nr_chrs_S) return;     // char code out of range

            wid = pgm_read_byte(&widtbl_S[a]);              // number of columns
            bitp = (PBYTE *)pgm_read_word(&chrtbl_S[a]);    // bitmap data pointer
            size = chr_hgt_S;               // cell height
            break;
    }

    if (wid == 0) return;

    if (chr != ' ') {                       // don't process space char
        d = size / 8;                       // number of bytes per column
        for (a = 0; a < wid; a++) {         // do columns
            LCDSetTCur(dsrow, dscol + a);   // set starting row, column
            flag = 0;
            for (b = 0; b < d; b++) {       // do column bytes
                bits = pgm_read_byte(bitp + b + (d * a));   // get column byte
                mask = 1;                   // start with D0 (top pixel of col)
                rw = dsrow + b * 8;
                for (c = 0; c < 8; c++) {   // do bits
                    if (mask & bits) {      // put fg pixel
                        if (flag) {         // cursor addr need to be set
                            LCDSetTCur(rw + c, dscol + a);  // set starting row, column
                            flag = 0;
                        }
                        LCDData(fgcolor);
                    } else {
                        if (! (CJ_code & NOBG)) {   // put bg pixel
                            if (flag) { // cursor addr need to be set
                                LCDSetTCur(rw + c, dscol + a);	// set starting row, column
                                flag = 0;
                            }
                            LCDData(bgcolor);
                        } else flag = 1;      // skip write, flag for addr update
                    }
                    mask <<= 1;             // next bit
                }   // end do bits
            }   // end do column bytes
        }   // end do columns
    }   // end if not space


    if (! (CJ_code & NOSPC)) {              // space at end of char
        if (! (CJ_code & NOBG)) {           // put bg pixels
            LCDSetTCur(dsrow, dscol + wid); // set starting row, column
            for (a = 0; a < size; a++) {
                LCDData(bgcolor);
            }
        }
        wid++;                              // add space to char wid
    }
    dscol += wid;                           // update current column
}

/***********************************************************
  Erase a block of pixels, using bgcolor
 ***********************************************************/

void LCDErase(WORD row, WORD col, WORD hgt, WORD wid)
{
    WORD a, b, c;

    b = row + hgt;
    for (c = row; c <= b; c++)              // do rows
    {
        LCDSetGCur(c, col);
        CLR(CS);
        SET(CD);
#ifdef BIT16
        OUT_HI = bgcolor >> 8;
        OUT_LO = bgcolor & 0xFF;
#endif
        for (a = 0; a < wid; a++)           // do columns
        {
#ifdef BIT16
            CLR(WR);
            SET(WR);
#else
            OUT_HI = bgcolor >> 8;
            CLR(WR);
            SET(WR);

            OUT_HI = bgcolor & 0xFF;
            CLR(WR);
            SET(WR);
#endif
        }
        SET(CS);
    }
}


/***********************************************************
  Read a block of pixels from screen into memory array
 ***********************************************************/
void LCDRdBlock(WORD *mem, WORD row, WORD col, WORD hgt, WORD wid)
{
    WORD a, c;

    for (c = row; c <= row + hgt; c++) {
        LCDSetGCur(c, col);
        LCDRead();                          // do dummy read
        for (a = 0; a < wid; a++) {
            *mem++ = LCDRead();
        }
    }
}


/***********************************************************
  Write a block of pixels from memory array to screen
 ***********************************************************/
void LCDWrBlock(WORD *mem, WORD row, WORD col, WORD hgt, WORD wid)
{
    WORD a, c;

    for (c = row; c <= row + hgt; c++) {
        LCDSetGCur(c, col);
        for (a = 0; a < wid; a++) {
            LCDData(*mem++);
        }
    }
}


/***********************************************************
  Write cursor address to controller
 ***********************************************************/
void LCDSetGCur(WORD row, WORD col)
{
    if (col > LASTCOL || row > LASTROW) return;
    LCDCmd(0x4E);                           // RAM X address set
    LCDData(col);
    LCDCmd(0x4F);                           // RAM Y address set
    LCDData(row);
    LCDCmd(0x22);                           // access to display RAM
}


/***********************************************************
  Write cursor address to controller
 ***********************************************************/
void LCDSetTCur(WORD row, WORD col)
{
    BYTE a;
    WORD x, y;

    a = CJ_code & ROTN;
    if (a == 0) {
        x = col;
        y = row;
    } else if (a == ROT90) {
        x = row;
        y = LASTROW - col;
    } else if (a == ROT180) {
        x = LASTCOL - col;
        y = LASTROW - row;
    } else {    // ROT270
        x = LASTCOL - row;
        y = col;
    }
    if (x > LASTCOL || y > LASTROW) return;
    LCDCmd(0x4E);                           // RAM X address set
    LCDData(x);
    LCDCmd(0x4F);                           // RAM Y address set
    LCDData(y);
    LCDCmd(0x22);                           // access to display RAM
}


/***********************************************************
  Get width of display text, in pixels
 ***********************************************************/
WORD LCDPixLen(char *chr)
{
    WORD wid;
    BYTE first, nrc, txt;
    const PBYTE *widtbl;

    switch (font) {
        case FONT_H:
            first = firstchr_H;
            widtbl = widtbl_H;
            nrc = nr_chrs_H;
            break;

        case FONT_G:
            first = firstchr_G;
            widtbl = widtbl_G;
            nrc = nr_chrs_G;
            break;

        case FONT_L:
            first = firstchr_L;
            widtbl = widtbl_L;
            nrc = nr_chrs_L;
            break;

        case FONT_M:
            first = firstchr_M;
            widtbl = widtbl_M;
            nrc = nr_chrs_M;
            break;

        default:
            first = firstchr_S;
            widtbl = widtbl_S;
            nrc = nr_chrs_S;
            break;
    }

    wid = 0;

    while (*chr) {
        txt = *chr;
        if ((txt >= first) && ((txt - first) < nrc)) {
            wid += pgm_read_byte(&widtbl[txt - first]);

            if ( !(CJ_code & NOSPC) ) {
                wid++;                      // include blank space
            }
        }
        chr++;
    }
    return wid;
}


/***********************************************************
  Plot a pixel at X,Y
  0,0 is at upper left
 ***********************************************************/
void plot(WORD x, WORD y)
{
    LCDSetGCur(y, x);                       // address the pixel
    LCDData(fgcolor);
}


/***********************************************************
  Plot a pixel at X,Y and save point
 ***********************************************************/
void point(WORD x, WORD y)
{
    gr_x = x;
    gr_y = y;
    plot(x, y);
}


/***********************************************************
  Draw a line from saved X,Y to X2,Y2 and save new endpoint
  Bresenham's algorithm
 ***********************************************************/
void lineto(WORD x2, WORD y2)
{
    int i, delta_x, delta_y, numpixels;
    int d, dinc1, dinc2;
    int x, xinc1, xinc2;
    int y, yinc1, yinc2;

                                            // Calculate delta_x and delta_y for initialisation
    delta_x = abs(x2 - gr_x);
    delta_y = abs(y2 - gr_y);

                                            // Initialize all vars based on which is the independent variable
    if (delta_x >= delta_y) {               // x is independent variable
        numpixels = delta_x + 1;
        d = (delta_y << 1) - delta_x;
        dinc1 = delta_y << 1;
        dinc2 = (delta_y - delta_x) << 1;
        xinc1 = 1;
        xinc2 = 1;
        yinc1 = 0;
        yinc2 = 1;
    } else {  // y is independent variable
        numpixels = delta_y + 1;
        d = (delta_x << 1) - delta_y;
        dinc1 = delta_x << 1;
        dinc2 = (delta_x - delta_y) << 1;
        xinc1 = 0;
        xinc2 = 1;
        yinc1 = 1;
        yinc2 = 1;
    }

    if (gr_x > x2) {                        // Make sure x and y move in the right directions
        xinc1 = - xinc1;
        xinc2 = - xinc2;
    }
    if (gr_y > y2) {
        yinc1 = - yinc1;
        yinc2 = - yinc2;
    }


    x = gr_x;                               // Start drawing from previous endpoint
    y = gr_y;
    gr_x = x2;                              // save new endpoint for next line
    gr_y = y2;

    // Draw the pixels
    for (i = 0; i < numpixels; i++)
    {
        plot(x, y);
        if (d < 0) {
            d = d + dinc1;
            x = x + xinc1;
            y = y + yinc1;
        } else {
            d = d + dinc2;
            x = x + xinc2;
            y = y + yinc2;
        }
    }
}
