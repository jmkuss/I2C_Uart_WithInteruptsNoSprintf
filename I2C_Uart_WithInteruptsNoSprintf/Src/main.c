/**
* @file main.c
* @brief Main program body
*
* This source file is where top level of control for this demo firmware resides.
* The target platform is STM32VLDISCOVERY demo board with STM Arm Cortex M3
* p/n STM32F100RBT6B.
*
* @author Joe Kuss (JMK)
*
* @date 2/13/2018
*/

/*
  ******************************************************************************
  * File Name          : main.c
  * REGARDING HAL CODE, AUTO GENERATED OR PROVIDED AS EXAMPLES.
  ******************************************************************************
  ** This notice applies to any and all portions of this file that are
  *  autogenerated or modified from STM designed application examples.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdbool.h>
#include "main.h"

// STM HAL code:
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"


// JMK code:
#include "i2c_jmk.h"
#include "uart_jmk.h"
#include "led.h"
#include "pushButton.h"
#include "serialCmdParser.h"


/* External Variables ------------------------------------------------------- */
// JMK code:
extern uint8_t		Rx_data[2];
extern char			buffer[256];
extern uint8_t		Rx_Buffer[256];
extern bool			Transfer_cplt;

/* Private variables ---------------------------------------------------------*/
// STM HAL types utilized by JMK:
ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c2;
UART_HandleTypeDef huart1;

// JMK code:
/// count of 100 mS intervals, for blinking LED.
uint32_t count = 0;
/// Blink rate code for LED:
uint32_t BlinkSpeed = 0;
/// Previous blue button pressed state, 1 == pressed.
uint32_t KeyState = 0;

//uint16_t targetAddressU16;

/// Header msg displayed at startup
char msg[] = "Serial Command Interpreter: v0.02 Copyright Feb 12, 2018, J.M. Kuss \r\n\r\n";
uint8_t returnedByte;

/* Private function prototypes -----------------------------------------------*/
// STM code, and or modified by JMK:
static void SystemClock_Config(void); // added "static"
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C2_Init(void);
static void MX_ADC1_Init(void);

