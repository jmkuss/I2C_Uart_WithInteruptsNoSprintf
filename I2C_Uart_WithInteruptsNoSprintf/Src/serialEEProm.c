/**
  @file serialEEProm.c
  @brief Functions to support read//write to serial eeproms.
<pre>
	Presently the target device is Atmel AT24C256.
	Ref: AT24C128/256 data sheets - 0670F–SEEPR–2/02

</pre>

   @author 	Joe Kuss (JMK)
   @date 	03/01/2018

*/
#include "serialEEProm.h"
#include "led.h"
#include "main.h"
#include <stddef.h>

/// Device Addr needed by HAL I2C routines (7bit addr << 1)
uint16_t targetAddressU16;

/// Single eePromByte that has just been read.
uint8_t eePromByteRead;

//############ Just for expermental purposes:
pageByteArrayStruct bas1;

// But here is a possible trick:
// https://www.embedded.com/design/prototyping-and-development/4024941/Learn-a-new-trick-with-the-offsetof--macro
// "the offsetof() macro returns the number of bytes of offset before a particular element of a struct or union. "
// In the case below the array is of bytes in the page, so bytes in page (size of array is just this offset).
pageByteArrayStruct bas2 = { .array={0},.bytesInPage = offsetof(pageByteArrayStruct,bytesInPage)};
//################

/// Mutiple eePromBytes, <= page size, just read.
pageByteArrayStruct eePromReadPageBytes = { .array={0},.bytesInPage = AT24C_PAGE_SIZE };

/// Single eePromByte to be written.
uint8_t eePromByteToBeWritten;

/// Mutiple eePromBytes, <= page size, to be written

uint8_t value = sizeof(pageByteArrayStruct);

pageByteArrayStruct eePromWritePageBytes = { .array={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		   	   	   	   	   	   	   	   	   	   	  0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
												  0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
												  0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
												  0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
												  0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
												  0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
												  0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F},
												  //.bytesInPage=sizeof(pageByteArrayStruct.array) }; //
												  .bytesInPage=sizeof(eePromReadPageBytes.array) };

// Note for previous line: you can not do sizeof an inner component of a struct via the structure
// typedef name, instead you must use a previous instantiaton of the struct, which here is
// eePromReadPageBytes - This is a work around for something that would be better if it
// was allowed... ###### We still have problems since we really need to have even an
// un-initialized version of pageByteArrayStruct to have .bytesInPage = size of array, but this is
// chicken before the egg problem.

// But here is a possible trick:
// https://www.embedded.com/design/prototyping-and-development/4024941/Learn-a-new-trick-with-the-offsetof--macro


void sEEPromPageBufferFill(pageByteArrayStruct* pByteArrayStruct, enumArrayFillType fillType )
{
	int i;

	for (i=0; i < pByteArrayStruct->bytesInPage; i++)
	{
		switch ( fillType ){
			case FILL_0:
				pByteArrayStruct->array[i] = 0x00;
				break;
			case FILL_FF:
				pByteArrayStruct->array[i] = 0xFF;
				break;
			case FILL_INDEX:
				pByteArrayStruct->array[i] = (uint8_t)i;
				break;
			case FILL_REVERSE_INDEX:
				pByteArrayStruct->array[i] = (pByteArrayStruct->bytesInPage -1) - (uint8_t)i; //presently 63 down to 0..
				break;
		}
	}
}

