/**
 * @file I2CFuncs.c
 *
 * @date Nov 11, 2013
 */
#include "I2CFuncs.h"


/**
 * @brief Use this to write multiple BITS sequentially on a register.
 @note This function CANNOT write more than one byte!
 @details bitStart is the nth bit from the leftmost bit of the register, bit 0.
  length is the number of bits to write, must be <= 8. Measured from the rightmost bit of the data byte.
  data is the actual data to write. Only 1 byte can be written with this method. If data is a small number, it will
  be left-padded with zeros, up to (length), on the register(s).
*/
int i2c_write_bits(Uint16 slave_address, Uint16 register_address, Uint16 bitStart, Uint16 length, Uint16 data) {
	Uint16 orig[3] = { 0, 0, 0 };
	i2c_read(slave_address, register_address, 1, orig);
	Uint16 tempStore = orig[0];

	//puts("Original:");
	//printBytes(orig[0], 0);
	//char string[32];
	//sprintf(string, "Start:%d Length:%d", bitStart, length);
	//puts(string);
	//printBytes(data, 0);

	short bytebits[8];
	short i;
	for (i = 0; i <= 7; i++) {
		bytebits[i] = getBitFromByte(tempStore, 7 - i);
	}

	data <<= 8 - length; // Cut zeros in front of data.
	for (i = 0; i < length; i++)
		bytebits[7 - bitStart + i] = getBitFromByte(data, 7 - i);

	Uint16 finalByte = 128 * bytebits[0] + 64 * bytebits[1] + 32 * bytebits[2]
			+ 16 * bytebits[3] + 8 * bytebits[4] + 4 * bytebits[5]
			+ 2 * bytebits[6] + bytebits[7];

	//puts("Final");
	//printBytes(finalByte, 0);

	// Save modified bit:
	return i2c_write_byte(slave_address, register_address, finalByte);
}

/// @brief Use this to write a single bit on a register, instead of an entire byte:
int i2c_write_bit(Uint16 slave_address, Uint16 register_address, short bitNum, short bit) {
	return i2c_write_bits(slave_address, register_address, bitNum, 1, (Uint16)bit);
}

int i2c_write_byte(Uint16 Slave_address, Uint16 Start_address, Uint16 databyte) {
	Uint16 b[1];
	b[0] = databyte;
	return i2c_write(Slave_address, Start_address, 1, b);
}

