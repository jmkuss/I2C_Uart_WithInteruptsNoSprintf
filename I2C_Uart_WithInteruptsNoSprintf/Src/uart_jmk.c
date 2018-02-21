/**
  @file uart_jmk.c
  @brief
<pre>

</pre>

   @author 	Joe Kuss (JMK)
   @date 	11/08/2017 - Original.
   @date 	2/19/2018  - Added Doxygen comments, etc.

*/
#include <stdbool.h>
#include <string.h>
#include "stm32f100xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include "uart_jmk.h"



extern UART_HandleTypeDef huart1;

/* Globally available buffers ------------------------------ */

/// Incoming Data as picked up in "HAL_UART_Receive_IT" isr.
uint8_t	Rx_data[2];

/// Buffer holding latest incoming command.
char	cmdBuffer[MSG_MAX_CHARS +1];

/// Buffer holding outgoing response to command.
char 	respBuffer[MSG_MAX_CHARS+1];

char	strOf20CharsMax[21];
char 	*crlf_msg = "\r\n";

/// Flag indicating Command received, but not yet parsed/executed.
bool	Cmd_Recieved = false;

/* Private variables --------------------------------------- */
int 	i=0;



uint8_t	Rx_index=0;

uint8_t	Rx_Buffer[MSG_MAX_CHARS +1]; // >= 1 of 0x00 to terminate msg string.



//  HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
/**
 * @brief Called upon reception of incoming UART data
 * <pre>
 * This routine is called by "UART_Receive_IT" which is
 * Called by HAL_UART_IRQHandler upon UART interrupt.
 *
 * Upon completion this routine calls HAL_UART_Receive_IT
 * which re activates UART interrupts to pick up the next byte.
 *
 * This routine replaces the "__weak " version of this routine,
 * located in stm32f1xx_hal_uart.c
 * </pre>
 *
 * @param huart - pointer to UART_HandleTypeDef
 *
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint32_t i;
	if (huart->Instance == USART1) // Current UART
	{
		if (Rx_index == 0)
		{
			for (i=0; i<256; i++)
			{
				Rx_Buffer[i]=0; // Clear buffer before getting new data.
								// This assures that it is 0 terminated.
			}
		}

		// We presently require that end of line from "Enter" key,
		// produces only a <CR> (0x0D) and not a <LF> (0x0A).
		// This received byte <CR> is presently used as the end of message flag,
		// and is not put into Rx_Buffer.

		if ((Rx_data[0] != 0x0D) && (Rx_index<255))// If received data different than 13 <CR>
		{										   // and we are not about to overflow buffer,
												   // presently limit msg to 255 chars.
												   // index 0..254 so Rx_Buffer[255] remains 0
			Rx_Buffer[Rx_index++] = Rx_data[0];	   // to terminate string.

		}
		else
		{
			// If we receive Rx_data[0] == 0x0D, this byte will be ignored it is only a flag.
			// Also if Rx_index == 255 we will also end up here to attempt to parse
			// what we have so far since we will loose data if we go any further.

			// Rx_index is set to zero for next time that we come back to get more
			// incoming bytes.
			Rx_index = 0;

			strcpy(cmdBuffer, (char *) Rx_Buffer); // Save the raw buffer into cmd buffer.

			Cmd_Recieved = true;	// Indicate  potential command received
		}
		HAL_UART_Receive_IT(&huart1, Rx_data, 1); // Activate UART rx interrupt every time get 1 byte..
	}
}		// end HAL_UART_RxCpltCallback


/**
 * @brief Called when sending a string via the UART
 * <pre>
 *
 *	Note:	Blocking TX means that routine will not exit
 *			until all bytes are sent.
 *			Not blocking TX means that routine sets up
 *			Interrupts to send individual bytes but can
 *			exit after that.
 *
 *			Not blocking allows other processing to resume
 *		    before all bytes are sent, but it may NOT mean
 *		    it is ok to attempt to send another message before
 *		    all bytes are sent.
 *
 *
 * </pre>
 *
 * @param strToTransmit - pointer to null terminated character array to TX.
 * @param isBlocking - true for blocking TX of all bytes, false for non blocking.
 *
 *
 */
void UartPutString(char *strToTransmit, bool isBlocking)
{
	HAL_StatusTypeDef eHAL_Status;
	size_t 			  len;

	len = 			strlen(strToTransmit);

	if (isBlocking == true)
	{
		eHAL_Status = 	HAL_UART_Transmit(&huart1, (uint8_t *)strToTransmit, len, 1000); // Blocking.
		// Will and wait inside this call up to 1000ms, for transmission of all bytes to complete
	}
	else
	{
		eHAL_Status = HAL_UART_Transmit_IT(&huart1, (uint8_t *)strToTransmit, len); // Non-Blocking.
		// Will not wait for transmission of all bytes to complete, ISR's handle all needed bytes
		HAL_Delay(100); // May need this delay, Especially if we attempt to initiate
						// another transmission before this one has completed.
						// Might expect to be able to send 1000 chars in 100 ms at 115Kb...
	}
}