/*  >>>>>>>>> Ref: AT24C128/256 data sheets - 0670F–SEEPR–2/02
Part is organized as 32,768 x 8

which is k bits of 256

For AT24C256
1 MHz (5V), 400 kHz (2.7V, 2.5V) and 100 kHz (1.8V) Compatibility

64-byte Page Write Mode (Partial Page Writes Allowed)


Self-timed Write Cycle (5 ms Typical, 10ms Max @ 5 or 2.5 volts)  - not exactly blazing speed writes !

High Reliability
– Endurance: 100,000 Write Cycles, But for AT24C256C it is 1,000,000 Write Cycles
– Data Retention: 40 Years


The SCL input is used to positive edge clock data into each
EEPROM device and negative edge clock data out of each device.


The SDA pin is bidirectional for serial data transfer. This pin is opendrain
driven and may be wire-ORed with any number of other open-drain or open collector
devices.


The 128K/256K is internally organized as 256/512 pages of 64-bytes each.
Random word addressing requires a 14/15-bit data word address.

512 pages of 64 bytes or 512 bits, 512^^2 bits == 256K bits

Write cycle time for 5 or 2.5 volts Vcc is 10ms

Basic I2C rules:

The SDA pin is normally pulled high with an external
device. Data on the SDA pin may change only during SCL low time periods (refer to Data
Validity timing diagram). Data changes during SCL high periods will indicate a start or stop
condition as defined below.

START CONDITION: A high-to-low transition of SDA with SCL high is a start condition which
must precede any other command (refer to Start and Stop Definition timing diagram).

STOP CONDITION: A low-to-high transition of SDA with SCL high is a stop condition. After a
read sequence, the stop command will place the EEPROM in a standby power mode (refer to
Start and Stop Definition timing diagram).

All addresses and data words are serially transmitted to and from the
EEPROM in 8-bit words. The EEPROM sends a zero during the ninth clock cycle to acknowledge
that it has received each word.<< when it is being written to..>>

MEMORY RESET: After an interruption in protocol, power loss or system reset, any 2-wire
part can be reset by following these steps: (a) Clock up to 9 cycles, (b) look for SDA high in
each cycle while SCL is high and then (c) create a start condition as SDA is high.


Note: 1. The write cycle time tWR is the time from a valid stop condition of a
write sequence to the end of the internal clear/write cycle. This is 10ms max - Slow !

The A1 and A0 pins use an internal proprietary circuit that biases them to a logic low condition if the
pins are allowed to float.

How to talk to read/write Memory:

==========================================================================================
BYTE WRITE: A write operation requires two 8-bit data word addresses following the device
address word and acknowledgment. Upon receipt of this address, the EEPROM will again
respond with a zero and then clock in the first 8-bit data word. Following receipt of the 8-bit
data word, the EEPROM will output a zero. The addressing device, such as a microcontroller,
then must terminate the write sequence with a stop condition. At this time the EEPROM
enters an internally-timed write cycle, tWR, to the nonvolatile memory. All inputs are disabled
during this write cycle and the EEPROM will not respond until the write is complete (refer to
Figure 2).

Refering to Figure 2: Looks like word address can be 0 to 7FFF i.e. 0 to 32767 for 256K bit part,
so we can write any byte address directly it seems, but then we must delay tWR to do the next byte
tWR =  10ms
==========================================================================================
PAGE WRITE: The 128K/256K EEPROM is capable of 64-byte page writes.
A page write is initiated the same way as a byte write, but the microcontroller does not send a
stop condition after the first data word is clocked in. Instead, after the EEPROM acknowledges
receipt of the first data word, the microcontroller can transmit up to 63 more data words. The
EEPROM will respond with a zero after each data word received. The microcontroller must
terminate the page write sequence with a stop condition (refer to Figure 3).

The data word address lower 6 bits are internally incremented following the receipt of each
data word. The higher data word address bits are not incremented, retaining the memory page
row location. When the word address, internally generated, reaches the page boundary, the
following byte is placed at the beginning of the same page. If more than 64 data words are
transmitted to the EEPROM, the data word address will “roll over” and previous data will be
overwritten. The address “roll over” during write is from the last byte of the current page to the
first byte of the same page.

<<So it looks like a page write could be used to update a string of bytes starting
anywhere in the 64 byte page, but the internal hardware will, cause the 6 lower bits
of the address (representing locations in the page) to increment once for each consecutive
byte to be written sent,so if we keep writing we will end up overflowing and going
back to byte addr 0 of this page. For max efficiency we might want to update
64 bytes at a time.>>

ACKNOWLEDGE POLLING: Once the internally-timed write cycle has started and the
EEPROM inputs are disabled, acknowledge polling can be initiated. This involves sending a
start condition followed by the device address word. The read/write bit is representative of the
operation desired. Only if the internal write cycle has completed will the EEPROM respond
with a zero, allowing the read or write sequence to continue.

So one might estimate the fastest time to write out all 32K bytes would be (32K/64 * 10mS)
+ time to clock out the 64 bytes x 512 pages: (tck64 * 32K/64) = 512*10ms (5.12 seconds + 512*tck64)
Note: 32K/64 = 512 pages of 64 bytes in a 256Kbit eeprom.
==========================================================================================

Read
Operations
Read operations are initiated the same way as write operations with the exception that the
read/write select bit in the device address word is set to one. There are three read operations:
current address read, random address read and sequential read.

CURRENT ADDRESS READ: The internal data word address counter maintains the last
address accessed during the last read or write operation, incremented by one. This address
stays valid between operations as long as the chip power is maintained. The address “roll
over” during read is from the last byte of the last memory page, to the first byte of the first
page.

Once the device address with the read/write select bit set to one is clocked in and acknowledged
by the EEPROM, the current address data word is serially clocked out. The
microcontroller does not respond with an input zero but does generate a following stop condition

<< Nice thing aboout this is that the CURRENT ADDRESS READ does not require sending
   the device address byte twice (2nd time with repeated start condx) to change the modality
   from write (first send the eeprom two bytes of internal starting address), to  then read
   (recieve the byte or bytes from the eeprom.>>

(refer to Figure 4).

RANDOM READ: A random read requires a “dummy” byte write sequence to load in the data
word address. Once the device address word and data word address are clocked in and
acknowledged by the EEPROM, the microcontroller must generate another start condition.
The microcontroller now initiates a current address read by sending a device address with the
read/write select bit high. The EEPROM acknowledges the device address and serially clocks
out the data word. The microcontroller does not respond with a zero <ack> but does generate a following
stop condition (refer to Figure 5).

SEQUENTIAL READ: Sequential reads are initiated by either a current address read or a random
address read. After the microcontroller receives a data word, it responds with an
acknowledge. As long as the EEPROM receives an acknowledge, it will continue to increment
the data word address and serially clock out sequential data words. When the memory
address limit is reached, the data word address will “roll over” and the sequential read will continue.
The sequential read operation is terminated when the microcontroller does not respond
with a zero <ack> but does generate a following stop condition (refer to Figure 6).

Note reads are available upon completion of I2C clocking sequence and no further
tWR delay is needed in between reads.

 */

