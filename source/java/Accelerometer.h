#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

void Accelerometer_class_init();

void Accelerometer_onSensorChanged(float currX, float currY, float currZ,
                                   float prevX, float prevY, float prevZ);

#endif