/**
@brief I2C_Write
@details Return Type: void
 Arguments: Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Write_Array[]
 Description: I2C Write Driver. Pass Slave Address, Write location, No of databytes and the array with data.'
 Returns 0 if unsuccessful, 1 if successful.
*/
int i2c_write(Uint16 Slave_address, Uint16 Start_address, Uint16 no_databytes, Uint16 databytes[])
{
	I2caRegs.I2CSAR = Slave_address;
	//puts("write");
	while(I2caRegs.I2CMDR.bit.STP != 0);

	// Start bit, write mode, Higher 16 address bits, Master, Repeat mode.
	//I2caRegs.I2CMDR.bit.TRX = TRANSMIT_MESSAGE; Where is TRANSMIT_MESSAGE defined?????? ERROR!

	//######## Only for EEPROM #########################//
	/*if(Slave_address == EEPROM_24LC256_ADDRESS)
	{
	I2caRegs.I2CDXR = (Start_address)>>8;
	}*/
	//##################################################//

	while (I2caRegs.I2CSTR.bit.BB != 0) {
		//puts ("I2C bus is busy, cannot proceed. Could be caused by noise.");
	}

	I2caRegs.I2CMDR.all = 0x26A0; // 0010 0110 1010 0000
	// Sets NACKMOD to 0: The I2C module sends an ACK bit during each acknowledge cycle until the internal data counter counts down to 0. At that point, the I2C module sends a NACK bit to the transmitter. To have a NACK bit sent earlier, you must set the NACKMOD bit.
	// Sets FREE to 0: For breakpoints: if SCL is low when the breakpoint occurs, the I2C module stops immediately and keeps driving SCL low, whether the I2C module is the transmitter or the receiver. If SCL is high, the I2C module waits until SCL becomes low and then stops.
	// Sets STT to 1: A START condition is generated on the I2C bus!
	// Sets Reserved Bit 12 to 0, does nothing!
	// Sets STP to 0: STP is cleared after the STOP condition is generated.
	// Sets MST to 1: The I2C module is a master and generates the serial clock on the SCL pin.
	// Sets TRX to 1: Sets the I2C module to transmitter mode, and transmits data on the SDA pin.
	// Sets XA to 0: 7-bit addressing mode
	// Sets RM to 1: Repeat mode is turned ON. A data byte is transmitted each time the I2CDXR register is written to (or until the transmit FIFO is empty when in FIFO mode) until the STP bit is manually set. The value of I2CCNT is ignored. The ARDY bit/interrupt can be used to determine when the I2CDXR (or FIFO) is ready for more data, or when the data has all been sent and the CPU is allowed to write to the STP bit.
	// Sets DLB to 0: Digital loopback mode is DISABLED
	// Sets IRS to 1: The I2C module is ENABLED!
		// Sets STB to 0: I2C module is NOT in START byte mode.
	// Sets FDF to 0: free data format mode DISABLED, transfers are using standard addressing format selected by XA bit.
	// Sets BC to 000: 8 bits per data byte, in the next data byte that is to be transmitted or received by the I2C module

	while(I2caRegs.I2CSTR.bit.ARBL != 0) {
		//puts("Arbitration Problem/Noise on line!"); // The I2C believes there is another master attempting to transmit at this time!
	}

	//(Lower 16) address bits
	while(I2caRegs.I2CSTR.bit.ARDY != 1) {
		//puts("I2C registers are not ready to be accessed." );
	}

	if (I2caRegs.I2CSTR.bit.ARDY == 1)
	{
		I2caRegs.I2CDXR = Start_address; // Send start address.
	}
	while (I2caRegs.I2CSTR.bit.NACK == 1) {
		//puts("Sent address was not acknowledged! NACK (no acknowledgment bit received)! Aborting write...");

		I2caRegs.I2CMDR.all = 0x0EA0; // 0000 1110 1010 0000
					// Send stop condition.
					// Set master mode, transmitter mode
					// Enable repeat mode, enable i2c

					while(I2caRegs.I2CSTR.bit.SCD != 1) {
						//puts("Stop condition not detected yet...");
					}
					I2caRegs.I2CSTR.bit.SCD = 1; // Clear stop condition

					while (I2caRegs.I2CMDR.bit.MST != 0); // Wait for stop condition to be accepted.

	//				I2caRegs.I2CMDR.all = 0; // Reset I2C.
	//	I2CA_Init();
		puts("Write error 0, retrying!");


		i2c_write(Slave_address, Start_address, no_databytes, databytes);
		return 0;
	}
	//puts("Send address was acknowledged.");

	Uint16 i = 0;
	for(i = 0; i < no_databytes; i++) {
		// Transmit data byte:
		while(I2caRegs.I2CSTR.bit.ARDY != 1 || I2caRegs.I2CSTR.bit.XRDY != 1) {
			//puts("I2C transmit registers are not ready.");
		}
		// DSP28x_usDelay(5000);
		if (I2caRegs.I2CSTR.bit.ARDY == 1)
		{
			I2caRegs.I2CDXR = databytes[i]; // Transmit data by putting it here.
		}

		if (I2caRegs.I2CSTR.bit.NACK == 1)
		{
			//puts("NACK bit received in transmit! Aborting transmit...");
			I2caRegs.I2CSTR.bit.NACK = 1; // Reset NACK bit.
			I2caRegs.I2CMDR.all = 0; // Reset I2C.
			puts("Write Error 1");
			return 0;
		}
		// Done transmitting.
	}

	I2caRegs.I2CMDR.all = 0x0EA0; // 0000 1110 1010 0000
	// Send stop condition.
	// Set master mode, transmitter mode
	// Enable repeat mode, enable i2c

	while(I2caRegs.I2CSTR.bit.SCD != 1) {
		//puts("Stop condition not detected yet...");
	}
	I2caRegs.I2CSTR.bit.SCD = 1; // Clear stop condition

	while (I2caRegs.I2CMDR.bit.MST != 0); // Wait for stop condition to be accepted.
	return 1;
}

// Reads a single bit from a 8-bit register, where bitNum=0=LSB=RIGHTMOST bit.
int i2c_read_bit(Uint16 slave_address, Uint16 register_address, short bitNum) {
	Uint16 b[1] = { 0 };
	i2c_read(slave_address, register_address, 1, b);
	return getBitFromByte(b[0], bitNum);
}

