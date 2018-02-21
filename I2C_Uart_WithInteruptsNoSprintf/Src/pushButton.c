/**
 * @file pushButton.c
 * @brief Supports reading the push buttons on STM VL discovery board.
 *	      Specifically support the single "User" blue push button.
 *
 * @author Joe Kuss (JMK)
 * @date 2/13/2018
 */

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include "pushButton.h"

/// Array to specify pin # for selected button.
const uint16_t BUTTON_PIN[BUTTONn] 	= {BlueButton_Pin};
/// Array to specify port address for selected button.
GPIO_TypeDef* BUTTON_PORT[BUTTONn] = {BlueButton_GPIO_Port};


/*
 This method not needed since PBInit is taken care of by  "MX_GPIO_Init(void)"
 in main.c, which is simplified but adequate.

void STM32vldisc_PBInit(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

   Enable the BUTTON Clock
  RCC_APB2PeriphClockCmd(BUTTON_CLK[Button] | RCC_APB2Periph_AFIO, ENABLE);

   Configure Button pin as input floating
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = BUTTON_PIN[Button];
  GPIO_Init(BUTTON_PORT[Button], &GPIO_InitStructure);

  if (Button_Mode == BUTTON_MODE_EXTI)
  {
     Connect Button EXTI Line to Button GPIO Pin
    GPIO_EXTILineConfig(BUTTON_PORT_SOURCE[Button], BUTTON_PIN_SOURCE[Button]);

     Configure Button EXTI line
    EXTI_InitStructure.EXTI_Line = BUTTON_EXTI_LINE[Button];
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;

    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

     Enable and set Button EXTI Interrupt to the lowest priority
    NVIC_InitStructure.NVIC_IRQChannel = BUTTON_IRQn[Button];
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
  }
}
*/

/**
  * @brief  Returns the selected Button state, for button specified.
  * This function designed to allow multiple ports and pins, for multiple buttons.
  * @param  Button: Specifies the Button to be checked.
  * Presently there is only one choice for this enumeration.
  * @arg BUTTON_USER: USER Blue Push Button
  * @retval The Button GPIO pin value.
  */

uint32_t STM32vldisc_PBGetState(Button_TypeDef Button)
{
	GPIO_PinState pinState;
	pinState = HAL_GPIO_ReadPin(BUTTON_PORT[Button], BUTTON_PIN[Button]);
	/// (When pinState is RESET, blue button not pressed on STM VL discovery board,
	///  this returns a zero. When pressed a one.)
	return (pinState == GPIO_PIN_RESET) ? (uint32_t) 0 : (uint32_t) 1;
}


/**
  * @brief  Returns pressed state of the blue "User" push button.
  * This function is specific for blue button on STM VL discovery board.
  * @param  Void - no parameter required.
  * @retval true - when blue button activated (pressed).
  */
bool STM32vldisc_BlueButtonPressed(void)
{
	GPIO_PinState pinState;
	/// When pinstate is not RESET, in other words is 1, then button was pressed.
	pinState = HAL_GPIO_ReadPin(BlueButton_GPIO_Port, BlueButton_Pin);
	return (pinState == GPIO_PIN_RESET) ? false : true;
}

