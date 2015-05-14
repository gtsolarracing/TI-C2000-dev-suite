/*
 * I2CFuncs.h
 *
 *  Created on: Nov 11, 2013
 *      Author: Alex
 */
#include "PeripheralHeaderIncludes.h"
#include <stdio.h>

#ifndef I2CFUNCS_H_
#define I2CFUNCS_H_

// Function prototypes:
int i2c_write_bit(Uint16 slave_address, Uint16 register_address, short bitNum, short bit);
int i2c_write_bits(Uint16 slave_address, Uint16 register_address, Uint16 bitStart, Uint16 length, Uint16 data);
int i2c_write_byte(Uint16 Slave_address, Uint16 Start_address, Uint16 databyte);
int i2c_write(Uint16 Slave_address, Uint16 Start_address, Uint16 no_databytes, Uint16 databytes[]);

int i2c_read_bit(Uint16 slave_address, Uint16 register_address, short bitNum);
void i2c_read(Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Read_Array[]);
void I2CA_Init(void);

short getBitFromByte(Uint16 byte, short bitNum);
void printBytes(Uint16 byte, Uint16 byte2);

#endif /* I2CFUNCS_H_ */
