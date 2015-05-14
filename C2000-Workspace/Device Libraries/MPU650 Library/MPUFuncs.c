/**
 * @file MPUFuncs.c
 *
 *  @date Nov 18, 2013
 *  @author Alex Popescu
 */
#include "MPUFuncs.h"

/// @brief Sets gyro calibration offsets:
void set_MPU_gyro_offsets(int16 x, int16 y, int16 z) {
	Uint16 wr[1];
	wr[0] = x>>8;
	wr[1] = x;
	i2c_write(MPU6050_ADDRESS, MPU6050_RA_XG_OFFS_USRH, 2, wr);
	wr[0] = y>>8;
	wr[1] = y;
	i2c_write(MPU6050_ADDRESS, MPU6050_RA_YG_OFFS_USRH, 2, wr);
	wr[0] = z>>8;
	wr[1] = z;
	i2c_write(MPU6050_ADDRESS, MPU6050_RA_ZG_OFFS_USRH, 2, wr);
}

/// @brief Sets acceleration calibration offsets:
void set_MPU_accel_offsets(int16 x, int16 y, int16 z) {
	Uint16 wr[1];
	wr[0] = x>>8;
	wr[1] = x;
	i2c_write(MPU6050_ADDRESS, MPU6050_RA_XA_OFFS_H, 2, wr);
	wr[0] = y>>8;
	wr[1] = y;
	i2c_write(MPU6050_ADDRESS, MPU6050_RA_YA_OFFS_H, 2, wr);
	wr[0] = z>>8;
	wr[1] = z;
	i2c_write(MPU6050_ADDRESS, MPU6050_RA_ZA_OFFS_H, 2, wr);
}

/// @brief Select the memory bank register:
void set_MPU_memory_bank(Uint16 bank, int prefetchEnabled, int userBank) {
    bank &= 0x1F;
    if (userBank) bank |= 0x20;
    if (prefetchEnabled) bank |= 0x40;
    i2c_write_byte(MPU6050_ADDRESS, MPU6050_RA_BANK_SEL, bank);
}

/// @brief Set the MPU memory start address:
void set_MPU_start_address(Uint16 address) {
	i2c_write_byte(MPU6050_ADDRESS, MPU6050_RA_MEM_START_ADDR, address);
}

/// @brief Writes Digital Motion Processor configuration to IMU.
int write_DMP_configuration(const unsigned char *data, Uint16 dataSize) {
	Uint16 i, j;
	unsigned char *progBuffer = (unsigned char *)malloc(8); // Assume 8 byte blocks.

	for (i = 0; i < dataSize;) {
		Uint16 bank = data[i++];
		Uint16 offset = data[i++];
		Uint16 length = data[i++];

		if (length > 0) {
			if (sizeof(progBuffer) < length) progBuffer = (unsigned char *)realloc(progBuffer, length); // Realloc if necessary.
			for (j = 0; j < length; j++) progBuffer[j] = data[i + j];
			// Send this data to be written:
			if(write_MPU_memory_block(progBuffer, length, bank, offset, true, true) == false) {
				free(progBuffer); // Failed writing config.
				return false;
			}
			i += length;
		}
		else {
			// This is a special setting.
			Uint16 special = data[i++];
			if (special == 0x01) {
				// Enable DMP-related interrupts:
				i2c_write_byte(MPU6050_ADDRESS, MPU6050_RA_INT_ENABLE, 0x32);
			} else {
				puts("No special detected!");
				free(progBuffer);
				return false;
			}
		}
	}
	free(progBuffer);
	return true;
}

