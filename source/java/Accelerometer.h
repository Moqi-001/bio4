#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <so_jni/so_jni.h>

void setup_Accelerometer();

void Accelerometer_onSensorChanged(JFloat currX, JFloat currY, JFloat currZ,
                                   JFloat prevX, JFloat prevY, JFloat prevZ);

#endif