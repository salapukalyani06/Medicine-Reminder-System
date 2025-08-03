#include <LPC21xx.H>
#include <stdio.h>  // Include stdio.h for sprintf
#include "delay.h"
#include "rtc.h"
#include "lcd.h"
#include "lcd_defines.h"
#include "keypad.h"

#define RTC_EDIT_SWITCH_PIN   24   // SW1 (RTC Edit) on P1.24
#define RTC_EDIT_PRESSED  (!(IOPIN1 & (1 << RTC_EDIT_SWITCH_PIN)))  // Active Low

#define BUZZER_PIN   7    // Buzzer on P0.7
#define BUZZER_ON    IOSET0 = (1 << BUZZER_PIN)  // Turn buzzer ON
#define BUZZER_OFF    IOCLR0 = (1 << BUZZER_PIN)  // Turn buzzer OFF

#define SWITCH_PIN   25   // Switch on P1.25 (Active Low)
#define SWITCH_PRESSED  (!(IOPIN1 & (1 << SWITCH_PIN)))  // Reads 0 when pressed

u32 MED_HOUR = 19;  // Initial Medicine Hour
u32 MED_MIN  = 15;  // Initial Medicine Minute
u32 MED_SEC  = 0;   // Initial Medicine Second
u32 hour, min, sec, day, date, month, year;
u32 password = 1234; // Initial Password

// Function declarations with argument types
void ShowMainMenu(void);
u32 ReadNumberFromKeypad(u32 digits, const char* prompt, u32 maxValue);
void EditRTCSubMenu(void);
void EditMedicineTimeSubMenu(void);
u32 VerifyPassword(void);
void EditPassword(void);

u32 ReadNumberFromKeypad(u32 digits, const char* prompt, u32 maxValue)
{
    u32 num = 0;
    u32 count = 0;
    u8 key;
    char buffer[10];  // Temporary buffer to hold the displayed string

    CmdLCD(CLEAR_LCD);
    SetCursor(1, 0);
    StrLCD((s8*)prompt);  // Display the prompt
    SetCursor(2, 0);
    StrLCD(":");  // Display initial colon for formatting

    while (count < digits)
    {
        key = KeyDetect();  // Wait for a key press

        if (key >= 0 && key <= 9)  // If a number key is pressed
        {
            num = (num * 10) + key;  // Append digit to number

            // Display the pressed digit
            SetCursor(2, 1 + count);  // Adjust cursor position
            CharLCD('0' + key);  // Convert integer to corresponding character

            count++;
        }
        else if (key == 66)  // Backspace key
        {
            if (count > 0)
            {
                num = num / 10;  // Remove last digit
                count--;

                // Clear last digit on display
                SetCursor(2, 1 + count);
                CharLCD(' ');
                SetCursor(2, 1 + count);
            }
        }
        else if (key == 65)  // Enter key
        {
            if (count == 1)  // If only one digit is entered, format it as two-digit
            {
                num = num;  // This ensures the format remains correct.
            }
            break;  // Exit loop and return the value
        }
    }

    // Check for validity and display an error if invalid
    if (num > maxValue)
    {
        CmdLCD(CLEAR_LCD);
        SetCursor(1, 0);
        StrLCD("Error: Invalid");
        delay_ms(2000);
        return ReadNumberFromKeypad(digits, prompt, maxValue);  // Retry input
    }

    return num;
}


