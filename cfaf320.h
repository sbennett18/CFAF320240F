#ifndef CFAF320_H_
#define CFAF320_H_

//#define BIT16

#define M68_16  0
#define M68_8   1
#define M80_16  2
#define M80_8   3
#define MSPI_3  0xE
#define MSPI_4  0xF

/* Color Definitions */
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Maximum values for R, G, B are 31, 63, 31 (5b, 6b, 5b)
// 16 bit color RRRR RGGG GGGB BBBB
#define RGB(r,g,b)  (WORD)(((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F))

void delay(WORD);

#endif /* CFAF320_H_ */