/* Type reference only
HAL_StatusTypeDef HAL_I2C_Mem_Write_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
*/


/**
 * @brief Write a single byte into the serial EEprom.
 * <pre>
 *	This function will write a single byte to a particular eeprom device
 *	specified by DevAddress, into its memory at location EEaddress.
 *
 *	This function is "non blocking", so it just starts this process running, by
 *	passing in for the I2C_Handle structure the desired communications information,
 *	and then calling the HAL initiation function: HAL_I2C_Mem_Write_IT(..),
 *	which updates the I2C_Handle structure for the duration this process will run.
 *
 *	The total process is expected to run as follows:
 *  @li Send Start condx. Send one byte Device address, with R/W* bit as W*.
 *  @li Send Two I2C bytes for two byte EEaddress. (Low Ack bit returned after each byte.)
 *  @li Send one I2C byte to write data to EEaddress. (Low Ack bit seen if ok)
 *  @li Send Stop condx.
 *
 *  @note Expected return status will be "HAL_BUSY" ??? I believe ???
 *  @note Some time in future status will be "HAL_OK" once above 4 step process is complete.
 *  @note IMPORTANT - Write process is not actually complete until tWR time after "HAL_OK"
 *  				  tWR is specified as a worst case of 10ms. We must either:
 *  				  1) Setup a timer to prevent future writes until tWR is elapsed, or
 *  				  2) Implement "ACKNOWLEDGE POLLING" as mentioned in the data sheet
 *						 ("ACKNOWLEDGE POLLING" method will allow faster response.)
 * </pre>
 *
 * @param hi2c 		 - pointer to I2C_HandleTypeDef, (info struct for this I2C transaction in process).
 * @param DevAddress - I2C device address to load into *hi2c. (normally only use 8 bits or less)
 * @param EEaddress  - EE memory address to load into *hi2c. (0-0x7FFF, 32K 24C256K bit part)
 * @param pByteBuffer - Pointer to 1 byte transmission buffer, external to this function.
 *
 * @returns retVal	  - One of: {HAL_OK, HAL_ERROR, HAL_BUSY, HAL_TIMEOUT}
 *
 */
