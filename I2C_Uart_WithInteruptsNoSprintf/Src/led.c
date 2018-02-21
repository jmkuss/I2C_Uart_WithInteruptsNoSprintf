/**
 * @file led.c
 * @brief
 * 		Use HAL functions to control the two LED's on the STM-32VL board.
 *
 *      Adapted from "stm32vldiscovery.c" using instead HAL functions to control the
 *      two LED's on the STM-32VL board
 *
 * 		Basic facts of this hardware:
 *
 * 		LED 3 Green, (LD3) is on port/pin PC9
 * 		LED 4 Blue, (LD4) is on port/pin PC8
 *
 * 		These LEDS are one when the ports are driven high (1)
 *
 * @author 	Joe Kuss (JMK)
 * @date 	2/13/2018
 *
 */

//
// This file or equiv was possibly auto-generated for STM32VL BOARD but was not
// auto-generated for the stmcube with just the STM32F100RBTx
// Since we still wanted to blink the LED's it was created for the,
// I2C_100Khz and I2C_Uart_WithInterupts projects.


#include "stm32f1xx_hal.h"
#include "led.h"

const uint16_t GPIO_PIN[LEDn] 		= {LD3_Green_Pin, LD4_Blue_Pin};

// Not needed since have it all done by "MX_GPIO_Init(void)"
//void STM32vldisc_LEDInit(Led_TypeDef Led)

/// Function Modified by JMK to use HAL call to turn on LED.
void STM32vldisc_LEDOn(Led_TypeDef Led)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN[Led], GPIO_PIN_SET);
}

/// Function Modified by JMK to use HAL call to turn off LED.
void STM32vldisc_LEDOff(Led_TypeDef Led)
{
	//GPIO_PORT[Led]->BRR = GPIO_PIN[Led];
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN[Led], GPIO_PIN_RESET);
}

/// Function Modified by JMK to use HAL call to toggle LED.
void STM32vldisc_LEDToggle(Led_TypeDef Led)
{
  //GPIO_PORT[Led]->ODR ^= GPIO_PIN[Led];
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN[Led]);
}

