#ifndef OLED_TYPES_H__
#define OLED_TYPES_H__

#include <stdint.h>

#define _OLEDC(cmd) ((uint8_t) ((cmd) & 0xFF))

/*
Set Lower Column Start Address for Page Addressing Mode

Set the lower nibble of the column start
address register for Page Addressing Mode
using X[3:0] as data bits. The initial display
line register is reset to 0000b after RESET
*/
#define OLEDC_SET_LC_START_ADDR(addr) \
    _OLEDC((addr) & 0x0F)


/*
Set Higher Column
Start Address for
Page Addressing
Mode

Set the higher nibble of the column start
address register for Page Addressing Mode
using X[3:0] as data bits. The initial display
line register is reset to 0000b after RESET.
*/
#define OLEDC_SET_HC_START_ADDR(addr) \
    _OLEDC(0x10 | ((addr) & 0x0F))

/*
Set Memory
Addressing Mode

A[1:0] = 00b, Horizontal Addressing Mode
A[1:0] = 01b, Vertical Addressing Mode
A[1:0] = 10b, Page Addressing Mode
(RESET)
A[1:0] = 11b, Invalid
*/
#define OLEDC_SET_MEM_ADDR_MODE_0 \
    _OLEDC(0x20)

#define OLEDC_SET_MEM_ADDR_MODE_1(mode) \
    _OLEDC((mode) & 0x03)


/*
Set Column Address

Setup column start and end address
A[6:0] : Column start address, range : 0-127d,
(RESET=0d)
B[6:0]: Column end address, range : 0-127d,
(RESET =127d)
Note
(1) This command is only for horizontal or vertical
addressing mode
*/
#define OLEDC_SET_COL_ADDR_0 \
    _OLEDC(0x21)