HAL_StatusTypeDef sEEPromByteWrite(I2C_HandleTypeDef *hi2c, enumAT24C_7BitAddr addr7Bit, uint16_t EEaddress, uint8_t *pByteBuffer) {

	HAL_StatusTypeDef I2CWriteStatus;

	// The HAL device addr is u16, (7 bit addr shifted 1 bit left).
	uint16_t HAL_DevAddr = ((uint16_t)addr7Bit << 1);

	while (hi2c->State != HAL_I2C_STATE_READY)
	{
		// While transfer count from previous is not zero we are still sending bytes..
		// asm("nop");

		STM32vldisc_LEDToggle(LED3);
	}
	// Non blocking (interrupt based) call to write one byte.
	I2CWriteStatus = HAL_I2C_Mem_Write_IT(hi2c, HAL_DevAddr, EEaddress, (uint16_t)2, pByteBuffer, (uint16_t) 1);

	return I2CWriteStatus;
}

/**
 * @brief Write a multiple bytes, max of 64 (page size) into serial EEprom.
 * <pre>
 *	This function will write a multiple bytes to a particular eeprom device
 *	specified by DevAddress, Starting at memory at location EEaddress.
 *
 *	Optimal use for this function is to write a full page at a time.
 *	This requires 64 bytes always sent, and EEaddress to always have
 *	lower 6 bits as 0, to start at selected page boundary.
 *
 *	Other uses allow less than 64 bytes to be written, but then
 *	"last 6 bits of EEaddress" + "bufferlength"  must be <= 64.
 *	If this rule is not followed, upper bytes of data written
 *	will fill into areas before the starting EEaddress starting
 *	with last 6 bytes of EEaddress at zero.
 *
 *	This function is "non blocking", so it just starts this process running, by
 *	passing in for the I2C_Handle structure the desired communications information,
 *	and then calling the HAL initiation function: HAL_I2C_Mem_Write_IT(..),
 *	which updates the I2C_Handle structure for the duration this process will run.
 *
 *	The total process is expected to run as follows:
 *  @li Send Start condx. Send one byte Device address, with R/W* bit as W*.
 *  @li Send Two I2C bytes for two byte EEaddress. (Low Ack bit returned after each byte.)
 *  @li Send 1 to 64 I2C bytes to write data to EEaddress. (Low Ack for each byte, expected.)
 *  @li Send Stop condx.
 *
 *  @note Expected return status will be "HAL_BUSY" ??? I believe ???
 *  @note Some time in future status will be "HAL_OK" once above 4 step process is complete.
 *  @note IMPORTANT - Write process is not actually complete upon "HAL_OK" until aftertWR time.
 *  				  tWR is specified as a worst case of 10ms to complete full 64 byte or less
 *  				  write.  We must either:
 *  				  1) Setup a timer to prevent future writes until tWR is elapsed, or
 *  				  2) Implement "ACKNOWLEDGE POLLING" as mentioned in the data sheet
 *						 ("ACKNOWLEDGE POLLING" method will allow faster response.)
 * </pre>
 *
 * @param hi2c 		 - pointer to I2C_HandleTypeDef, (info struct for this I2C transaction in process).
 * @param DevAddress - I2C device address to load into *hi2c. (normally only use 8 bits or less)
 * @param EEaddress  - EE memory address to load into *hi2c. (0-0x7FFF, 32K 24C256K bit part)
 * @param pByteBuffer - Pointer to multi-byte transmission buffer, external to this function.
 * @param bufferLength - Number of bytes to be sent, 1 to 64 presently.
 *
 * @returns retVal	  - One of: {HAL_OK, HAL_ERROR, HAL_BUSY, HAL_TIMEOUT}
 *
 */
HAL_StatusTypeDef sEEPromBytesWrite(I2C_HandleTypeDef *hi2c,  enumAT24C_7BitAddr addr7Bit, uint16_t EEaddress, uint8_t *pByteBuffer, uint16_t bufferLength) {

	HAL_StatusTypeDef I2CWriteStatus;

	// The HAL device addr is u16, (7 bit addr shifted 1 bit left).
	uint16_t HAL_DevAddr = ((uint16_t)addr7Bit << 1);

	while (hi2c->State != HAL_I2C_STATE_READY)
	{
		// While transfer count from previous is not zero we are still sending bytes..
		// asm("nop");

		STM32vldisc_LEDToggle(LED3);
	}
	// Non blocking (interrupt based) call to write multiple bytes.
	// Note: The ATMEL 24C256 / 24C128 has 64 byte pages and the max that can be
	//       written on any given call to this function is one full page, 64 bytes, worth.
	//       after that there is a max 10 ms delay (tWR) before we can talk to the eeprom again.

	// So if we use a starting EEaddress that is not a multiple of 64 bytes, then any part of the
	// buffer to be sent that exceeds the next multiple of 64 bytes will fold back to continue
	// at the previous multiple of 64 bytes's starting address. << VERIFY THIS IS TRUE >>

	I2CWriteStatus = HAL_I2C_Mem_Write_IT(hi2c, HAL_DevAddr, EEaddress, (uint16_t)2, pByteBuffer, bufferLength);

	return I2CWriteStatus;
}

