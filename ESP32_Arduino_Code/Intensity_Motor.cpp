#include "Intensity_Motor_Headers/Intensity_Motor_Config.h"
#include "Intensity_Motor_Headers/Intensity_Motor_Interface.h"
#include "Arduino.h"
#include "driver/ledc.h" // Include LEDC driver header

// Function to Initialize The Intensity Motor
void Intensity_Motor_Init() {
  // Configure the PWM timer
  ledc_timer_config_t pwmTimerConfig;
  pwmTimerConfig.speed_mode       = LEDC_HIGH_SPEED_MODE;           // High-speed mode
  pwmTimerConfig.timer_num        = LEDC_TIMER_0;                   // Timer 0
  pwmTimerConfig.freq_hz          = pwmFrequency;                   // PWM frequency in Hz
  pwmTimerConfig.duty_resolution  = (ledc_timer_bit_t)pwmResolution; // PWM resolution
  pwmTimerConfig.clk_cfg          = LEDC_AUTO_CLK;                  // Auto select the source clock

  // Apply the PWM timer configuration
  esp_err_t timer_config_result = ledc_timer_config(&pwmTimerConfig);
  if (timer_config_result != ESP_OK) {
    Serial.println("Failed to configure PWM timer.");
    return;
  }

  // Configure the PWM channel
  ledc_channel_config_t pwmChannelConfig;
  pwmChannelConfig.speed_mode     = LEDC_HIGH_SPEED_MODE;          // Must match timer's speed mode
  pwmChannelConfig.channel         = (ledc_channel_t)pwmChannel;  // PWM channel
  pwmChannelConfig.timer_sel       = LEDC_TIMER_0;                 // Select the timer
  pwmChannelConfig.intr_type       = LEDC_INTR_DISABLE;            // Disable interrupts
  pwmChannelConfig.gpio_num        = motorPin;                     // GPIO number
  pwmChannelConfig.duty            = 0;                            // Initial duty cycle
  pwmChannelConfig.hpoint          = 0;                            // Set hpoint to 0

  // Apply the PWM channel configuration
  esp_err_t channel_config_result = ledc_channel_config(&pwmChannelConfig);
  if (channel_config_result != ESP_OK) {
    Serial.println("Failed to configure PWM channel.");
    return;
  }

  // Initialize motor speed to 0%
  setMotorSpeed(0);
}

// Function to set motor speed based on percentage
void setMotorSpeed(int percent) {
  // Calculate duty cycle based on percentage and resolution
  // For 8-bit resolution: 0-255
  int maxDuty = (1 << pwmResolution) - 1; // e.g., 255 for 8-bit
  int dutyCycle = map(percent, 0, 100, 0, maxDuty);

  // Set the PWM duty cycle
  esp_err_t set_duty_result = ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)pwmChannel, dutyCycle);
  if (set_duty_result != ESP_OK) {
    Serial.println("Failed to set PWM duty cycle.");
    return;
  }

  // Update the duty cycle to apply the new value
  esp_err_t update_duty_result = ledc_update_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)pwmChannel);
  if (update_duty_result != ESP_OK) {
    Serial.println("Failed to update PWM duty cycle.");
    return;
  }

  // Serial Print for testing
  Serial.print("Motor speed set to ");
  Serial.print(percent);
  Serial.println("%");  
}
