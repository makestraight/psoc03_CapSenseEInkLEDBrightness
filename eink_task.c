/******************************************************************************
*
* File Name: eink_task.c
*
* Description: This file contains task and functions related to the of E-Ink
* that demonstrates controlling a EInk display using the EmWin Graphics Library.
* The project displays a start up screen with
* text "CYPRESS EMWIN GRAPHICS DEMO EINK DISPLAY".
*
* The project then displays the menu page screens with "LED On" as the default option
* 3 options are provided
*
*   1. LED On
*   2. LED Off
*   3. LED Brightness
*
* If brightness is selected, then "brightness page" will be shown
*
*******************************************************************************
* The code is modified based on the example code provided by Infineon
* Example Code Name: emWin E-Ink FreeRTOS
* Modifier: Yen-Chen Chang @ makestraight
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "GUI.h"
#include "cy8ckit_028_epd_pins.h"
#include "mtb_e2271cs021.h"
#include "LCDConf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "capsense_task.h"
#include "led_task.h"
#include "eink_task.h"

/*******************************************************************************
* Global variables
*******************************************************************************/
/* Queue handle used for eink data */
QueueHandle_t eink_command_q;
/* HAL SPI object to interface with display driver */
cyhal_spi_t spi;

/* Configuration structure defining the necessary pins to communicate with
 * the E-ink display */
const mtb_e2271cs021_pins_t pins =
{
    .spi_mosi = CY8CKIT_028_EPD_PIN_DISPLAY_SPI_MOSI,
    .spi_miso = CY8CKIT_028_EPD_PIN_DISPLAY_SPI_MISO,
    .spi_sclk = CY8CKIT_028_EPD_PIN_DISPLAY_SPI_SCLK,
    .spi_cs = CY8CKIT_028_EPD_PIN_DISPLAY_CS,
    .reset = CY8CKIT_028_EPD_PIN_DISPLAY_RST,
    .busy = CY8CKIT_028_EPD_PIN_DISPLAY_BUSY,
    .discharge = CY8CKIT_028_EPD_PIN_DISPLAY_DISCHARGE,
    .enable = CY8CKIT_028_EPD_PIN_DISPLAY_EN,
    .border = CY8CKIT_028_EPD_PIN_DISPLAY_BORDER,
    .io_enable = CY8CKIT_028_EPD_PIN_DISPLAY_IOEN,
};


/*******************************************************************
*
* The points of the triangle
*/
static const GUI_POINT aPointTriangle[] = {
 { 10,  0},
 {  0,  5},
 {  0, -5}
};

/* Buffer to the previous frame written on the display */
uint8_t previous_frame[PV_EINK_IMAGE_SIZE] = {0};

/* Pointer to the new frame that need to be written */
uint8_t *current_frame;

/*******************************************************************************
* Macros
*******************************************************************************/
#define DELAY_AFTER_STARTUP_SCREEN_MS       (2000)
#define AMBIENT_TEMPERATURE_C               (20)
#define SPI_BAUD_RATE_HZ                    (20000000)

/*******************************************************************************
* Forward declaration
*******************************************************************************/
void show_startup_screen(void);
void show_instructions_screen(void);
void clear_screen(void);

void show_led_on(void);
void show_led_off(void);
void show_led_brightness(void);

/*******************************************************************************
* Function Name: void show_startup_screen(void)
********************************************************************************
*
* Summary: This function displays the startup screen with
*           the demo description text
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_startup_screen(void)
{
    /* Set foreground and background color and font size */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();

    //GUI_DrawBitmap(&bmCypressLogoFullColor_PNG_1bpp, 2, 2);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("CYPRESS", 132, 85);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("EMWIN GRAPHICS", 132, 105);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("EINK DISPLAY DEMO", 132, 125);
}


/*******************************************************************************
* Function Name: void show_instructions_screen(void)
********************************************************************************
*
* Summary: This function shows screen with instructions to use CapSense slider to
*           control LED brightness
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_instructions_screen(void)
{
    /* Set font size, background color and text mode */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    /* Display instructions text */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("Move your finger on the slider", 132, 58);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("to adjust ", 132, 78);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("LED brightness!", 132, 98);
}

/*******************************************************************************
* Function Name: void show_led_on(void)
********************************************************************************
*
* Summary: This function shows screen with LED on option
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_led_on(void)
{
    /* Set font size, background color and text mode */
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("LED", 132, 5);

    GUI_SetFont(GUI_FONT_16B_1);
    GUI_DispStringAt("ON", 110, 58);
    GUI_DispStringAt("OFF", 110, 78);
    GUI_DispStringAt("BRIGHTNESS", 110, 98);

    // draw arrows
    GUI_FillPolygon(&aPointTriangle[0], 3, 90, 65);

}
/*******************************************************************************
* Function Name: void show_led_off(void)
********************************************************************************
*
* Summary: This function shows screen with LED off option
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_led_off(void)
{
    /* Set font size, background color and text mode */
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("LED", 132, 5);

    GUI_SetFont(GUI_FONT_16B_1);
    GUI_DispStringAt("ON", 110, 58);
    GUI_DispStringAt("OFF", 110, 78);
    GUI_DispStringAt("BRIGHTNESS", 110, 98);

    // draw arrows
    GUI_FillPolygon(&aPointTriangle[0], 3, 90,85);
}
/*******************************************************************************
* Function Name: void show_led_blink(void)
********************************************************************************
*
* Summary: This function shows screen with LED brightness option
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_led_brightness(void)
{
    /* Set font size, background color and text mode */
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("LED", 132, 5);

    GUI_SetFont(GUI_FONT_16B_1);
    GUI_DispStringAt("ON", 110, 58);
    GUI_DispStringAt("OFF", 110, 78);
    GUI_DispStringAt("BRIGHTNESS", 110, 98);

    // draw arrows
    GUI_FillPolygon(&aPointTriangle[0], 3, 90, 105);
}