/**
 * @brief Read one byte serial eeprom's current address.
 * <pre>
 *	This function does not require a parameter specifying EEaddress.
 *
 *	The internally determined current address, depends on
 *	eeprom initialization/reset (will set it to 0) or previous
 *	eeprom activity where internally determined current address
 *	will be last previously accessed read or write address + 1.
 *
 *	If last previously accessed read or write address was at maximum
 *	for the part then current address will return to 0.
 *
 *	The total process is expected to run as follows:
 *  @li Send Start condx. Send one byte Device address, with R/W* bit as R.
 *  @li Read One I2C bytes at internally determined address ( Respond with Ack* bit high.)
 *  @li Send Stop condx.
 *
 *  @note Expected return status will be "HAL_BUSY" ??? I believe ???
 *  @note Some time in future status will be "HAL_OK" once 3 step process is complete.
 *  @note Read process is complete upon "HAL_OK".
 *
 * </pre>
 *
 * @param hi2c 		 - pointer to I2C_HandleTypeDef, (info struct for this I2C transaction in process).
 * @param DevAddress - I2C device address to load into *hi2c. (normally only use 8 bits or less)
 * @param pByteRcvd  - Pointer to  single byte receive buffer, external to this function.
 *
 * @returns retVal	  - HAL_StatusTypeDef, One of: {HAL_OK, HAL_ERROR, HAL_BUSY, HAL_TIMEOUT}
 *
 */
HAL_StatusTypeDef sEEPromCurrentAddrReadByte(I2C_HandleTypeDef *hi2c, enumAT24C_7BitAddr addr7Bit, uint8_t *pByteRcvd)
{
	HAL_StatusTypeDef I2CWriteStatus;
	// The HAL device addr is u16, (7 bit addr shifted 1 bit left).
	uint16_t HAL_DevAddr = ((uint16_t)addr7Bit << 1);


	// This function can not use HAL_I2C_Mem_Read_IT(..) since we do not have an EEaddress.

	// We just do a quick start, Dev Address, and Read a Byte, this is blocking but will be quick:
	// ### Check if HAL has non-blocking version ??? ###
	I2CWriteStatus = HAL_I2C_Master_Receive(hi2c, HAL_DevAddr, pByteRcvd, (uint16_t) SIZE_OF_ONE_BYTE, (uint32_t)TIMEOUT_100MS);

	return I2CWriteStatus;
}

/**
 * @brief Read multiple bytes starting at serial eeprom's current address.
 * <pre>
 *	This function does not have a parameter specifying EEaddress.
 *
 *	The internally determined current address, depends on
 *	eeprom initialization/reset (will set it to 0) or previous
 *	eeprom activity where internally determined current address
 *	will be last previously accessed read or write address + 1.
 *
 *	If last previously accessed read or write address was at maximum
 *	for the part then current address will revert to to 0.
 *
 *	The total process is expected to run as follows:
 *  @li Send Start condx. Send one byte Device address, with R/W* bit as R.
 *  @li Read Multiple I2C bytes starting at internally determined address ( Respond with Ack* bit high.)
 *  @li Send Stop condx.
 * </pre>
 *
 * @param hi2c 		 		- pointer to I2C_HandleTypeDef, (info struct for this I2C transaction in process).
 * @param DevAddress 		- I2C device address to load into *hi2c. (normally only use 8 bits or less)
 * @param pBytesRcvd  		- Pointer to  single byte receive buffer, external to this function.
 * @param expectedByteCount - Bytes expected to be read.
 *
 * @returns retVal	  - HAL_StatusTypeDef, One of: {HAL_OK, HAL_ERROR, HAL_BUSY, HAL_TIMEOUT}
 *
 * ####### Could ensure expectedByteCount will never be  > sizeof buffer for pBytesRcvd
 *
 */