void EditRTCSubMenu(void)
{
    u32 newValue;
    u32 key;
    u8 exit = 0;

    while(!exit)
    {
        CmdLCD(CLEAR_LCD);
        SetCursor(1, 0);
        StrLCD("1:HH 2:MM 3:SS");
        SetCursor(2, 0);
        StrLCD("4:DD 5:MM 6:YYYY     7:DAY 8:EXIT");

        key = KeyDetect();  // Wait for key input

        if(key == 1)  // HH
        {
            newValue = ReadNumberFromKeypad(2, "Set Hour", 23);
            RTCSetTime(newValue, min, sec);  // Update RTC Hour
            RTCGetTime(&hour, &min, &sec);  // Update global variables
            CmdLCD(CLEAR_LCD);
            StrLCD("RTC UPDATED");
            delay_ms(200);
        }
        else if(key == 2)  // MM
        {
            newValue = ReadNumberFromKeypad(2, "Set Min", 59);
            RTCSetTime(hour, newValue, sec);  // Update RTC Minute
            RTCGetTime(&hour, &min, &sec);  // Update global variables
            CmdLCD(CLEAR_LCD);
            StrLCD("RTC UPDATED");
            delay_ms(200);
        }
        else if(key == 3)  // SS
        {
            newValue = ReadNumberFromKeypad(2, "Set Sec", 59);
            RTCSetTime(hour, min, newValue);  // Update RTC Second
            RTCGetTime(&hour, &min, &sec);  // Update global variables
            CmdLCD(CLEAR_LCD);
            StrLCD("RTC UPDATED");
            delay_ms(200);
        }
        else if(key == 4)  // DD
        {
            newValue = ReadNumberFromKeypad(2, "Set Date", 31);
            RTCSetDate(newValue, month, year);  // Update RTC Date
            RTCGetDate(&date, &month, &year);  // Update global variables
            CmdLCD(CLEAR_LCD);
            StrLCD("RTC UPDATED");
            delay_ms(200);
        }
        else if(key == 5)  // MM (Month)
        {
            newValue = ReadNumberFromKeypad(2, "Set Month", 12);
            RTCSetDate(date, newValue, year);  // Update RTC Month
            RTCGetDate(&date, &month, &year);  // Update global variables
            CmdLCD(CLEAR_LCD);
            StrLCD("RTC UPDATED");
            delay_ms(200);
        }
        else if(key == 6)  // YYYY
        {
            newValue = ReadNumberFromKeypad(4, "Set Year", 9999);  // Assuming a max year of 9999
            RTCSetDate(date, month, newValue);  // Update RTC Year
            RTCGetDate(&date, &month, &year);  // Update global variables
            CmdLCD(CLEAR_LCD);
            StrLCD("RTC UPDATED");
            delay_ms(200);
        }
        else if(key == 7)  // DAY
        {
            newValue = ReadNumberFromKeypad(1, "Set Day (0-6)", 6);
            RTCSetDay(newValue);  // Update RTC Day
            RTCGetDay(&day);  // Update global variable
            CmdLCD(CLEAR_LCD);
            StrLCD("RTC UPDATED");
            delay_ms(200);
        }
        else if(key == 8)  // EXIT
        {
            CmdLCD(CLEAR_LCD);  // Clear the LCD
            exit = 1;  // Exit the loop
        }
    }
}


void EditMedicineTimeSubMenu(void)
{
    u32 newValue;
    u32 key;
    u8 exit = 0;

    while(!exit)
    {
        CmdLCD(CLEAR_LCD);
        SetCursor(1, 0);
        StrLCD("1:HH 2:MM 3:SS");
        SetCursor(2, 0);
        StrLCD("4:EDIT PASSWORD      5:EXIT");

        key = KeyDetect();  // Wait for key input

        if(key == 1)  // HH
        {
            newValue = ReadNumberFromKeypad(2, "Set Hour", 23);
            MED_HOUR = newValue;  // Update Medicine Hour
            CmdLCD(CLEAR_LCD);
            StrLCD("MEDICINE TIME UPDATED");
            delay_ms(200);
        }
        else if(key == 2)  // MM
        {
            newValue = ReadNumberFromKeypad(2, "Set Min", 59);
            MED_MIN = newValue;  // Update Medicine Minute
            CmdLCD(CLEAR_LCD);
            StrLCD("MEDICINE TIME UPDATED");
            delay_ms(200);
        }
        else if(key == 3)  // SS
        {
            newValue = ReadNumberFromKeypad(2, "Set Sec", 59);
            MED_SEC = newValue;  // Update Medicine Second
            CmdLCD(CLEAR_LCD);
            StrLCD("MEDICINE TIME UPDATED");
            delay_ms(200);
        }
        else if(key == 4)  // EDIT PASSWORD
        {
            EditPassword();  // Call EditPassword function
        }
        else if(key == 5)  // EXIT
        {
            CmdLCD(CLEAR_LCD);  // Clear the LCD
            exit = 1;  // Exit the loop
        }
    }
}


void EditPassword(void)
{
    u32 newPassword;

    newPassword = ReadNumberFromKeypad(4, "New Password", 9999);  // Assuming a max password value of 9999
    password = newPassword;  // Update the password
    CmdLCD(CLEAR_LCD);
    StrLCD("PASSWORD CHANGED");
    delay_ms(2000);
}


