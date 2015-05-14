/*
 * IMU_Interface.h
 *
 *  Created on: Jan 11, 2014
 *      Author: Alex
 */

#include "DMPFuncs.h"

#ifndef IMU_INTERFACE_H_
#define IMU_INTERFACE_H_

typedef struct imu_read_data {
	char read_status;
	//       1 : Read went OK; data is valid.
	//      -1 : MPU is not responding! Try disconnecting and restarting, then re-setting-up.
	float32 roll; // (degrees)
	float32 pitch; // (degrees)
	float32 yaw; // (degrees)
	float32 roll_ang_vel; // Roll angular velocity (degrees/sec)
	float32 pitch_ang_vel; // Pitch angular velocity (degrees/sec)
	float32 yaw_ang_vel; // Yaw angular velocity (degrees/sec)
	float32 lin_acc; // Linear acceleration magnitude (g, where 1g=9.81 m/s^2)
	float32 z_acc; // z acceleration (g)
	float32 x_acc; // x acceleration (g)
	float32 y_acc; // y acceleration (g)
	float32 temp; // Device temperature, in degrees farenheight.
} imu_read_data;

imu_read_data imu_subsystem_get_data_samples(int num_samples);
imu_read_data imu_subsystem_current_data_read();
int imu_subsystem_setup();


#endif /* IMU_INTERFACE_H_ */
