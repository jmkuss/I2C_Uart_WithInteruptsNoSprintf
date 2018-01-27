/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
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
//#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include "i2c_jmk.h"
#include "uart_jmk.h"
#include "led.h"
//#include "flipTheArray.h"
#include "serialCmdParser.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
/* External Variables ------------------------------------------------------- */
extern uint8_t		Rx_data[2];
extern char			buffer[256];
extern uint8_t		Rx_Buffer[256];
extern bool			Transfer_cplt;

/* Private variables ---------------------------------------------------------*/


ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart1;



/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/


uint32_t count = 0;
uint32_t BlinkSpeed = 0;
uint32_t KeyState = 0;
uint32_t i2cByteSendAttempts = 0;

uint32_t I2C_demoMode = 1;

uint16_t targetAddressU16;

char msg[] = "Serial Command Interpreter: v0.01 Copyright Jan 22, 2018, J.M. Kuss \r\n\r\n";


// This buffer will be cleared to all zeros to demonstrate I2C can fill it back up.
static uint8_t EEPROM_Buffer[] =
   {
       0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
       0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
       0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
       0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
       0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
       0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
       0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
       0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
   };

uint8_t returnedByte;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C2_Init(void);
static void MX_ADC1_Init(void);

void clearEEPROM_Buffer(void) {
	int i;
	for (i = 0; i<sizeof(EEPROM_Buffer);i++ )  { EEPROM_Buffer[i] = 0x00; }
}

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  MX_ADC1_Init();

  HAL_UART_Receive_IT(&huart1, Rx_data, 1); // Activate non blocking UART rx interrupt every time get 1 byte..

  /* USER CODE BEGIN 2 */
  STM32vldisc_LEDOff(LED3);
  STM32vldisc_LEDOff(LED4);

  //##### Not doing these things FBOW.
  /* Enable access to the backup register => LSE can be enabled */
  /* Enable LSE (Low Speed External Oscillation) */
  /* Check the LSE Status */

  // PIC I2C ADDR is 08 but it seems our system, HAL
  // does not shift right this value by one to fit in I2C stream,
  // so we do it here:

  targetAddressU16 = 0x0008;
  targetAddressU16 <<=1;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  // Introductory HW string ==========================
   UartPutString(msg, true);    // Blocking.

  while (1)
  {
	  //=================================
	  if (Cmd_Recieved == true)
	  {
		  cmdHandler(cmdBuffer);

		  // The receive is picked up by ISR, and handled by the callback
		  // routine "HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)", in uart_jmk.c
		  // This routine will flag "Transfer_cplt" which occurs every time terminal
		  // sends a CR 0x0D charactor.

		  // Rx_Buffer is a zero terminated string that has the latest
		  // incoming cmd, that must always end with a <CR> which is used to trigger
		  // Transfer_cplt but the <CR> is presently otherwise discarded.

		  /* ECHO DEMO CODE *****************************************************
		  sprintf(buffer, "%s\r\n",Rx_Buffer);
		  len = strlen(buffer);


		  // Then we echo back to the terminal what we received since the last CR:
		  // -------------------------------------------------------------
		  // Note: This original function is actually "blocking" i.e. not
		  //       interrupt based, although rx of bytes is "non-blocking" interrupt based..
		  //       A call to "HAL_UART_Transmit_IT(...)" could be made to make it non-blocking,
		  //       and this might also work as well...
		  // Other comments:
		  //       During the time that the response is being
		  //       transmitted to other side, what happens if the other side
		  //       start transmitting back to us. In the case of blocking, for tx back,
		  //       It may still be possible to have RX bytes incoming interrupts serviced.
		  //       In the case of non-blocking, we continue past line 227 without
		  //       completing the TX of all bytes in the buffer.
		  //
		  //       Will experiment with non-blocking to see if it still works ok,
		  //       under no stress normal conditions. (Non-blocking is more complex,
		  //       and may not really be needed unless other system services can not
		  //       be tied up during a transmit.)

		  UartPutString(cmdBuffer, false);    // Non-Blocking.



		  *************************************************************************/

		  // Look for next "Transfer":
		  Cmd_Recieved = false;
	  }
	  //================================

	  if(0 == STM32vldisc_PBGetState(BUTTON_USER))
	  {
		if(KeyState == 1)
		{
		  if(0 == STM32vldisc_PBGetState(BUTTON_USER))
		  {
			/* USER Button released */
			  KeyState = 0;
			/* Turn Off LED4 */
			  STM32vldisc_LEDOff(LED4);
		  }
		}
	  }
	  else if(STM32vldisc_PBGetState(BUTTON_USER))
	  {
		if(KeyState == 0)
		{
		   if(STM32vldisc_PBGetState(BUTTON_USER))
		  {
			/* USER Button released */
			  KeyState = 1;
			/* Turn ON LED4 */
			STM32vldisc_LEDOn(LED4);

			//########################

			// For the PIC16F15376 demo board and demo firmware,
			// Board acts like an "EEPROM" with 128 bytes of data,
			// But addressing these bytes is crude, from power cycle,
			// reading or writing bytes starts at addr 0 and then will progress,
			// from 0 to 127, and then loop back to 0.
			// Normal eeproms work a little differently...

//#define FIRST
#ifdef FIRST
			// First trial:
			//This below just writes 0x55 to I2C at PIC device addr (Ox08 actually)
			//Data in the eeprom is not directly addressable, but will start at 0
			//and advance by 1 for each byte read or written to by us,
			//(It will overflow at 127 and go back to 0)
			//Only way to know where eeprom data, index is presently is to
			//reset the PIC (restart it's program) to know index is 0

			I2C_WriteOneByte(&hi2c2, targetAddressU16, (uint8_t) 0x55);
			i2cByteSendAttempts++;
			HAL_Delay(100);
#else
			// As a couple demo tricks we could create commands to
			// 1) Read all 128 bytes starting at I2C addr of 0 (to 127)
			// 2) Write all 128 bytes to 0x00 , and then read back..
			// Assuming addr of 0x00 is legit for this system as configured..
			//########### clearEEPROM_Buffer();
			switch (I2C_demoMode) {
/*
			case 0:
				// Write the byte at I2C addr 0x7e to 0x55
				// Keep in mind the addr really needs to be shifted 1 left..
				I2C_WriteOneByte(&hi2c2, targetAddressU16, (uint8_t) 0x55);
				// Read the byte back to verify:
				returnedByte = 0xff;
				I2C_ReadOneByte(&hi2c2, targetAddressU16, returnedByte);
				I2C_demoMode++;
				break;
*/

			case 1:
				// Read all 128 of the bytes back from the slave PIC16F15376:
				// Clear local buffer before it to know we really are reading it back.
				clearEEPROM_Buffer();
				//MX_I2C_ReadBytes(&hi2c2,targetAddressU16,EEPROM_Buffer, sizeof(EEPROM_Buffer) );
				STM32vldisc_LEDToggle(LED3);
				RandomAccesPicEEReadBytes(&hi2c2, targetAddressU16, (uint8_t)0, EEPROM_Buffer,sizeof(EEPROM_Buffer) );
				STM32vldisc_LEDToggle(LED3);

				I2C_demoMode++;
				break;
			case 2:
				// This time actually clear the buffer on the PIC side:
				clearEEPROM_Buffer();
				//MX_I2C_WriteBytes(&hi2c2,targetAddressU16,EEPROM_Buffer, sizeof(EEPROM_Buffer) );
				//MX_I2C_ReadBytes(&hi2c2,targetAddressU16,EEPROM_Buffer, sizeof(EEPROM_Buffer) );
				//!!!! What is very important to notice is that the interrupt based "PicEEWriteBytes"
				//!!!! is not blocking and does not just sit there during the call to it and wait
				//!!!! for all the bytes to come in... The isr will keep
				//!!!! being issued while other code lines below will be executed, well if you
				//!!!! attempt to issue a read on the same data before the write is completed, that
				//!!!! is going to cause conflicts, therefore you must wait for the data operation
				//     to complete before you do another one, (you could do independent processing,
				//     though...)

				STM32vldisc_LEDToggle(LED3);
				RandomAccesPicEEWriteBytes(&hi2c2, targetAddressU16, (uint8_t)0, EEPROM_Buffer,sizeof(EEPROM_Buffer));
				STM32vldisc_LEDToggle(LED3);

				STM32vldisc_LEDToggle(LED3);
				RandomAccesPicEEReadBytes(&hi2c2, targetAddressU16, (uint8_t)0, EEPROM_Buffer,sizeof(EEPROM_Buffer));
				STM32vldisc_LEDToggle(LED3);
				I2C_demoMode = 1;
				break;
			default:
				I2C_demoMode = 1;
				break;
			}


			//########################
#endif
			//Delay(1000);
			HAL_Delay(1000);

			/* Turn OFF LED4 */
			STM32vldisc_LEDOff(LED4);
			/* BlinkSpeed: 0 -> 1 -> 2, then re-cycle */
			  BlinkSpeed ++ ;
		  }							// if(STM32vldisc_PBGetState(BUTTON_USER))
		}							// if(KeyState == 0)
	  }								// else if(STM32vldisc_PBGetState(BUTTON_USER))
	  count++;
	  //Delay(100);
	  HAL_Delay(100);
	  /* BlinkSpeed: 0 */
	  if(BlinkSpeed == 0)
      {
		  if(4 == (count % 8))
		  STM32vldisc_LEDOn(LED3);
		  if(0 == (count % 8))
		  STM32vldisc_LEDOff(LED3);
	  }
	  /* BlinkSpeed: 1 */
	  if(BlinkSpeed == 1)
	  {
		if(2 == (count % 4))
		STM32vldisc_LEDOn(LED3);
		if(0 == (count % 4))
		STM32vldisc_LEDOff(LED3);
	  }
	  /* BlinkSpeed: 2 */
	  if(BlinkSpeed == 2)
	  {
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
  /* USER CODE END WHILE */
  }
  /* USER CODE BEGIN 3 */
/* Old auto generated little stub , try playing with it sometime...*/
// #### How was the timing regulated ???? ########
//	    if(0 == STM32vldiscovery_PBGetState(BUTTON_USER))
//	    {
//	      /* Toggle LED3 */
//	      STM32vldiscovery_LEDToggle(LED3);
//	      /* Turn Off LED4 */
//	      STM32vldiscovery_LEDOff(LED4);
//	    }
//	    else
//	    {
//	      /* Toggle LED4 */
//	        STM32vldiscovery_LEDToggle(LED4);
//	      /* Turn Off LED3 */
//	      STM32vldiscovery_LEDOff(LED3);
//	    }
//  	  }
  /* USER CODE END 3 */

}

//##########################################################
//Note: SystemClock_Config(void) and
//MX_xxx functions below are all auto-generated by  "STM32CubeMX.exe"
//##########################################################
/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
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

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
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

/* I2C2 init function */
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

/* USART1 init function */
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

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LD4_Blue_Pin|LD3_Green_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LD4_Blue_Pin LD3_Green_Pin */
  GPIO_InitStruct.Pin = LD4_Blue_Pin|LD3_Green_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
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

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
