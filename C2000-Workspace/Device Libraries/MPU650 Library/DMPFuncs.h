/*
 * DMPFuncs.h
 *
 *  Created on: Nov 23, 2013
 *      Author: Alex
 */

#include "MPUFuncs.h"

#ifndef DMPFUNCS_H_
#define DMPFUNCS_H_

void DMP_get_quaternion(float q[], Uint16 packet[]);
void DMP_get_gravity(float v[], float q[]);
void DMP_get_yaw_pitch_roll(float data[], float q[], float gravity[]);

void DMP_get_raw_accel(int16 data[], Uint16 packet[]);
void DMP_get_raw_gyro(int16 data[], Uint16 packet[]);

int initialize_DMP();


#endif /* DMPFUNCS_H_ */
