/******************************************************************************
*
* File Name: eink_task.h
*
* Description: This file contains declaration of task and functions related to
* the E-Ink task.
*
*******************************************************************************
* The code is modified based on the example code provided by Infineon
* Example Code Name: emWin E-Ink FreeRTOS
* Modifier: Yen-Chen Chang @ makestraight
*******************************************************************************
* Copyright 2019-2022, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#ifndef EINK_TASK_H_
#define EINK_TASK_H_
/*******************************************************************************
 * Enumeration
 ******************************************************************************/
typedef enum
{
    EINK_BTN0,
    EINK_BTN1,
    EINK_RIGHT,
    EINK_LEFT
} eink_command_t;


/*******************************************************************************
 * Global variable
 ******************************************************************************/
extern QueueHandle_t eink_command_q;

void eInk_task(void *arg);

#endif /* EINK_TASK_H_ */
