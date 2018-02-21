/**
  @file uart_jmk.h
  @brief
<pre>

</pre>

   @author 	Joe Kuss (JMK)
   @date 	11/13/2017 - Original.
   @date 	2/20/2018  - Added Doxygen comments, etc.

*/
#ifndef UART_JMK_H_
#define UART_JMK_H_

#define MSG_MAX_CHARS 255


extern char		cmdBuffer[MSG_MAX_CHARS+1];
extern char 	respBuffer[MSG_MAX_CHARS+1];   // May change size
extern char		strOf20CharsMax[21];			//+1 for null
extern char 	*crlf_msg;
extern bool		Cmd_Recieved;

/* ------------ Function Prototypes --------------------------------------*/
void UartPutString(char *strToTransmit, bool isBlocking);


#endif /* UART_JMK_H_ */