/*******************************************************************************
* Function Name: void clear_screen(void)
********************************************************************************
*
* Summary: This function clears the screen
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void clear_screen(void)
{
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();
}


/*******************************************************************************
* Function Name: void eInk_task(void *arg)
********************************************************************************
*
* Summary: Following functions are performed
*           1. Initialize the EmWin library
*           2. Display the startup screen for 2 seconds
*           3. Display the menu page screen and wait for selection or option switching
*           4. If brightness option is selected, then "brightness page" will be shown
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void eInk_task(void *arg)
{
    cy_rslt_t result;
    uint8_t page_number = 0;
    BaseType_t rtos_api_result;
    eink_command_t eink_cmd_data;
    led_command_data_t led_cmd_data;

    /* Array of menu pages functions */
    void (*ledPageArray[])(void) = {
    		show_led_on,
			show_led_off,
			show_led_brightness
    };

    uint8_t num_of_demo_pages = (sizeof(ledPageArray)/sizeof(ledPageArray[0]));

    /* Initialize SPI and EINK display */
    result = cyhal_spi_init(&spi, CY8CKIT_028_EPD_PIN_DISPLAY_SPI_MOSI,
            CY8CKIT_028_EPD_PIN_DISPLAY_SPI_MISO,
            CY8CKIT_028_EPD_PIN_DISPLAY_SPI_SCLK, NC, NULL, 8,
            CYHAL_SPI_MODE_00_MSB, false);
    if (CY_RSLT_SUCCESS == result)
    {
        result = cyhal_spi_set_frequency(&spi, SPI_BAUD_RATE_HZ);
        if (CY_RSLT_SUCCESS == result)
        {
            result = mtb_e2271cs021_init(&pins, &spi);

            /* Set ambient temperature, in degree C, in order to perform temperature
             * compensation of E-INK parameters */
            mtb_e2271cs021_set_temp_factor(AMBIENT_TEMPERATURE_C);

            current_frame = (uint8_t*)LCD_GetDisplayBuffer();

            /* Initialize EmWin driver*/
            GUI_Init();

            /* Show the startup screen */
            show_startup_screen();

            /* Update the display */
            mtb_e2271cs021_show_frame(previous_frame, current_frame,
                                      MTB_E2271CS021_FULL_4STAGE, true);

            vTaskDelay(DELAY_AFTER_STARTUP_SCREEN_MS);

            /* Show the menu page screen */
            (*ledPageArray[page_number])();

            /* Update the display */
            mtb_e2271cs021_show_frame(previous_frame, current_frame,
                                      MTB_E2271CS021_FULL_4STAGE, true);

            for(;;)
            {
                /* Block until a command has been received over queue */
                rtos_api_result = xQueueReceive(eink_command_q, &eink_cmd_data,
                                    portMAX_DELAY);

                /* Command has been received from queue */
                if(rtos_api_result == pdTRUE)
                {
                    switch(eink_cmd_data)
                    {
                        /* Go back to the menu page from brightness page */
                        case EINK_BTN0:
                        {
                            if (led_brightness)
                            {
                        		led_brightness = false;
                                (*ledPageArray[page_number])();
                                mtb_e2271cs021_show_frame(previous_frame, current_frame,
                                                          MTB_E2271CS021_FULL_4STAGE, true);
                            }
                            break;
                        }
                        /* Act accordingly to the selected option */
                        case EINK_BTN1:
                        {
                            if(!led_brightness)
                            {
                            	if(page_number == 0){
                            		// turn on LED
                                    led_cmd_data.command = LED_TURN_ON;
                                    xQueueSendToBack(led_command_data_q, &led_cmd_data, 0u);
                            	}else if(page_number == 1){
                            		// turn off LED
                                    led_cmd_data.command = LED_TURN_OFF;
                                    xQueueSendToBack(led_command_data_q, &led_cmd_data, 0u);
                            	}else if(page_number == 2){
                            		// enter brightness page
                            		led_brightness = true;
                            		show_instructions_screen();
                                    mtb_e2271cs021_show_frame(previous_frame, current_frame,
                                                              MTB_E2271CS021_FULL_4STAGE, true);
                            	}
                            }
                            break;
                        }
                        /* switch to next option */
                        case EINK_RIGHT:
                        {
                            if(!led_brightness){
                                page_number = (page_number+1) % num_of_demo_pages;
                                (*ledPageArray[page_number])();
                                mtb_e2271cs021_show_frame(previous_frame, current_frame,
                                                          MTB_E2271CS021_FULL_2STAGE, true);
                            }
                            break;
                        }
                        /* switch to previous option */
                        case EINK_LEFT:
                        {
                            if(!led_brightness){
                                page_number = (page_number+num_of_demo_pages-1) % num_of_demo_pages;
                                (*ledPageArray[page_number])();
                                mtb_e2271cs021_show_frame(previous_frame, current_frame,
                                                          MTB_E2271CS021_FULL_2STAGE, true);
                            }
                            break;
                        }
                        /* Invalid command */
                        default:
                        {
                            /* Handle invalid command here */
                            break;
                        }
                    }
                }

                /* Task has timed out and received no data during an interval of
                 * portMAXDELAY ticks.
                 */
                else
                {
                    /* Handle timeout here */
                }
            }
        }
    }
}
