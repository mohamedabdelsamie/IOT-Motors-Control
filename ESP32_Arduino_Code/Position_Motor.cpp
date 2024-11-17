/* position_Motor_Mood   
      200 ---> Looping
      100 ---> Forward      
       50 ---> Stop
        0 ---> Reverse
*/
#include "Position_Motor_Headers/Position_Motor_Config.h"
#include "Position_Motor_Headers/Position_Motor_Interface.h"
#include "Arduino.h"
extern int position_Motor_Mood;
int change_Direction_Flag = 0;
unsigned long LastReading = 0;
void Position_Motor_Init() {
  pinMode(position_FWD_MotorPin, OUTPUT);
  pinMode(position_REV_MotorPin, OUTPUT);
  digitalWrite(position_FWD_MotorPin, LOW);
  digitalWrite(position_REV_MotorPin, LOW);
}

void Position_Motor_Prog() {
  switch (position_Motor_Mood) {
    case 200:
      if ((millis() - LastReading) >= 600) {
        LastReading = millis();
        switch (change_Direction_Flag) {
          case 0:
          Serial.println("Start Looping");
            digitalWrite(position_FWD_MotorPin, LOW);
            digitalWrite(position_REV_MotorPin, LOW);
            change_Direction_Flag++;
            break;
          case 1:
          Serial.println("Forward");
            digitalWrite(position_REV_MotorPin, LOW);
            delay(50);
            digitalWrite(position_FWD_MotorPin, HIGH);
            change_Direction_Flag++;
            break;
          case 2:
          Serial.println("Reverse");
            digitalWrite(position_FWD_MotorPin, LOW);
            delay(50);
            digitalWrite(position_REV_MotorPin, HIGH);
            change_Direction_Flag = 1;
            break;
        }
      }
      break;
    case 100:
    Serial.println("Forward");
      digitalWrite(position_REV_MotorPin, LOW);
      delay(50);
      digitalWrite(position_FWD_MotorPin, HIGH);
      change_Direction_Flag = 0;
      break;

    case 50:
    //Serial.println("STOP !");
      digitalWrite(position_FWD_MotorPin, LOW);
      digitalWrite(position_REV_MotorPin, LOW);
      change_Direction_Flag = 0;
      break;

    case 0:
    Serial.println("Reverse");
      digitalWrite(position_FWD_MotorPin, LOW);
      delay(50);
      digitalWrite(position_REV_MotorPin, HIGH);
      change_Direction_Flag = 0;
      break;
  }
}