#ifndef INTENSITY_MOTOR_CONFIG_H
#define INTENSITY_MOTOR_CONFIG_H
// Define PWM properties
const int motorPin = 27;            // GPIO27 connected to motor driver PWM input
const int pwmChannel = 0;           // PWM channel (0-15 for ESP32)
const int pwmFrequency = 22000;     // PWM frequency in Hz
const int pwmResolution = 8;        // PWM resolution in bits (0-255 for 8-bit)
#endif