/**
@brief I2C_Read
@details Function Name: void I2C_Read
 Return Type: void
 Arguments: Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Read_Array[]
 Description: I2C Read Driver. Pass Slave Address, Write location, No of databytes, Array where received will be copied.
*/
void i2c_read(Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Read_Array[])
{
	I2caRegs.I2CSAR = Slave_address; // This stores the next slave address that
									 // will be transmitted to by the I2C module.
	I2caRegs.I2CCNT = No_of_databytes; // When operating in non repeat mode, this
									   // value will determine how many bytes to
									   // receive/send, before a STOP signal is
									   // sent.
	Uint16 *Temp_Pointer;
	Temp_Pointer = Read_Array;

	// Wait until the STP bit is cleared from any previous master communication.
	// Clearing of this bit by the module is delayed until after the SCD bit is
	// set. If this bit is not checked prior to initiating a new message, the
	// I2C could get confused.
	while (I2caRegs.I2CMDR.bit.STP != 0) {//this should be 0
		//puts("Waiting to clear stop bit...");
	}

	while (I2caRegs.I2CSTR.bit.BB != 0) {
		//puts ("I2C bus is busy, cannot proceed. Could be caused by noise.");
	}

	// Start bit, write mode, Higher 16 address bits, Master, Non Repeat mode.
	I2caRegs.I2CMDR.bit.TRX = 1; // This sets the I2C module to transmitter mode.
								 // Now, it transmits data on the SDA pin.

	//######## Only for EEPROM #########################//
	/*if(Slave_address == EEPROM_24LC256_ADDRESS)
	{
		// Start_address is a 16 bit unsigned integer.
		I2caRegs.I2CDXR = (Start_address)>>8; // This shifts the address over by 8 bits, putting 8 zeroes before the first bit of Start_address.
	}*/
	//##################################################//

	I2caRegs.I2CMDR.all = 0x26A0; // This sets the register to 0010011010100 000

	// Sets NACKMOD to 0: The I2C module sends an ACK bit during each acknowledge cycle until the internal data counter counts down to 0. At that point, the I2C module sends a NACK bit to the transmitter. To have a NACK bit sent earlier, you must set the NACKMOD bit.
	// Sets FREE to 0: For breakpoints: if SCL is low when the breakpoint occurs, the I2C module stops immediately and keeps driving SCL low, whether the I2C module is the transmitter or the receiver. If SCL is high, the I2C module waits until SCL becomes low and then stops.
	// Sets STT to 1: A START condition is generated on the I2C bus!
	// Sets Reserved Bit 12 to 0, does nothing!
	// Sets STP to 0: STP is cleared after the STOP condition is generated.
	// Sets MST to 1: The I2C module is a master and generates the serial clock on the SCL pin.
	// Sets TRX to 1: Sets the I2C module to transmitter mode, and transmits data on the SDA pin.
	// Sets XA to 0: 7-bit addressing mode
	// Sets RM to 1: Repeat mode is turned ON. A data byte is transmitted each time the I2CDXR register is written to (or until the transmit FIFO is empty when in FIFO mode) until the STP bit is manually set. The value of I2CCNT is ignored. The ARDY bit/interrupt can be used to determine when the I2CDXR (or FIFO) is ready for more data, or when the data has all been sent and the CPU is allowed to write to the STP bit.
	// Sets DLB to 0: Digital loopback mode is DISABLED
	// Sets IRS to 1: The I2C module is ENABLED!
		// Sets STB to 0: I2C module is NOT in START byte mode.
	// Sets FDF to 0: free data format mode DISABLED, transfers are using standard addressing format selected by XA bit.
	// Sets BC to 000: 8 bits per data byte, in the next data byte that is to be transmitted or received by the I2C module

	Uint16 ARDY_wait = 0;
	while(I2caRegs.I2CSTR.bit.ARDY != 1) { // When ARDY is 0, the I2C module registers are NOT ready to be accessed. 1 means they are.
		ARDY_wait++;
		if (ARDY_wait > 65000) {
			// Timeout.
			I2caRegs.I2CSTR.bit.ARDY = 1; // Clear ARDY.
			// Manually send a stop signal.
			I2caRegs.I2CMDR.bit.STP = 1;
			I2caRegs.I2CMDR.all = 0; // Reset module.
			I2CA_Init(); // Re-initialize module.
			// Wait a bit.
			Uint16 counter = 0;
			while(counter < 30000) counter++; // Wait a bit
			//puts("I2C registers are not ready to be accessed." );
			puts("Read Error 0!");
			//i2c_read(Slave_address, Start_address, No_of_databytes, Read_Array);
			return;
		}
	}

	if (I2caRegs.I2CSTR.bit.ARDY == 1) // Execute only when the registers are ready
	{
		// Say what data we want to transmit to the slave to the I2CDXR register.
		// We want to transmit the start address in the memory where we want to
		// read from, in the slave’s memory registers.
		I2caRegs.I2CDXR = Start_address; // (Lower 16) address bits
	}

	// Wait for I2C registers to be ready...
	while(I2caRegs.I2CSTR.bit.ARDY != 1) {
		//puts("I2C registers are not ready to be accessed." );
	}
	//if (I2caRegs.I2CSTR.bit.NACK == 1) {
	//	//puts("Sent address was not acknowledged! NACK (no acknowledgment bit received)! Aborting read...");
	//	I2caRegs.I2CMDR.all = 0; // Reset I2C.
	//	puts("Read Error 1");
	//	return;
	//}
	//puts("Send address was acknowledged.");

	// Manually send a stop signal, because we need to change to non repeat mode.
	I2caRegs.I2CMDR.bit.STP = 1;

	while(I2caRegs.I2CMDR.bit.STP != 0); // Wait till the STOP is detected...

	I2caRegs.I2CMDR.all = 0x2C20; // Sets I2CMDR to 0010110000100 000, so this is the same as the above set except:
	// Sets STP to 1: A STOP condition is automatically generated when the internal data counter of the I2C module counts down to 0 (all data is read/transmitted).
	// Sets TRX to 0: The I2C module is now in Receiver mode!
	// Sets RM to 0, so the I2C module is in Nonrepeat mode; the value in the data count register I2CCNT determines how many bytes are received now.
	// Also sends another start condition on the lines: this is the re-start that indicates a read operation!

	int i = 0;
	while( i < No_of_databytes ) // i is initially 0.
	{
		while(I2caRegs.I2CSTR.bit.RRDY != 1) { // This is 1 when a received FIFO interrupt condition has occurred - for FIFO only! *For Non-FIFO, check if I2CSTR.bit.RRDY
			//puts("Read in progress, no FIFO data received yet...");
		}

		*Temp_Pointer++ = I2caRegs.I2CDRR; // Save the read data into the read array.
		i++;

		//if (I2caRegs.I2CSTR.bit.NACK == 1) {
		//	//puts("Read failed, NACK (no acknowledgment bit received)! Aborting read...");
		//	I2caRegs.I2CMDR.all = 0; // Reset I2C.
		//	puts("Read Error 2");
		//	return;
		//}
	}
	while (I2caRegs.I2CMDR.bit.MST != 0); // Wait for stop condition to be accepted.
}

