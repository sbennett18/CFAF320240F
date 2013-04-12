#ifndef GRLCD_H_
#define GRLCD_H_

/* Display size */

#define NR_COLS 320
#define LASTCOL (NR_COLS - 1)
#define NR_ROWS 240
#define LASTROW (NR_ROWS - 1)

/* SSD2119 controller entry modes */

#define WRTDWN  0x6878          // Rows downward, cols rightward, 0°
#define WRTRGT  x6870           // Rows rightward, cols upward, 90°
#define WRTUP   x6848           // Rows upward, cols leftward, 180°
#define WRTLFT  x6860           // Rows leftward, cols downward, 270°

typedef struct
{
    BYTE font;                  // Font to use
    BYTE cj_code;               // Control/justify
    BYTE row;                   // Start row
    BYTE col;                   // Start col IF ABSJ, else not used
    WORD fg;                    // Foreground color
    WORD bg;                    // Background color
    char *text;                 // Pointer to null terminated string
} String_T;

/* Text string display control/justification codes */

#define FONT_Hi     4           // Huge font
#define FONT_Gi     3           // Great font
#define FONT_Li     2           // Large font
#define FONT_Mi     1           // Med font
#define FONT_Si     0           // Small font

#define ROTN        0xC0        // Bit mask
#define ROT270      0xC0        // 270° CCW, top to bottom
#define ROT180      0x80        // 180° CCW, right to left
#define ROT90       0x40        // 90° CCW, bottom to top
#define NOBG        0x20        // No background (transparent)
#define NOSPC       0x10        // No space between characters
#define JUSTIFY     7           // Bit mask
#define L00J        0           // Left, starting @ col 0
#define L25J        1           // Left, centered @ 1/4
#define L33J        2           // Left, centered @ 1/3
#define C50J        3           // Centered @ 1/2
#define R66J        4           // Right, centered @ 2/3
#define R75J        5           // Right, centered @ 3/4
#define R100J       6           // Right, ending @ last col
#define ABSJ        7           // Starting @ abs col #

extern BYTE CJ_code;            // Control/justify code for text display
extern BYTE linenr;
extern BYTE font;
extern WORD fgcolor, bgcolor;   // Colors
extern WORD gr_x, gr_y;

void LCDInit(void);
void LCDChar(char);
void LCDCmd(WORD arg);
void LCDData(WORD arg);
void LCDClear(WORD row, WORD hgt);
void LCDText(String_T *text);
void LCDErase(WORD row, WORD col, WORD hgt, WORD wid);
void LCDRdBlock(WORD *mem, WORD row, WORD col, WORD hgt, WORD wid);
void LCDWrBlock(WORD *mem, WORD row, WORD col, WORD hgt, WORD wid);
void LCDSetTCur(WORD row, WORD col);
void LCDSetGCur(WORD row, WORD col);
WORD LCDPixLen(char *chr);
WORD LCDRead(void);
void point(WORD x, WORD y);
void lineto(WORD x2, WORD y2);

#endif /* GRLCD_H_ */
