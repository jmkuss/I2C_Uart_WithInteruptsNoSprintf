/**
 * @file led.h
 * @brief Contains declarations/defines for led.c
 *
 * @author Joe Kuss (JMK)
 * @date 2/13/2018
 */
// This file or equiv was possibly autogenerated for STM32VL BOARD but was not
// autogenerated for the stmcube with just the STM32F100RBTx
// Since we still wanted to blink the LED's it was hand inserted,
// into the I2C_100Khz and I2C_Uart_WithInterupts projects.

#ifndef LED_H_
#define LED_H_

/* Includes ------------------------------------------------------------------*/

// Blue LED - LD4 is at PC8
#define LD4_Blue_Pin GPIO_PIN_8
#define LD4_Blue_GPIO_Port GPIOC

// Green LED - LD3 is at PC9
#define LD3_Green_Pin GPIO_PIN_9
#define LD3_Green_GPIO_Port GPIOC


#define LEDn   			2

typedef enum
{
  LED3 = 0,
  LED4 = 1
} Led_TypeDef;


/* Function Prototypes: */
void STM32vldisc_LEDOn(Led_TypeDef Led);
void STM32vldisc_LEDOff(Led_TypeDef Led);
void STM32vldisc_LEDToggle(Led_TypeDef Led);

#endif /* LED_H_ */