int main(void)
{

	// STM HAL  and initialization code:
	/* MCU Configuration */

	/** Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	/** Configure the system clock */
	SystemClock_Config();

	/** Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	MX_I2C2_Init();
	MX_ADC1_Init();

	/// Activate non blocking UART rx interrupt every time get 1 byte..
	HAL_UART_Receive_IT(&huart1, Rx_data, 1);

	// STM example code modified by JMK:
	STM32vldisc_LEDOff(LED3);
	STM32vldisc_LEDOff(LED4);

	// JMK code:

	// Introductory HW string ==========================
	UartPutString(msg, true);    // Blocking.

	while (1)
	{
		/// Respond to any serial commands sent via "cmdHandler"
		// Cmd_Recievd will be true if we receive the code 0x0D <13-Carriage Return>, in
		// the data stream indicating end of cmd msg. (It will also be true if we have
		// seen 255 bytes in the data stream since last Cmd_Recieved, as a back up,
		// to check incoming bytes before overflow data buffer.)

		if (Cmd_Recieved == true)
		{
			/// Go parse and execute latest command:
			cmdHandler(cmdBuffer);

			// Look for next "Transfer":
			Cmd_Recieved = false;
		}

		//================================

		/// Utilize LEDs and pushbuttons on STM32VLDISCOVERY demo board
		/// as alternative to serial commands, control panel human interface.

		if	( 0 == STM32vldisc_PBGetState(BUTTON_USER)	)  // 0 == USER BUTTON not pressed.
		{
			if(KeyState == 1)
			{
				// If previously pressed, and now continues not pressed:
				if(0 == STM32vldisc_PBGetState(BUTTON_USER)) // If not pressed.
				{
					/* USER Button released */
					// Finally set previous blue button state to released.
					KeyState = 0;
					/* Turn Off LED4 */
					STM32vldisc_LEDOff(LED4);
				}
			}
		}
		else if(STM32vldisc_PBGetState(BUTTON_USER)) // 1 == USER BUTON is pressed
		{
			if(KeyState == 0)
			{
				// If previously not pressed but now continues as pressed.
				if(STM32vldisc_PBGetState(BUTTON_USER))
				{
					// A new button pressed down transition
					// has occurred:

					/* USER Button pressed */
					// Finally set previous state to pressed.
					KeyState = 1;
					/* Turn ON LED4 - Indicate button pressed ! */
					STM32vldisc_LEDOn(LED4);

					/// Every time pushbutton is pressed, cycle among
					/// demoModes (test cases) : 1, 2, 3.

					switch (demoMode) {

					case 1:
						// This time do test case 1:

						// Next time do demo/test #2
						demoMode++;
						break;

					case 2:
						// This time do test case 2:

						// Next time do demo/test #3
						demoMode++;
						break;
					case 3:
						// This time do test case 3:

						// Next time do demo/test #1
						demoMode = 1;
						break;
					default:
						// backstop:
						demoMode = 1;
						break;
					}

					// Delay for 1 second after reacting to keypress down,
					// good for debouncing..

					HAL_Delay(1000);

					/* LED4 on for at least 1 sec after blue button down */
					STM32vldisc_LEDOff(LED4);

					// Presently expect BlinkSpeed 0,1,2, to follow demoMode 1->2->3.
					BlinkSpeed++ ;

				}	// if(STM32vldisc_PBGetState(BUTTON_USER))
			}	// if(KeyState == 0)
		}	// else if(STM32vldisc_PBGetState(BUTTON_USER))

		/// Final part of while loop will exec and blink LED continually
		/// depending on demo mode, except when in 1 sec delay after blue button down
		/// Blink speed has 3 rates:
		/// 1.25 Hz (DemoMode 1)
		/// 2.5 Hz  (DemoMode 2)
		/// 5 Hz    (DemoMode 3).

		count++;
		HAL_Delay(100);


		/* BlinkSpeed: 0 */
		if(BlinkSpeed == 0)
		{
			// While looking at count of 0..7, repeating:
			// Toggle LED as counts of 0 and 4:
			// When BlinkSpeed = 0 then LED is on 400mS , off 400mS.
			if(4 == (count % 8))
				STM32vldisc_LEDOn(LED3);
			if(0 == (count % 8))
				STM32vldisc_LEDOff(LED3);
		}

		/* BlinkSpeed: 1 */
		if(BlinkSpeed == 1)
		{
			// While looking at count of 0..3 repeating,
			// Toggle LED at counts of counts of 0 and 2
			// When BlinkSpeed = 1 then LED is on 200mS, off 200mS
			if(2 == (count % 4))
				STM32vldisc_LEDOn(LED3);
			if(0 == (count % 4))
				STM32vldisc_LEDOff(LED3);
		}
		/* BlinkSpeed: 2 */
		if(BlinkSpeed == 2)
		{
			// While looking at count of 0 to 1 repeating,
			// Toggle LED at counts of 0 and 1
			// When BlinkSpeed = 2 then LED is on 100mS, off 100mS
			if(0 == (count % 2))
				STM32vldisc_LEDOn(LED3);
			else
				STM32vldisc_LEDOff(LED3);
		}
		/* BlinkSpeed: 3 */
		else if(BlinkSpeed == 3)
		{
			BlinkSpeed = 0;
		}

  } // End while (1)

} // End main()

//##########################################################
//	Note: SystemClock_Config(void) and
//	MX_xxx functions below are all auto-generated by  "STM32CubeMX.exe"
//
//##########################################################

/** 	System Clock Configuration
*/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  /**	Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Configure the Systick interrupt time
  */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  /**Configure the Systick
  */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /** Configure the SysTick_IRQn interrupt */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/** ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

  /**Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

/** I2C2 init function */
static void MX_I2C2_Init(void)
{

  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /** GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /** Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LD4_Blue_Pin|LD3_Green_Pin, GPIO_PIN_RESET);

  /** Configure GPIO pins : LD4_Blue_Pin LD3_Green_Pin */
  GPIO_InitStruct.Pin = LD4_Blue_Pin|LD3_Green_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /// User can add his own implementation to report the HAL error return state */
  /// ==> Right here:
  while(1) 
  {
	  // A way of ensuring a NOP, even if code optimizer is on:
	  // http://www.ethernut.de/en/documents/arm-inline-asm.html
	  asm volatile("mov r0, r0");
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

