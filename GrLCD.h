#if !defined(GrLCD_H)
#define GrLCD_H

// display size

#define NR_COLS		320
#define LASTCOL		(NR_COLS - 1)
#define NR_ROWS		240
#define LASTROW		(NR_ROWS - 1)

// SSD2119 controller entry modes

#define WRTDWN	0x6878	// rows downward, cols rightward, 0°
#define	WRTRGT	0x6870	// rows rightward, cols upward, 90°
#define WRTUP	0x6848	// rows upward, cols leftward, 180°
#define WRTLFT	0x6860	// rows leftward, cols downward, 270°

// ports for LCD control & data bus

#define	RD	0x80	// control bits
#define	WR	0x40
#define	CD	0x20
#define	CS	0x10
#define	RES	8

#define MODE	PORTD	// output ports
#define CTRL	PORTC
#define OUT_LO	PORTE
#define OUT_HI	PORTA
#define IN_LO	PINE	// input ports
#define IN_HI	PINA
#define DIR_HI	DDRA	// direction registers
#define DIR_CT	DDRC
#define DIR_MD	DDRD
#define DIR_LO	DDRE
#define OUTS	0xFF	// port direction states
#define INS		0

#define SET(arg1)	CTRL |= arg1
#define CLR(arg1)	CTRL &= ~arg1

typedef struct
{
	BYTE font;		// font to use
	BYTE cj_code;	// control/justify
	BYTE row;		// start row
	BYTE col;		// start col IF ABSJ, else not used
	WORD fg;		// foreground color
	WORD bg;		// background color
	char *text;		// pointer to null terminated string
} String_T;

//	text string display control/justification codes

#define FONT_H		4	// huge font
#define FONT_G		3	// great font
#define FONT_L		2	// large font
#define FONT_M		1	// med font
#define FONT_S		0	// small font

#define ROTN		0xC0	// bit mask
#define ROT270		0xC0	// 270° CCW, top to bottom
#define ROT180		0x80	// 180° CCW, right to left
#define ROT90		0x40	// 90° CCW, bottom to top
#define NOBG		0x20	// no background (transparent)
#define NOSPC		0x10	// no space between characters
#define JUSTIFY		7		// bit mask
#define L00J		0		// left, starting @ col 0
#define L25J		1		// left, centered @ 1/4
#define L33J		2		// left, centered @ 1/3
#define C50J		3		// centered @ 1/2
#define R66J		4		// right, centered @ 2/3
#define R75J		5		// right, centered @ 3/4
#define R100J		6		// right, ending @ last col
#define ABSJ		7		// starting @ abs col #

extern BYTE CJ_code;		// control/justify code for text display
extern BYTE linenr;
extern BYTE font;
extern WORD fgcolor, bgcolor;	// colors
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

#endif
