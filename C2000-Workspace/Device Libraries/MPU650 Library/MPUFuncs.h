/*
 * MPUFuncs.h
 *
 *  Created on: Nov 18, 2013
 *      Author: Alex
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "MPU6050_Constants.h"
#include "I2CFuncs.h"

#ifndef MPUFUNCS_H_
#define MPUFUNCS_H_

void set_MPU_gyro_offsets(int16 x, int16 y, int16 z);
void set_MPU_accel_offsets(int16 x, int16 y, int16 z);

int get_MPU6050_status();
unsigned char get_MPU_internal_status();

// DMP-related functions:
int write_DMP_configuration(const unsigned char *data, Uint16 dataSize);
int write_MPU_memory_block(const unsigned char *data, Uint16 dataSize, Uint16 bank, Uint16 address, bool verify, bool useProgMem);
void set_MPU_memory_bank(Uint16 bank, int prefetchEnabled, int userBank);
void set_MPU_start_address(Uint16 address);

int get_FIFO_count();
float32 get_MPU6050_temperature();


#endif /* MPUFUNCS_H_ */
