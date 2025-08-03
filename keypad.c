#include <LPC21xx.H>
#include "types.h"
#include"keypad.h"
#include "lcd_defines.h"
#include "defines.h"
#include "lcd.h"

/* Keypad pin connections */
// rows are connected to P1.16 to P1.19
#define ROW0 16
#define ROW1 17
#define ROW2 18
#define ROW3 19

// cols are connected to P1.20 to P1.23
#define COL0 20
#define COL1 21
#define COL2 22
#define COL3 23

// keypad lookup table
unsigned int keypad_lut[4][4] = {
    {1, 2, 3, 65},
    {4, 5, 6, 66},
    {7, 8, 9, 67},
    {12, 0, 14, 15}
};

void Keypad_Init(void)
{
    // configuring the rows port pins as output port pins    
    IODIR1 |= ((1 << ROW0) | (1 << ROW1) | (1 << ROW2) | (1 << ROW3));
    // initially making all rows to zero
    //IOCLR1 = ((1 << ROW0) | (1 << ROW1) | (1 << ROW2) | (1 << ROW3));
    // initially making all columns to one
    //IOSET1 |= ((1 << COL0) | (1 << COL1) | (1 << COL2) | (1 << COL3));
}

u32 ColScan(void)
{
    u32 c0, c1, c2, c3;

    c0 = ((IOPIN1 >> COL0) & 1);
    c1 = ((IOPIN1 >> COL1) & 1);
    c2 = ((IOPIN1 >> COL2) & 1);
    c3 = ((IOPIN1 >> COL3) & 1);

    if((c0 & c1 & c2 & c3) == 0)
        return 0;
    else
        return 1;    
}

u32 RowCheck(void)
{
    u32 rNo;

    // row0 identification
    while(1)
    {                    
        // row0 making low
        IOCLR1 = (1 << ROW0);
        // remaining rows making high
        IOSET1 = ((1 << ROW1) | (1 << ROW2) | (1 << ROW3));
        // if any one of the column value is 0 then row value is 0 then no need to check other rows
        if(ColScan() == 0)
        {
            rNo = 0;
            break;
        }

        // row1 identification
        // row1 making low
        IOCLR1 = (1 << ROW1);
        // remaining rows making high
        IOSET1 = ((1 << ROW0) | (1 << ROW2) | (1 << ROW3));
        // if any column value is 0 then row value is 1 then no need to check other rows
        if(ColScan() == 0)
        {
            rNo = 1;
            break;
        }

        // row2 identification
        // row2 making low
        IOCLR1 = (1 << ROW2);
        // remaining rows making high
        IOSET1 = ((1 << ROW0) | (1 << ROW1) | (1 << ROW3));
        // if any column value is 0 then row value is 2 then no need to check other rows
        if(ColScan() == 0)
        {
            rNo = 2;
            break;
        }

        // row3 identification
        // row3 making low
        IOCLR1 = (1 << ROW3);
        // remaining rows making high
        IOSET1 = ((1 << ROW0) | (1 << ROW1) | (1 << ROW2));
        // if any column value is 0 then row value is 3
        if(ColScan() == 0)
        {
            rNo = 3;
            break;
        }
    }
    return rNo;
}

u32 ColCheck(void)
{
    u32 cNo;
    // columns identification process
    if(((IOPIN1 >> COL0) & 1) == 0)
        cNo = 0;
    else if(((IOPIN1 >> COL1) & 1) == 0)
        cNo = 1;
    else if(((IOPIN1 >> COL2) & 1) == 0)
        cNo = 2;
    else if(((IOPIN1 >> COL3) & 1) == 0)
        cNo = 3;

    return cNo;
}

u32 KeyDetect(void)
{
    u32 rowval, colval;

    // Drive all rows LOW before scanning
    IOCLR1 = ((1 << ROW0) | (1 << ROW1) | (1 << ROW2) | (1 << ROW3));

    // Wait for a key press
    while (ColScan());  // This should return 0 when a key is pressed

    delay_ms(50);  // Debounce delay

    // Identify which row is active
    rowval = RowCheck();

    // Identify which column is active
    colval = ColCheck();

    // Wait for key release
    while (!ColScan());

    delay_ms(50);  // Debounce delay

    // Return the corresponding key from the lookup table
    return keypad_lut[rowval][colval];    
}

u32 ReadNum(void)
{
	u32 KeyC,num=0;
	CmdLCD(CLEAR_LCD);
	while(1)
	{
		KeyC = KeyDetect();
		if((KeyC>='0') && (KeyC<='9'))
		{
			CharLCD(KeyC);
			num = (num*10) + (KeyC-48);
		}
		else if(KeyC == 'C')//for backspace
		{
			if(num>0)
			{
				num = num/10;
				CmdLCD(SHIFT_CUR_LEFT);
				CharLCD(' ');
				CmdLCD(SHIFT_CUR_LEFT);
			}
		}
		else if(KeyC == '=')//for enter
			break;
	}
	return num;	
}