#define OLEDC_SET_COL_ADDR_1(addr) \
    _OLEDC(((addr) & 0x7F)

#define OLEDC_SET_COL_ADDR_2(addr) \
    _OLEDC(((addr) & 0x7F)

/*
Set Page Address

Setup page start and end address
A[2:0] : Page start Address, range : 0-7d,
(RESET = 0d)
B[2:0] : Page end Address, range : 0-7d,
(RESET = 7d)
Note
(1) This command is only for horizontal or vertical
addressing mode.
*/
#define OLEDC_SET_PAGE_ADDR_0 \
    _OLEDC(0x22)

#define OLEDC_SET_PAGE_ADDR_1(addr) \
    _OLEDC((addr) & 0x07)

#define OLEDC_SET_PAGE_ADDR_2(addr) \
    _OLEDC((addr) & 0x07)

/*
Set Display Start
Line

Set display RAM display start line register
from 0-63 using X5X3X2X1X0.
Display start line register is reset to 000000b
during RESET.
*/
#define OLEDC_SET_DISPLAY_START_LINE(line_reg) \
    _OLEDC(0x40 | ((addr) & 0x3F))

/*
Set Contrast Control

Double byte command to select 1 out of 256
contrast steps. Contrast increases as the value
increases.
(RESET = 7Fh )
*/
#define OLEDC_SET_CONTRAST_CONTROL_0 \
    _OLEDC(0x81)

#define OLEDC_SET_CONTRAST_CONTROL_1(step) \
    _OLEDC(step)

/*
Set Segment Re-map

A0h, X[0]=0b: column address 0 is mapped to
SEG0 (RESET)
A1h, X[0]=1b: column address 127 is mapped
to SEG0
*/
#define OLEDC_SET_SEGMENT_REMAP(seg) \
    _OLEDC(0xA0 | ((seg) & 0x01))

/*
Entire Display ON

A4h, X0=0b: Resume to RAM content display
(RESET)
 Output follows RAM content
A5h, X0=1b: Entire display ON
 Output ignores RAM content
*/
#define OLEDC_ENTIRE_DISPLAY_ON(follow_ram) \
    _OLEDC(0xA4 | ((follow_ram) & 0x01))


/*
Set Normal/Inverse
Display

A6h, X[0]=0b: Normal display (RESET)
 0 in RAM: OFF in display panel
 1 in RAM: ON in display panel
A7h, X[0]=1b: Inverse display
 0 in RAM: ON in display panel
 1 in RAM: OFF in display panel
*/
#define OLEDC_SET_NORMAL_INVERSE_DISPLAY(normal_inverse) \
    _OLEDC(0xA6 | ((normal_inverse) & 0x01))

/*
Set Multiplex Ratio

Set MUX ratio to N+1 MUX
N=A[5:0] : from 16MUX to 64MUX, RESET=
111111b (i.e. 63d, 64MUX)
A[5:0] from 0 to 14 are invalid entry.
*/
#define OLEDC_SET_MUX_RATIO_0 \
    _OLEDC(0xA8)

#define OLEDC_SET_MUX_RATIO_1(ratio) \
    _OLEDC((ratio) & 0x3F)

/*
External or internal
IREF Selection

Select external or internal IREF :
A[4] = ‘0’ Select external IREF (RESET)
A[4] = ‘1’ Enable internal IREF during display
ON
Note
(1) Refer to section 7.8 for details.
*/
#define OLEDC_IREF_SELECTION_0 \
    _OLEDC(0xAD)

#define OLEDC_IREF_SELECTION_1(selection) \
    _OLEDC(((selection) << 4) & 0x10)

/*
Set Display ON/OFF

AEh, X[0]=0b:Display OFF (sleep mode) (RESET)
AFh X[0]=1b:Display ON in normal mode
*/
#define OLEDC_SET_DISPLAY_ON_OFF(on_off) \
    _OLEDC(0xAE | ((on_off) & 0x01))

/*
Set Page Start
Address for Page
Addressing Mode

Set GDDRAM Page Start Address
(PAGE0~PAGE7) for Page Addressing Mode
using X[2:0].
Note
(1) This command is only for page addressing mode
*/
#define OLEDC_SET_PAGE_START_ADDR(addr) \
    _OLEDC(0xB0 | ((addr) & 0x07))

/*
Set COM Output
Scan Direction

C0h, X[3]=0b: normal mode (RESET) Scan
from COM0 to COM[N –1]
C8h, X[3]=1b: remapped mode. Scan from
COM[N-1] to COM0
Where N is the Multiplex ratio.
*/
#define OLEDC_SET_COM_OUT_SCAN_DIR(dir) \
    _OLEDC(0xC0 | (((dir) << 3) & 0x08))


/*
Set Display Offset

Set vertical shift by COM from 0d~63d
The value is reset to 00h after RESET.
*/
#define OLEDC_SET_DISPLAY_OFFSET_0 \
    _OLEDC(0xD3)

#define OLEDC_SET_DISPLAY_OFFSET_1(offset) \
    _OLEDC((addr) & 0x3F)


/*
Set Display Clock
Divide
Ratio/Oscillator
Frequency

A[3:0] : Define the divide ratio (D) of the
display clocks (DCLK):
Divide ratio= A[3:0] + 1, RESET is
0000b (divide ratio = 1)
A[7:4] : Set the Oscillator Frequency, FOSC.
Oscillator Frequency increases with
the value of A[7:4] and vice versa.
RESET is 1000b
Range:0000b~1111b
Frequency increases as setting value
increases.
*/
#define OLEDC_SET_DISPLAY_CLOCK_DIV_0 \
    _OLEDC(0xD5)

#define OLEDC_SET_DISPLAY_CLOCK_DIV_1(ratio) \
    _OLEDC((ratio))


/*
Set Pre-charge
Period

A[3:0] : Phase 1 period of up to 15 DCLK
clocks 0 is invalid entry
(RESET=2h)
A[7:4] : Phase 2 period of up to 15 DCLK
clocks 0 is invalid entry
(RESET=2h )
*/
#define OLEDC_SET_PRE_CHARGE_PERIOD_0 \
    _OLEDC(0xD9)

#define OLEDC_SET_PRE_CHARGE_PERIOD_1(period) \
    _OLEDC((period))


/*
Set COM Pins
Hardware
Configuration

A[4]=0b, Sequential COM pin configuration
A[4]=1b(RESET), Alternative COM pin
configuration
A[5]=0b(RESET), Disable COM Left/Right
remap
A[5]=1b, Enable COM Left/Right remap
*/
#define OLEDC_SET_COM_PIN_HW_CFG_0 \
    _OLEDC(0xDA)

#define OLEDC_SET_COM_PIN_HW_CFG_1(cfg) \
    _OLEDC(0x02 | (((cfg) << 4) & 0x30))


/*
Set VCOMH Deselect
Level

A[6:4] Hex
code
V COMH deselect level
000b 00h ~ 0.65 x VCC
010b 20h ~ 0.77 x VCC (RESET)
011b 30h ~ 0.83 x VCC
*/
#define OLEDC_SET_VCOMH_DESELECT_LVL_0 \
    _OLEDC(0xDB)

#define OLEDC_SET_VCOMH_DESELECT_LVL_1(lvl) \
    _OLEDC(((lvl) << 4) & 0x70)

/*
NOP

Command for no operation
*/
#define OLEDC_NOP \
    _OLEDC(0xE3)


/*
Continuous
Horizontal
Scroll Setup

26h, X[0]=0, Right Horizontal Scroll
27h, X[0]=1, Left Horizontal Scroll
(Horizontal scroll by 1 column)
A[7:0] : Dummy byte (Set as 00h)
B[2:0] : Define start page address
000b – PAGE0 011b – PAGE3 110b – PAGE6
001b – PAGE1 100b – PAGE4 111b – PAGE7
010b – PAGE2 101b – PAGE5
C[2:0] : Set time interval between each scroll step in
terms of frame frequency
000b – 5 frames 100b – 3 frames
001b – 64 frames 101b – 4 frames
010b – 128 frames 110b – 25 frame
011b – 256 frames 111b – 2 frame
D[2:0] : Define end page address
000b – PAGE0 011b – PAGE3 110b – PAGE6
001b – PAGE1 100b – PAGE4 111b – PAGE7
010b – PAGE2 101b – PAGE5
 The value of D[2:0] must be larger or equal to
B[2:0]
E[7:0] : Dummy byte (Set as 00h)
F[7:0] : Dummy byte (Set as FFh)
*/
#define OLEDC_CONT_H_SCROLL_SETUP_0(scroll) \
    _OLEDC(0x26 | ((scroll) & 0x01))

#define OLEDC_CONT_H_SCROLL_SETUP_1 \
    _OLEDC(0x00)

#define OLEDC_CONT_H_SCROLL_SETUP_2(addr) \
    _OLEDC((addr) & 0x07)

#define OLEDC_CONT_H_SCROLL_SETUP_3(interval) \
    _OLEDC((interval) & 0x07)

#define OLEDC_CONT_H_SCROLL_SETUP_4(addr) \
    _OLEDC((addr) & 0x07)

#define OLEDC_CONT_H_SCROLL_SETUP_5 \
    _OLEDC(0x00)

#define OLEDC_CONT_H_SCROLL_SETUP_6 \
    _OLEDC(0xFF)

/*
Continuous
Vertical and
Horizontal
Scroll Setup

29h, X1X0=01b : Vertical and Right Horizontal Scroll
2Ah, X1X0=10b : Vertical and Left Horizontal Scroll
 (Horizontal scroll by 1 column)
A[7:0] : Dummy byte
B[2:0] : Define start page address
000b – PAGE0 011b – PAGE3 110b – PAGE6
001b – PAGE1 100b – PAGE4 111b – PAGE7
010b – PAGE2 101b – PAGE5
C[2:0] : Set time interval between each scroll step in
terms of frame frequency
000b – 5 frames 100b – 3 frames
001b – 64 frames 101b – 4 frames
010b – 128 frames 110b – 25 frame
011b – 256 frames 111b – 2 frame
D[2:0] : Define end page address
000b – PAGE0 011b – PAGE3 110b – PAGE6
001b – PAGE1 100b – PAGE4 111b – PAGE7
010b – PAGE2 101b – PAGE5
 The value of D[2:0] must be larger or equal
 to B[2:0]
E[5:0] : Vertical scrolling offset
 e.g. E[5:0]= 01h refer to offset =1 row
 E[5:0] =3Fh refer to offset =63 rows
Note
(1) No continuous vertical scrolling is available.
*/
#define OLEDC_CONT_HV_SCROLL_SETUP_0(scroll) \
    _OLEDC(0x29 | ((scroll) & 0x02))

#define OLEDC_CONT_HV_SCROLL_SETUP_1 \
    _OLEDC(0x00)

#define OLEDC_CONT_HV_SCROLL_SETUP_2(addr) \
    _OLEDC((addr) & 0x07)

#define OLEDC_CONT_HV_SCROLL_SETUP_3(interval) \
    _OLEDC((interval) & 0x07)

#define OLEDC_CONT_HV_SCROLL_SETUP_4(addr) \
    _OLEDC((addr) & 0x07)

#define OLEDC_CONT_HV_SCROLL_SETUP_5(offset) \
    _OLEDC((offset) & 0x3F)

/*
Deactivate
scroll

Stop scrolling that is configured by command
26h/27h/29h/2Ah.
Note
(1) After sending 2Eh command to deactivate the scrolling
action, the ram data needs to be rewritten.
*/
#define OLEDC_DEACTIVATE_SCROLL \
    _OLEDC(0x2E)

/*
Activate
scroll

Start scrolling that is configured by the scrolling setup
commands :26h/27h/29h/2Ah with the following valid
sequences:
Valid command sequence 1: 26h ;2Fh.
Valid command sequence 2: 27h ;2Fh.
Valid command sequence 3: 29h ;2Fh.
Valid command sequence 4: 2Ah ;2Fh.
For example, if “26h; 2Ah; 2Fh.” commands are
issued, the setting in the last scrolling setup command,
i.e. 2Ah in this case, will be executed. In other words,
setting in the last scrolling setup command overwrites
the setting in the previous scrolling setup commands
*/
#define OLEDC_ACTIVATE_SCROLL \
    _OLEDC(0x2F)


/*
Set Vertical
Scroll Area

A[5:0] : Set No. of rows in top fixed area. The No. of
rows in top fixed area is referenced to the top
of the GDDRAM (i.e. row 0).[RESET = 0]
B[6:0] : Set No. of rows in scroll area. This is the
number of rows to be used for vertical
scrolling. The scroll area starts in the first
row below the top fixed area. [RESET = 64]
Note
(1) A[5:0]+B[6:0] <= MUX ratio
(2) B[6:0] <= MUX ratio
(3a) Vertical scrolling offset (E[5:0] in 29h/2Ah) <
B[6:0]
(3b) Set Display Start Line (X5X4X3X2X1X0 of
40h~7Fh) < B[6:0]
(4) The last row of the scroll area shifts to the first row
of the scroll area.
(5) For 64d MUX display
 A[5:0] = 0, B[6:0]=64 : whole area scrolls
 A[5:0]= 0, B[6:0] < 64 : top area scrolls
 A[5:0] + B[6:0] < 64 : central area scrolls
 A[5:0] + B[6:0] = 64 : bottom area scrolls
*/
#define OLEDC_SET_V_SCROLL_AREA_0 \
    _OLEDC(0xA3)

#define OLEDC_SET_V_SCROLL_AREA_1(area) \
    _OLEDC((area) & 0x3F)

#define OLEDC_SET_V_SCROLL_AREA_2(rows) \
    _OLEDC((rows) & 0x7F)

#endif /* OLED_TYPES_H__ */