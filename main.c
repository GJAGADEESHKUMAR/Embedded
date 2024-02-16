#include <stdint.h>
#include "inc/tm4c123gh6pm.h"

#define COLOR_GREEN_ON 0x08
#define COLOR_BLUE_ON 0x04
#define COLOR_RED_ON 0x02
#define COLOR_MAGENTA_ON 0x06
#define COLOR_YELLOW_ON 0x0A
#define COLOR_WHITE_ON 0x0E
#define COLOR_CYAN_ON 0x0C
#define MAXBLINK_DELAY 2000
#define T1 1000
#define T2 500
#define T3 250
#define T4 125
#define T5 62
#define T6 31
#define T7 16
#define MINBLINK_DELAY 8


// Macros for switch debouncing
#define DEBOUNCE_DELAY 10 //10m sec

#define delayMs(ms) SysTick_Delay(ms)


uint32_t cycleColors(uint32_t Color);
uint32_t getColorGreen(void);
uint32_t getColorBlue(void);
uint32_t getColorCyan(void);
uint32_t getColorRed(void);
uint32_t getColorYellow(void);
uint32_t getColorMagenta(void);
uint32_t getColorWhite(void);

// Array of function pointers for colors
uint32_t (*colorFunctions[])(void) =
{
    getColorGreen,
    getColorBlue,
    getColorCyan,
    getColorRed,
    getColorYellow,
    getColorMagenta,
    getColorWhite
};

uint32_t getColorGreen(void){return COLOR_GREEN_ON;}
uint32_t getColorBlue(void){return COLOR_BLUE_ON;}
uint32_t getColorCyan(void){return COLOR_CYAN_ON;}
uint32_t getColorRed(void){return COLOR_RED_ON;}
uint32_t getColorYellow(void){return COLOR_YELLOW_ON;}
uint32_t getColorMagenta(void){return COLOR_MAGENTA_ON;}
uint32_t getColorWhite(void){return COLOR_WHITE_ON;}

#define SYS_CLK 16000

void switchState(uint32_t n);

int SW1;
int SW2;

// set the initial blinkSpeeds for ON / OFF state of the LEDs
int blinkSpeed = MAXBLINK_DELAY;
// set the initial state color to Green
uint32_t Color=COLOR_GREEN_ON;

int main(void)
{
    SYSCTL_RCGC2_R |= 0x00000020; /* enable clock to GPIOF at clock gating control register */
    GPIO_PORTF_LOCK_R = 0x4C4F434B; /* unlock commit register GPIOCR */
    GPIO_PORTF_CR_R = 0x01; /* make PORTF0 configurable */
    GPIO_PORTF_DIR_R = 0x0E; /* enable the GPIO pins for the LED (PF3, 2 1) as output */
    GPIO_PORTF_DEN_R = 0x1F; /* enable the GPIO pins for digital function */
    GPIO_PORTF_PUR_R = 0x11; /* enable pull up for SW1 and SW2 */

    while (1)
    {

        if(blinkSpeed<MAXBLINK_DELAY && blinkSpeed>MINBLINK_DELAY)
        {
            GPIO_PORTF_DATA_R = Color; /*turn on LED*/
            switchState(blinkSpeed);
            GPIO_PORTF_DATA_R = 0; /* turn off LED */
            switchState(blinkSpeed);
        }
        else
        {
            GPIO_PORTF_DATA_R = Color; /*turn on LED*/
            switchState(blinkSpeed);
            GPIO_PORTF_DATA_R = 0; /* turn off LED */

        }

    }
}
static uint32_t currentColorIndex = 1;
uint32_t cycleColors(uint32_t  Color)
{
    uint32_t color = colorFunctions[currentColorIndex]();
    currentColorIndex = (currentColorIndex + 1) % 7; // Cycle through colors
    return color;
}

int adjustBlinkSpeed(int blinkSpeed)
{
    if (blinkSpeed == MAXBLINK_DELAY)
        return T1;
    else if (blinkSpeed == T1)
        return T2;
    else if (blinkSpeed == T2)
        return T3;
    else if (blinkSpeed == T3)
        return T4;
    else if (blinkSpeed == T4)
        return T5;
    else if (blinkSpeed == T5)
        return T6;
    else if (blinkSpeed == T6)
        return T7;
    else if (blinkSpeed == T7)
        return MINBLINK_DELAY;
    else if (blinkSpeed == MINBLINK_DELAY)
        return T1;
    else
        return T1;
}

// Function to initialize SysTick blinkSpeedr for delay
        void SysTick_Delay(uint32_t delaySeconds)
        {
            uint32_t reloadValue = (delaySeconds * SYS_CLK);
            uint32_t count = 0;
            while (reloadValue > (1 << 24) - 1)
            {
                // If reloadValue exceeds the 24-bit limit, reduce it
                reloadValue -= (1 << 24);
                count++;
            }
            while (count--)
            {

                NVIC_ST_CTRL_R = 0;              // Disable SysTick during setup
                NVIC_ST_RELOAD_R = (1 << 24) - 1; // Set reload register for the desired delay
                NVIC_ST_CURRENT_R = 0;              // Clear current register
                NVIC_ST_CTRL_R |= 0x05; // Enable SysTick with core clock (0101)

                while ((NVIC_ST_CTRL_R & 0x00010000) == 0)
                    ;
            }
            NVIC_ST_CTRL_R = 0;                 // Disable SysTick during setup
            NVIC_ST_RELOAD_R = reloadValue - 1; // Set reload register for the desired delay
            NVIC_ST_CURRENT_R = 0;              // Clear current register
            NVIC_ST_CTRL_R |= 0x05;     // Enable SysTick with core clock (0101)

            while ((NVIC_ST_CTRL_R & 0x00010000) == 0)
                ;
        }

void switchState(uint32_t n)
{
    // read switchState every 10 ms in the blink Interval
    for (int i = 0; i <= n; i = i + DEBOUNCE_DELAY)
    {
        SW2 = GPIO_PORTF_DATA_R;
        // if sw2 is pressed, stay in loop until it is pressed again
        if (!(SW2 & 0x01))
        {
            delayMs(DEBOUNCE_DELAY);
            if (!(SW2 & 0x01))
            {
                //This takes care of debouncing
                while (!(SW2 & 0x01))
                {
                    SW2 = GPIO_PORTF_DATA_R;

                }

                blinkSpeed = adjustBlinkSpeed(blinkSpeed);
                break;
            }
        }
        delayMs(10);

        SW1 = GPIO_PORTF_DATA_R;
        // if sw1 is pressed, stay in loop until it is pressed again
        if (!(SW1 & 0x10))
        {
            //This takes care of debouncing
            while (!(SW1 & 0x10))
            {
                SW1 = GPIO_PORTF_DATA_R;

            }

            Color = cycleColors(Color);
            break;
        }
    }
}