/// @brief Write memory block on MPU:
int write_MPU_memory_block(const unsigned char *data, Uint16 dataSize, Uint16 bank, Uint16 address, bool verify, bool useProgMem) {
	set_MPU_memory_bank(bank, false, false);
	set_MPU_start_address(address);
	Uint16 chunkSize;
	Uint16 progBuffer[MPU6050_DMP_MEMORY_CHUNK_SIZE];
	Uint16 verifyBuffer[MPU6050_DMP_MEMORY_CHUNK_SIZE];
	Uint16 i; // Chunk start byte
	Uint16 j; // Byte in chunk

	for (i = 0; i < dataSize;) {
		// Write each *chunk* of information, and verify it:
		// determine correct chunk size according to bank position and data size (in bytes)
		chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

		// make sure we don't go past the data size
		if (i + chunkSize > dataSize)
			chunkSize = dataSize - i;

		// make sure this chunk doesn't go past the bank boundary (256 bytes)
		if (chunkSize > 256 - address)
			chunkSize = 256 - address;

		// Save the chunk of data:
		for (j = 0; j < chunkSize; j++)
			progBuffer[j] = (Uint16)data[i + j];

		// Write the program to MPU6050 memory:
		i2c_write(MPU6050_ADDRESS, MPU6050_RA_MEM_R_W, chunkSize, progBuffer);

		// Verify data:
		if (verify) {
			set_MPU_memory_bank(bank, false, false);
			set_MPU_start_address(address);
			i2c_read(MPU6050_ADDRESS, MPU6050_RA_MEM_R_W, chunkSize, verifyBuffer);

			if (memcmp(progBuffer, verifyBuffer, chunkSize) != 0) {
				puts("DMP memory chunk NOT copied successfully! Aborting...");
				return false;
			}
		}

		// increase byte index by [chunkSize]
		i += chunkSize;

		// Address must always be < 256!
		address += chunkSize;
		if (address >=256) address -= 256;

		//puts_int("Copied byte %d", i);
		// if we aren't done, update bank (if necessary) and address
		if (i < dataSize) {
			if (address == 0)
				bank++;
			set_MPU_memory_bank(bank, false, false);
			set_MPU_start_address(address);
		}
	}
	return true;
}

/// @brief Gets the number of FIFO messages ready, currently:
int get_FIFO_count() {
	Uint16 buffer[2] = {0, 0};
	i2c_read(MPU6050_ADDRESS, MPU6050_RA_FIFO_COUNTH, 2, buffer);
	return ((buffer[0]) << 8) | buffer[1];
}

/// @brief Returns the current temperature sensor reading, in farenheight.
float32 get_MPU6050_temperature() {
	Uint16 buffer[2] = {0, 0};
	i2c_read(MPU6050_ADDRESS, MPU6050_RA_TEMP_OUT_H, 2, buffer);
	int16 rawtemp = (int16)((buffer[0] << 8) | buffer[1]);
	float celcius = (float)rawtemp / 340.0f + 36.53f;
	return celcius*1.8f + 32.0f; // Convert to Fahrenheit
}

/// @brief This function returns the status of the MPU-6050 IMU:
/// 0: Not connected/connection problems
/// 1: Connected & ready
int get_MPU6050_status() {
	// To check if the MPU6050 is working, attempt to read the byte
	// at memory register 0x75 (MPU6050_RA_WHO_AM_I) on the IMU. This should
	// give 0x68, if all is working correctly.

	Uint16 received_data[1];
	received_data[0] = 0x00;
	// We are now sending a read query to the MPU address, to the WHO_AM_I register (0x75), which should contain 0x68.
	i2c_read(MPU6050_ADDRESS, MPU6050_RA_WHO_AM_I, 1, received_data);
	//puts_int("Received from I2C: %x\n", received_data[0]);

	if (received_data[0] == 0x68) {
		return 1;
	}
	return 0;
}

/// @brief Returns the internal status of the MPU6050. Tells if FIFO is in overflow!
unsigned char get_MPU_internal_status() {
	Uint16 buffer[1] = {0};
	i2c_read(MPU6050_ADDRESS, MPU6050_RA_INT_STATUS, 1, buffer);
	return (unsigned char)buffer[0];
}