u32 VerifyPassword(void)
{
    u32 enteredPassword;

    enteredPassword = ReadNumberFromKeypad(4, "Enter Password", 9999);  // Assuming a max password value of 9999

    if (enteredPassword == password)
    {
        return 1;  // Password is correct
    }
    else
    {
        CmdLCD(CLEAR_LCD);
        SetCursor(1, 0);
        StrLCD("Error: Wrong Password");
        delay_ms(200);
			  CmdLCD(CLEAR_LCD);
        return 0;  // Password is incorrect
    }
}

void ShowMainMenu(void)
{
    CmdLCD(CLEAR_LCD);
    SetCursor(1, 0);
    StrLCD("1:RTC EDIT");
    SetCursor(2, 0);
    StrLCD("2:MEDICINE TIME EDIT 3:EXIT");
}

void DisplayMedicineTime(u32 med_hour, u32 med_min, u32 med_sec)
{
    char medTime[16];
    sprintf(medTime, "MED: %02u:%02u:%02u", med_hour, med_min, med_sec);
    SetCursor(3, 0);
    StrLCD(medTime);
}

int main(void)
{ 
    u8 key;
    int menuMode = 0;
    
    IODIR0 |= (1 << BUZZER_PIN); // Set P0.7 as output
    BUZZER_OFF; // Ensure buzzer is off initially

    // Configure RTC Edit Switch (SW1) as Input
    IODIR1 &= ~(1 << RTC_EDIT_SWITCH_PIN);  // Set P1.24 as input
    PINSEL2 &= ~(3 << 16);  // Ensure P1.24 is in GPIO mode

    Keypad_Init(); // Initialize keypad
    RTC_Init();
    InitLCD();
    CmdLCD(0x0C);
    RTCSetTime(19, 14, 55);
    RTCSetDate(31, 1, 2025);
    RTCSetDay(5);

    while(1)
    {
        RTCGetTime(&hour, &min, &sec);
        RTCGetDate(&date, &month, &year);
        RTCGetDay(&day);

        // Check if SW1 (RTC Edit) is pressed
        if (RTC_EDIT_PRESSED)
        {
            delay_ms(200);  // Debounce delay

            if (RTC_EDIT_PRESSED) // Confirm button is still pressed
            {
                menuMode = 1;  // Enable menu mode
                ShowMainMenu();  // Display main menu

                // Wait for key input
                key = KeyDetect();  // Move the declaration to the beginning

                if (key == 1)
                {
                    EditRTCSubMenu();
                }
                else if (key == 2)
                {
                    if (VerifyPassword())  // Check if the password is correct
                    {
                        EditMedicineTimeSubMenu();
                    }
                }
                else if (key == 3)
                {
                    CmdLCD(CLEAR_LCD);  // Clear the LCD
                    menuMode = 0;  // Exit menu mode
                }

                // Wait until SW1 is released to prevent re-triggering
                while (RTC_EDIT_PRESSED);
                delay_ms(200);

                menuMode = 0;  // Exit menu mode
            }
        }

        // If menu is active, do not update RTC display
        if (menuMode)
        {
            // Skip normal display updates
        }
        else
        {
					  //CmdLCD(CLEAR_LCD);  // Clear the LCD
            // Normal RTC Display Mode
            DisplayRTCTime(hour, min, sec);
            DisplayRTCDate(date, month, year);
            DisplayRTCDay(day);
					  StrLCD("          ");
					  DisplayMedicineTime(MED_HOUR, MED_MIN, MED_SEC);
					  
        }

        // Check if it's Medicine Time
        if (hour == MED_HOUR && min == MED_MIN && sec == MED_SEC)
        {
            CmdLCD(CLEAR_LCD);
            SetCursor(1, 0);
            StrLCD("MEDICINE TIME!");
            BUZZER_ON;

            // Wait until Medicine Acknowledge Switch is Pressed
            while (SWITCH_PRESSED);
            delay_ms(200);  // Debounce
            while (!SWITCH_PRESSED);
            delay_ms(200);  // Debounce

            BUZZER_OFF;
            CmdLCD(CLEAR_LCD);
        }
    }
}