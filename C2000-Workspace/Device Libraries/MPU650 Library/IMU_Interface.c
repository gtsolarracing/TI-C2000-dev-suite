/**
   @file IMU_Interface.c
   @brief IMU_Interface Functions: This is how you can communicate with the Inertial Measurement Unit (IMU) subsystem. Currently working with MPU-6050. (VERSION 1.0)
   @details
   Georgia Tech Solar Jackets
     Electrical Team
       HMI/Telemetery Division
   These functions can be used to communicated over the I2C protocol with the IMU, and return averaged IMU data.
   @note I2C communication requires the SCL and SDA pins be connected to the IMU.

   @author Alex Popescu
   @author Andrew Tso
   @author Sanmeshkumar Udhayakumar
   @author Victoria Tuck

   @date 1/13/2014

   @note This code borrows heavily on the Arduino code of the I2C Devlib project, which can be found at:
   https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050
   and uses code found from:
   https://github.com/jrowberg/i2cdevlib/blob/master/Arduino/MPU6050/Examples/MPU6050_DMP6/MPU6050_DMP6.ino

   @note Special thanks to:
      Andrey Kurenkov
      Brian Kuo
*/

#include "IMU_Interface.h"
#include <math.h>

/**
@brief This function averages many data samples from the IMU, and returns the average.
@details This function returns the average of some number of IMU samples.
 Details on the return structure are in the header of this file.
 The samples parameter tells how many samples to be taken, and averaged together.
 Samples are taken at approximately ~75Hz, but probably can be configured to be faster.
@note For optimal operation, this function should be called as frequently as possible, since
 infrequent calling would mean dumping the FIFO buffer often!
*/
imu_read_data imu_subsystem_get_data_samples(int num_samples) {
	imu_read_data sum = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i;
	for (i = 0; i < num_samples; i++) {
		imu_read_data sample = imu_subsystem_current_data_read();
		sum.roll += sample.roll;
		sum.pitch += sample.pitch;
		sum.yaw += sample.yaw;
		sum.roll_ang_vel += sample.roll_ang_vel;
		sum.pitch_ang_vel += sample.pitch_ang_vel;
		sum.yaw_ang_vel += sample.yaw_ang_vel;
		sum.lin_acc += sample.lin_acc;
		sum.x_acc += sample.x_acc;
		sum.y_acc += sample.y_acc;
		sum.z_acc += sample.z_acc;
		sum.temp += sample.temp;
	}

	sum.roll /= (float)num_samples;
	sum.pitch /= (float)num_samples;
	sum.yaw /= (float)num_samples;
	sum.roll_ang_vel /= (float)num_samples;
	sum.pitch_ang_vel /= (float)num_samples;
	sum.yaw_ang_vel /= (float)num_samples;
	sum.lin_acc /= (float)num_samples;
	sum.x_acc /= (float)num_samples;
	sum.y_acc /= (float)num_samples;
	sum.z_acc /= (float)num_samples;
	sum.temp /= (float)num_samples;
	return sum;
}

/**
@brief This function reads one data sample from the IMU.
@details This function will read current rotation and acceleration data from the IMU, and return it.
 This function returns a struct of 32-bit floats named "imu_read_data".
 NOTE: Make sure to call imu_subsystem_setup first!
 Returns and errors are in the header of this file.
 */
imu_read_data imu_subsystem_current_data_read() {
	imu_read_data ret_data; // Create the return structure.

	//  Collect FIFO data from MPU6050
	while(1) {
		while(get_FIFO_count() < 42); // Wait till enough data has been collected.

		// Check for FIFO overflow:
		if ((get_MPU_internal_status() & 0x10) || get_FIFO_count() >= 1024) {
			//puts("FIFO overflow!");
			// Reset FIFO.
			i2c_write_bit(MPU6050_ADDRESS, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, true);
		} else {
			// == We can wait for a DMP data ready interrupt here ==
			// Or, we can just check for new data, which is what is currently implemented:

			// Read FIFO packet (packet size is default 42):
			Uint16 FIFO_data[42];
			i2c_read(MPU6050_ADDRESS, MPU6050_RA_FIFO_R_W, 42, FIFO_data);

			ret_data.read_status = 1; // Set to "data successfully read"

			// Get acceleration, minus gravity:
			float quaternion[4];
			DMP_get_quaternion(quaternion, FIFO_data);
			int16 accel[3];
			DMP_get_raw_accel(accel, FIFO_data);
			float gravity[3];
			DMP_get_gravity(gravity, quaternion);
			ret_data.x_acc = (float32)accel[0] / 4096.0f - gravity[0]; // NOTE: 8192/2 must be changed if the resolution is changed from +/- 4g
			ret_data.y_acc = (float32)accel[1] / 4096.0f - gravity[1];
			ret_data.z_acc = (float32)accel[2] / 4096.0f - gravity[2];

			// Calculate the magnitude of acceleration vector
			ret_data.lin_acc = sqrt(ret_data.x_acc*ret_data.x_acc + ret_data.y_acc*ret_data.y_acc + ret_data.z_acc*ret_data.z_acc);

			// Retrieve gyroscope information:
			int16 gyro[3];
			DMP_get_raw_gyro(gyro, FIFO_data);
			ret_data.roll_ang_vel = (float32)gyro[0] / 65.5f;
			ret_data.pitch_ang_vel = (float32)gyro[1] / 65.5f;
			ret_data.yaw_ang_vel = (float32)gyro[2] / 65.5f;

			// Get roll, pitch, and yaw:
			float ypr[3];
			DMP_get_yaw_pitch_roll(ypr, quaternion, gravity);
			ret_data.yaw = ypr[0] / 3.14159265f * 180.0f;
			ret_data.pitch = ypr[1] / 3.14159265f * 180.0f;
			ret_data.roll = ypr[2] / 3.14159265f * 180.0f;

			// Get temperature:
			ret_data.temp = get_MPU6050_temperature();

			return ret_data;; // Read operation was successful.
		}
	}
}

/**
@brief This function will set up the IMU subsystem, and return any errors that are encountered.
@warning If an error is reported here, FIX IT. DO NOT run the imu_system_data_read function.
       Most errors can be fixed by restarting the IMU, or re-trying setup.
@description Call this function before requesting data from the IMU.
 Returns are as follows:
   1 : Setup was OK
  -1 : ERROR: MPU is not responding; it may be disconnected! Try re-connecting.
  -2 : ERROR: MPU is connected and working, but the DMP has not initialized correctly!
*/
int imu_subsystem_setup() {
	I2CA_Init(); // Initialize I2C module

	// Try to connect with MPU6050:
	Uint16 tries = 600;
	Uint16 i = 0;
	for ( i = 0; i <= tries; i++ ) {
		if (get_MPU6050_status() == 1) break;
		puts("MPU6050 is not responding. Try restarting microcontroller.");
		if (i == tries) return -1; // Give up.
	}
	puts("MPU6050 connection successful.");

	// Initialize the Digital Motion Processor (DMP) on the MPU6050. This allows for more accurate
	// measurements of rotation.
	puts("Setting up DMP...");
	if( ! initialize_DMP() ) {
		puts("DMP setup unsuccessful!");
		return -2;
	}

	return 1; // Setup went OK.
}