HAL_StatusTypeDef sEEPromCurrentAddrReadBytes(I2C_HandleTypeDef *hi2c, enumAT24C_7BitAddr addr7Bit, uint8_t *pBytesRcvd, uint16_t expectedByteCount)
{
	HAL_StatusTypeDef I2CWriteStatus;
	// The HAL device addr is u16, (7 bit addr shifted 1 bit left).
	uint16_t HAL_DevAddr = ((uint16_t)addr7Bit << 1);

	// This function can not use HAL_I2C_Mem_Read_IT(..) since we do not have an EEaddress.
	// ### Check if HAL has non-blocking version of HAL_I2C_Master_Receive ??? ###
	I2CWriteStatus = HAL_I2C_Master_Receive(hi2c, HAL_DevAddr, pBytesRcvd, expectedByteCount, (uint32_t)TIMEOUT_100MS);

	return I2CWriteStatus;
}

/**
 * @brief Read single byte, max of 64 (page size) from serial EEprom.
 * <pre>
 *	This function reads a single byte in particular eeprom device
 *	specified by DevAddress, at selected EEaddress inside the device.
 *
 *	This function is "non blocking", so it just starts this process running, by
 *	passing in for the I2C_Handle structure the desired communications information,
 *	and then calling the HAL initiation function: HAL_I2C_Mem_Read_IT(..),
 *	which updates the I2C_Handle structure for the duration this process will run.
 *
 *	The total process is expected to run as follows:
 *  @li Send Start condx. Send one byte Device address, with R/W* bit as W*.
 *  @li Send Two I2C bytes for two byte EEaddress. (Low Ack bit returned after each byte.)
 *  @li Send Second Start condx. Send same one byte Device address, with R/W* bit as R.
 *  @li Recieve one I2C byte of read data at EEaddress.
 *  @li Send High Ack after single byte to be read.
 *  @li Send Stop condx.
 *
 *  @note Expected return status will be "HAL_BUSY"
 *  @note Some time in future status will be "HAL_OK" once receive process is complete.
 *
 * </pre>
 *
 * @param hi2c 		 - pointer to I2C_HandleTypeDef, (info struct for this I2C transaction in process).
 * @param DevAddress - I2C device address to load into *hi2c. (normally only use 8 bits or less)
 * @param EEaddress  - EE memory address to load into *hi2c. (0-0x7FFF, 32K 24C256K bit part)
 * @param pByteBuffer - Pointer to  single byte rx buffer, external to this function.
 *
 * @returns retVal	  - One of: {HAL_OK, HAL_ERROR, HAL_BUSY, HAL_TIMEOUT}
 *
 */

HAL_StatusTypeDef sEEPromRandomAddrByteRead(I2C_HandleTypeDef *hi2c, enumAT24C_7BitAddr addr7Bit, uint16_t EEaddress, uint8_t *pByteBuffer)
{
	HAL_StatusTypeDef I2CWriteStatus;
	// The HAL device addr is u16, (7 bit addr shifted 1 bit left).
	uint16_t HAL_DevAddr = ((uint16_t)addr7Bit << 1);

	while (hi2c->State != HAL_I2C_STATE_READY)
	{
		// While transfer count from previous is not zero we are still doing bytes
		// from previous EERead or Write...
		//asm("nop");
		STM32vldisc_LEDToggle(LED3);
	}
	// Non blocking (interrupt based) call to read some bytes.
	// EEaddress is supplied as a parameter - so would use "HAL_I2C_Mem_Read_IT(..)"
	I2CWriteStatus = HAL_I2C_Mem_Read_IT(hi2c, HAL_DevAddr, EEaddress, (uint16_t)2, pByteBuffer, (uint16_t) 1);

	return I2CWriteStatus;
}