/// @brief This function initializes I2C on the F2806 C2000 microcontroller.
void I2CA_Init(void)
{
   // Initialize I2C
   I2caRegs.I2CPSC.all = 89;		    // The I2C Clock speed is CLOCK/(I2CPSC+1). Set 222 KHz
   // The current peripheral clock speed is 80MHz/4
   I2caRegs.I2CCLKL = 10;			// NOTE: must be non zero, the amount of time the SCL clock pin is low
   I2caRegs.I2CCLKH = 5;			// NOTE: must be non zero, the amount of time the SCL is high
   I2caRegs.I2CIER.all = 0x24;		// Enable SCD & ARDY interrupts
   // 0000 0000 0010 0100
   // OVERRIDE: DISABLE SCD + ARDY INTERRUPTS
   I2caRegs.I2CIER.all = 0;

   //set MST to true
   I2caRegs.I2CMDR.all = 0x0420; // 0000 0100 0010 0000
   // Sets I2C module to master, receiver mode.
   // Takes I2C out of reset

   //I2caRegs.I2CFFTX.all = 0x6000; // 0110 0000 0000 0000
   // Enable FIFO mode and TXFIFO
   // OVERRIDE: DISABLE FIFO
   //I2caRegs.I2CFFTX.all = 0;

   //I2caRegs.I2CFFRX.all = 0x2040;// 0010 0000 0100 0000
   // Enable RXFIFO, clear RXFFINT.
   // OVERRIDE: DISABLE FIFO
   //I2caRegs.I2CFFRX.all = 0;

  // I2caRegs.I2CPSC.bit.IPSC = 125;
}

/// @brief Gets a certain bit from a byte, where 0=RIGHTMOST bit (LSB). Also works for 16-bit words.
short getBitFromByte(Uint16 byte, short bitNum) {
	Uint16 one = 1;
	int bitval = byte & (one << bitNum);
	if (bitval != 0)
		return 1;
	return 0;
}

/// @brief Prints out two bytes to std output. You can use this for testing.
void printBytes(Uint16 byte, Uint16 byte2) {
	char string[17];
	sprintf(string, "%d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d", (byte & 0x80 ? 1 : 0),
			(byte & 0x40 ? 1 : 0), (byte & 0x20 ? 1 : 0), (byte & 0x10 ? 1 : 0),
			(byte & 0x08 ? 1 : 0), (byte & 0x04 ? 1 : 0), (byte & 0x02 ? 1 : 0),
			(byte & 0x01 ? 1 : 0),

			(byte2 & 0x80 ? 1 : 0), (byte2 & 0x40 ? 1 : 0),
			(byte2 & 0x20 ? 1 : 0), (byte2 & 0x10 ? 1 : 0),
			(byte2 & 0x08 ? 1 : 0), (byte2 & 0x04 ? 1 : 0),
			(byte2 & 0x02 ? 1 : 0), (byte2 & 0x01 ? 1 : 0));
	puts(string);
}

