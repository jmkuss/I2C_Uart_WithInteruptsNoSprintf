/**
 * @file pushButton.h
 * @brief Contains declarations/defines for pushButton.c
 *
 * @author Joe Kuss (JMK)
 * @date 2/13/2018
 */


#ifndef PUSH_BUTTON_H_
#define PUSH_BUTTON_H_

/* Includes ------------------------------------------------------------------*/

// Blue Button is at PA0
#define BlueButton_Pin GPIO_PIN_0
#define BlueButton_GPIO_Port GPIOA

#define BUTTONn			1


/// Enum used in case we have multiple user buttons
typedef enum
{
  BUTTON_USER = 0
} Button_TypeDef;

/// Enum used in case we have multiple button input modes.
typedef enum
{
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef;


/* Function Prototypes: */

uint32_t STM32vldisc_PBGetState(Button_TypeDef Button);

/// Use instead of  STM32vldisc_PBGetState(Button_TypeDef Button)
/// to get state of demo board Blue button, for user friendliness
uint32_t demoBrdBlueButtonPressed(void);

#endif /* LED_H_ */
