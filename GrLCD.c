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
 * LCD display constants
 ***************************************/
static const WORD JustHrz[8] = {    // Text location table
    0, NR_COLS/4,                   // L00J,L25J
    NR_COLS/3, NR_COLS/2,           // L33J,C50J
    (NR_COLS*2)/3, (NR_COLS*3)/4,   // R66J,R75J
    NR_COLS, 0                      // R100J,ABSJ
};
static const WORD JustVrt[8] = {    // Text location table
    0, NR_ROWS/4,                   // L00J,L25J
    NR_ROWS/3, NR_ROWS/2,           // L33J,C50J
    (NR_ROWS*2)/3, (NR_ROWS*3)/4,   // R66J,R75J
    NR_ROWS, 0                      // R100J,ABSJ
};

BYTE font;                          // Text font in use
BYTE CJ_code = 0;                   // Control/justify code for text display
BYTE dsrow;                         // Starting row for text
WORD dscol;                         // last column written to
WORD fgcolor, bgcolor;              // Colors
WORD gr_x = 0;                      // X origin of pixel/line
WORD gr_y = 0;                      // Y origin of pixel/line

static const WORD LCDIni[] =
{                                   // All page #s refer to SSD2119 datasheet
    0x0028, 0x0006,                 // VCOM OTP Page 55-56
    0x0000, 0x0001,                 // Start Oscillator Page 36
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
 * Initialize display
 ***********************************************************/
void LCDInit(void)
{
    const WORD * ptr;

    SET(CS + RD + WR + CD);
    CLR(RES);                       // Assert hardware reset
    delay(100);
    SET(RES);                       // De-assert reset

    ptr = &LCDIni[0];
    while (*ptr < 0xFFFF)
    {
        LCDCmd(*(ptr++));
        LCDData(*(ptr++));
    }
    LCDClear(0, NR_ROWS);           // Clear data memory
}


/***********************************************************
 * Write command to LCD controller
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
    DIR_HI = INS;                   // Set hi port to inputs
#ifdef BIT16
    DIR_LO = INS;                   // Set lo port to inputs
    CLR(RD);                        // Assert read
    DLY;
    DLY;
    argl = IN_LO;
    argh = IN_HI;                   // Get word from the bus
    SET(RD);                        // De-assert read
    DIR_LO = OUTS;                  // Set lo port to outputs
#else
    CLR(RD);                        // Assert read
    DLY;
    DLY;
    argl = IN_HI;                   // Get byte from the bus
    SET(RD);                        // De-assert read
    DLY;
    DLY;
    CLR(RD);                        // Assert read
    DLY;
    DLY;
    argh = IN_HI;                   // Get byte from the bus
    SET(RD);                        // De-assert read
#endif
    DIR_HI = OUTS;                  // Set hi port to outputs
    SET(CS);
    return (WORD)((argh << 8) | argl);
}


/***********************************************************
 * Clear the display data RAM
 * 	row = starting row
 * 	hgt = number of rows
 ***********************************************************/
void LCDClear(WORD row, WORD hgt)
{
    WORD a, b;

    LCDSetGCur(row, 0);
    OUT_HI = 0;                     // Put black on the bus
    OUT_LO = 0;
    CLR(CS);
    SET(CD);
    for (a = hgt; a > 0; a--) {     // Each row
        for (b = 0; b < NR_COLS; b++) { // All columns
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
 * Write text string to display.
 * 	Sample: String_T str1 = { FONT_M, C50J, 2, 0, RED, BLACK, "Hello World" };
 *
 * 	strg = pointer to String_T structure
 ***********************************************************/
void LCDText(String_T *strg)
{
    WORD a, b;
    char *cp;

    font = strg->font;
    CJ_code = strg->cj_code;            // Save CJ code
    dsrow = strg->row;                  // Save starting row number
    fgcolor = strg->fg;
    bgcolor = strg->bg;
    b = CJ_code & JUSTIFY;              // Isolate justify mode

    if (b == L00J) dscol = 0;
    else if (b == ABSJ) dscol = strg->col;
    else {
        a = LCDPixLen(strg->text);      // Get display length in pixels
        if (b != R100J)                 // If not right justify
        {
            a /= 2;                     // Use half length for centering
        }
        dscol = ((CJ_code & ROT90) ? JustVrt[b] : JustHrz[b]) - a;  // Set starting column
    }

    if (dscol >= ((CJ_code & ROT90) ? NR_ROWS : NR_COLS)) { // If over/underflow
        dscol = 0;                      // Use left justify
    }
    cp = strg->text;                    // Get pointer to the text

    b = (CJ_code & ROTN);               // Get rotation angle
    if (b == 0) a = WRTDWN;             // Get entry mode command
    else if (b == ROT90) a = WRTRGT;    // For SSD2119 register 11h
    else if (b == ROT180) a = WRTUP;    // To set writing direction
    else a = WRTLFT;                    // 270

    LCDCmd(0x11);                       // Set entry mode, to write
    LCDData(a);                         // Pixels, column by column

    while (*cp) {
        LCDChar(*cp++);                 // Output chars to screen
    }

    LCDCmd(0x11);
    LCDData(WRTRGT);                    // Set default entry mode
}


/***********************************************************
 * Write character bitmap to display. Font data is expected
 * to be in columns, a byte for every 8 rows, LSB is top row.
 * Font data and tables are stored in flash (program memory).
 * "pgm_read_byte()" and "pgm_read_word()" are macros in
 * <pgmspace.h> to fetch a byte from flash instead of SRAM.
 ***********************************************************/
void LCDChar(char chr)
{
    BYTE wid, a, b, c, d, rw;
    BYTE flag, size;
    const PBYTE * bitp;                     // Pointer to flash memory space
    BYTE bits, mask;

    switch (font) {                         // Get the font information
        case FONT_M:                        // Med font
            if (chr < firstchr_M) return;   // Char code out of range
            a = chr - firstchr_M;
            if (a >= nr_chrs_M) return;     // Char code out of range

            wid = pgm_read_byte(&widtbl_M[a]);              // Number of columns
            bitp = (PBYTE *)pgm_read_word(&chrtbl_M[a]);    // Bitmap data pointer
            size = chr_hgt_M;               // Cell height
            break;

        case FONT_L:                        // Large font
            if (chr < firstchr_L) return;   // Char code out of range
            a = chr - firstchr_L;
            if (a >= nr_chrs_L) return;     // Char code out of range

            wid = pgm_read_byte(&widtbl_L[a]);              // Number of columns
            bitp = (PBYTE *)pgm_read_word(&chrtbl_L[a]);    // Bitmap data pointer
            size = chr_hgt_L;               // Cell height
            break;

        case FONT_G:                        // Great font
            if (chr < firstchr_G) return;   // Char code out of range
            a = chr - firstchr_G;
            if (a >= nr_chrs_G) return;     // Char code out of range

            wid = pgm_read_byte(&widtbl_G[a]);              // Number of columns
            bitp = (PBYTE *)pgm_read_word(&chrtbl_G[a]);    // Bitmap data pointer
            size = chr_hgt_G;               // Cell height
            break;

        case FONT_H:                        // Huge font
            if (chr < firstchr_H) return;   // Char code out of range
            a = chr - firstchr_H;
            if (a >= nr_chrs_H) return;     // Char code out of range

            wid = pgm_read_byte(&widtbl_H[a]);              // Number of columns
            bitp = (PBYTE *)pgm_read_word(&chrtbl_H[a]);    // Bitmap data pointer
            size = chr_hgt_H;               // Cell height
            break;

        default:                            // Small font
            if (chr < firstchr_S) return;   // Char code out of range
            a = chr - firstchr_S;
            if (a >= nr_chrs_S) return;     // Char code out of range

            wid = pgm_read_byte(&widtbl_S[a]);              // Number of columns
            bitp = (PBYTE *)pgm_read_word(&chrtbl_S[a]);    // Bitmap data pointer
            size = chr_hgt_S;               // Cell height
            break;
    }

    if (wid == 0) return;

    if (chr != ' ') {                       // Don't process space char
        d = size / 8;                       // Number of bytes per column
        for (a = 0; a < wid; a++) {         // Do columns
            LCDSetTCur(dsrow, dscol + a);   // Set starting row, column
            flag = 0;
            for (b = 0; b < d; b++) {       // Do column bytes
                bits = pgm_read_byte(bitp + b + (d * a));   // Get column byte
                mask = 1;                   // Start with D0 (top pixel of col)
                rw = dsrow + b * 8;
                for (c = 0; c < 8; c++) {   // Do bits
                    if (mask & bits) {      // Put fg pixel
                        if (flag) {         // Cursor addr need to be set
                            LCDSetTCur(rw + c, dscol + a);  // Set starting row, column
                            flag = 0;
                        }
                        LCDData(fgcolor);
                    } else {
                        if (! (CJ_code & NOBG)) {   // Put bg pixel
                            if (flag) {     // Cursor addr need to be set
                                LCDSetTCur(rw + c, dscol + a);  // Set starting row, column
                                flag = 0;
                            }
                            LCDData(bgcolor);
                        } else flag = 1;      // Skip write, flag for addr update
                    }
                    mask <<= 1;             // Next bit
                }   // End do bits
            }   // End do column bytes
        }   // End do columns
    }   // End if not space

    if (! (CJ_code & NOSPC)) {              // Space at end of char
        if (! (CJ_code & NOBG)) {           // Put bg pixels
            LCDSetTCur(dsrow, dscol + wid); // Set starting row, column
            for (a = 0; a < size; a++) {
                LCDData(bgcolor);
            }
        }
        wid++;                              // Add space to char wid
    }
    dscol += wid;                           // Update current column
}


/***********************************************************
 * Erase a block of pixels, using bgcolor
 ***********************************************************/
void LCDErase(WORD row, WORD col, WORD hgt, WORD wid)
{
    WORD a, b, c;

    b = row + hgt;
    for (c = row; c <= b; c++) {            // Do rows
        LCDSetGCur(c, col);
        CLR(CS);
        SET(CD);
#ifdef BIT16
        OUT_HI = bgcolor >> 8;
        OUT_LO = bgcolor & 0xFF;
#endif
        for (a = 0; a < wid; a++) {         // Do columns
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
 * Read a block of pixels from screen into memory array
 ***********************************************************/
void LCDRdBlock(WORD *mem, WORD row, WORD col, WORD hgt, WORD wid)
{
    WORD a, c;

    for (c = row; c <= row + hgt; c++) {
        LCDSetGCur(c, col);
        LCDRead();                          // Do dummy read
        for (a = 0; a < wid; a++) {
            *mem++ = LCDRead();
        }
    }
}


/***********************************************************
 * Write a block of pixels from memory array to screen
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
 * Write cursor address to controller
 ***********************************************************/
void LCDSetGCur(WORD row, WORD col)
{
    if (col > LASTCOL || row > LASTROW) return;
    LCDCmd(0x4E);                           // RAM X address set
    LCDData(col);
    LCDCmd(0x4F);                           // RAM Y address set
    LCDData(row);
    LCDCmd(0x22);                           // Access to display RAM
}


/***********************************************************
 * Write cursor address to controller
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
    LCDCmd(0x22);                           // Access to display RAM
}


/***********************************************************
 * Get width of display text, in pixels
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
                wid++;                      // Include blank space
            }
        }
        chr++;
    }
    return wid;
}


/***********************************************************
 * Plot a pixel at X,Y
 * 0,0 is at upper left
 ***********************************************************/
void plot(WORD x, WORD y)
{
    LCDSetGCur(y, x);                       // Address the pixel
    LCDData(fgcolor);
}


/***********************************************************
 * Plot a pixel at X,Y and save point
 ***********************************************************/
void point(WORD x, WORD y)
{
    gr_x = x;
    gr_y = y;
    plot(x, y);
}


/***********************************************************
 * Draw a line from saved X,Y to X2,Y2 and save new endpoint
 * Bresenham's algorithm
 ***********************************************************/
void lineto(WORD x2, WORD y2)
{
    int i, delta_x, delta_y, numpixels;
    int d, dinc1, dinc2;
    int x, xinc1, xinc2;
    int y, yinc1, yinc2;

                                            // Calculate delta_x and delta_y for initialization
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
    } else {                                // y is independent variable
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
    gr_x = x2;                              // Save new endpoint for next line
    gr_y = y2;

    /* Draw the pixels */
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