/**
 * @brief Read multiple bytes, max of 64 (page size) from serial EEprom.
 * <pre>
 *	This function reads multiple bytes to a particular eeprom device
 *	specified by DevAddress, Starting at memory at location EEaddress.
 *
 *	When less than 64 bytes to be read,
 *	"last 6 bits of EEaddress" + "bufferlength"  must be <= 64.
 *	otherwise, upper bytes of data read will come from areas before
 *	the starting EEaddress when last 6 bytes of EEaddress increment
 *	beyond page boundry.
 *
 *	This function is "non blocking", so it just starts this process running, by
 *	passing in for the I2C_Handle structure the desired communications information,
 *	and then calling the HAL initiation function: HAL_I2C_Mem_Read_IT(..),
 *	which updates the I2C_Handle structure for the duration this process will run.
 *
 *	The total process is expected to run as follows:
 *  @li Send Start condx. Send one byte Device address, with R/W* bit as W*.
 *  @li Send Two I2C bytes for two byte EEaddress. (Low Ack bit returned after each byte.)
 *  @li Send Second Start condx. Send same one byte Device address, with R/W* bit as R.
 *  @li Recieve 1 to 64 I2C bytes of read data starting at EEaddress. ( Send Low Ack to start next byte.)
 *  @li Send High Ack after final byte to be read.
 *  @li Send Stop condx.
 *
 *  @note Expected return status will be "HAL_BUSY"
 *  @note Some time in future status will be "HAL_OK" once receive process is complete.
 * </pre>
 *
 * @param hi2c 		 - pointer to I2C_HandleTypeDef, (info struct for this I2C transaction in process).
 * @param DevAddress - I2C device address to load into *hi2c. (normally only use 8 bits or less)
 * @param EEaddress  - EE memory address to load into *hi2c. (0-0x7FFF, 32K 24C256K bit part)
 * @param pByteBuffer - Pointer to multi-byte rx buffer, external to this function.
 * @param bufferLength - Number of bytes to be sent, 1 to 64 presently.
 *
 * @returns retVal	  - One of: {HAL_OK, HAL_ERROR, HAL_BUSY, HAL_TIMEOUT}
 *
 * ####### Could ensure expectedByteCount will never be  > sizeof buffer for pBytesRcvd
 *
 */

HAL_StatusTypeDef sEEPromRandomAddrReadBytes(I2C_HandleTypeDef *hi2c, enumAT24C_7BitAddr addr7Bit, uint16_t EEaddress, uint8_t *pByteBuffer, uint8_t bufferLength)
{
	HAL_StatusTypeDef I2CWriteStatus;
	// The HAL device addr is u16, (7 bit addr shifted 1 bit left).
	uint16_t HAL_DevAddr = ((uint16_t)addr7Bit << 1);

	while (hi2c->State != HAL_I2C_STATE_READY)
	{
		// While transfer count from previous is not zero we are still doing bytes
		// from previous EERead or Write...
		//asm("nop");
		STM32vldisc_LEDToggle(LED3);
	}
	// Non blocking (interrupt based) call to read some bytes.
	// EEaddress is supplied as a parameter - so would use "HAL_I2C_Mem_Read_IT(..)"
	I2CWriteStatus = HAL_I2C_Mem_Read_IT(hi2c, HAL_DevAddr, EEaddress, (uint16_t)2, pByteBuffer, (uint16_t) bufferLength);

	return I2CWriteStatus;
}


/**
 * @brief Reset the I2C serial eeprom by this I2C transmission.
 * <pre>
 *   Ref: AT24C128/256 data sheets - 0670F–SEEPR–2/02
 *
 *	 MEMORY RESET: After an interruption in protocol, power loss or system reset, any 2-wire
 *	 part can be reset by following these steps: (a) Clock up to 9 cycles, (b) look for SDA high in
 *	 each cycle while SCL is high and then (c) create a start condition as SDA is high.
 *
 *	The total process is expected to run as follows:
 *
 *  @li 1)
 *  @li 2)
 *  @li 3)
 *  @li 4)
 *
 *  @note Expected return status will be #########
 *  @note Parameters for this function are TBD ##########
 * </pre>
 *
 * @param hi2c 		 - pointer to I2C_HandleTypeDef, (info struct for this I2C transaction in process).
 * @param DevAddress - I2C device address to load into *hi2c. (normally only use 8 bits or less)
 *
 * @returns retVal	  - One of: {HAL_OK, HAL_ERROR, HAL_BUSY, HAL_TIMEOUT}
 *
 * ####### Could ensure expectedByteCount will never be  > sizeof buffer for pBytesRcvd
 *
 */
HAL_StatusTypeDef  sEE_I2C_PartReset(I2C_HandleTypeDef *hi2c)
{
	HAL_StatusTypeDef I2CWriteStatus = HAL_OK; //#############

	/// Not implemented yet !

	return I2CWriteStatus;
}
