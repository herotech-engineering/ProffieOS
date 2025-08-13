// Custom prop file for spinning-activated lightsaber with retraction motors
// For ProffieOS and Proffieboard V3.9
#ifndef PROPS_SPINNING_LIGHTSABER_H
#define PROPS_SPINNING_LIGHTSABER_H

#include "prop_base.h"
#include "../sound/hybrid_font.h"
#include "../motion/motion_util.h"

#define PROP_TYPE Spinning

class Spinning : public PROP_INHERIT_PREFIX PropBase {
public:
  Spinning() : PropBase() {}
  
  const char* name() override { return "Spinning"; }

  // State tracking
  bool is_on_ = false;
  enum SpinState {
    STOPPED,
    SPINNING
  };
  SpinState spin_state_ = STOPPED;
  
  // Pin definitions
  static const int LED_STRIP_1_PIN = bladePowerPin5;     // LED5 pin for LED strip 1
  static const int LED_STRIP_2_PIN = bladePowerPin6;    // LED6 pin for LED strip 2
  static const int RETRACTION_MOTOR_1_PIN = bladePowerPin1; // LED1 pin for retraction motor 1
  static const int RETRACTION_MOTOR_2_PIN = bladePowerPin2; // LED2 pin for retraction motor 2
  static const int CANE_ROTATION_MOTOR_PIN = bladePowerPin4; // LED4 pin for cane rotation motor
  static const int CLUTCH_PIN = bladePowerPin3;  // LED3 pin for clutch control
  
  // Thresholds for spin detection
  const float SPIN_THRESHOLD = 520.0f;  // Angular velocity threshold for activation (deg/s)
  const float SLOW_THRESHOLD = 320.0f;  // Angular velocity threshold for slow spin (deg/s)
  
  bool rotating_chassis_spin_on_ = false;
  uint32_t clutch_return_time_ = 0;
  uint32_t blade_tighten_time_ = 0;
  uint32_t blade_tension_time_ = 0;
  uint32_t activation_buffer_ = 0;
  uint32_t last_check_time_ = 0;
  uint32_t failsafe_off_ = 0;
  uint32_t spin_speed_buffer_ = 0;
  uint32_t ignite_timer_ = 0;
  uint32_t sound_off_ = 0;

  void Setup() override {
    PropBase::Setup();
    
    // Initialize pins
    pinMode(LED_STRIP_1_PIN, OUTPUT);
    pinMode(LED_STRIP_2_PIN, OUTPUT);
    pinMode(RETRACTION_MOTOR_1_PIN, OUTPUT);
    pinMode(RETRACTION_MOTOR_2_PIN, OUTPUT);
    pinMode(CANE_ROTATION_MOTOR_PIN, OUTPUT);
    pinMode(CLUTCH_PIN, OUTPUT);

    // Turn everything off initially
    digitalWrite(LED_STRIP_1_PIN, LOW);
    digitalWrite(LED_STRIP_2_PIN, LOW);
    LSanalogWriteSetup(RETRACTION_MOTOR_1_PIN);
    LSanalogWriteSetup(RETRACTION_MOTOR_2_PIN);
    analogWrite(RETRACTION_MOTOR_1_PIN, 0);
    analogWrite(RETRACTION_MOTOR_2_PIN, 0);
    digitalWrite(CANE_ROTATION_MOTOR_PIN, LOW);
    digitalWrite(CLUTCH_PIN, LOW);

  }

  // Main loop function that gets called by ProffieOS
  void Loop() override {
    PropBase::Loop();

    // Get gyroscope data from IMU to detect rotation
    float rotation_speed = GetRotationSpeed();

    if (millis() > ignite_timer_ && ignite_timer_ > 0) {
      ignite_timer_ = 0;
      SaberBase::TurnOn();
    // Turn on LED strips (simple on/off, no PWM)
      digitalWrite(LED_STRIP_1_PIN, HIGH);
      digitalWrite(LED_STRIP_2_PIN, HIGH);
    // Move clutch right 5mm
      digitalWrite(CLUTCH_PIN, HIGH);
    // Schedule clutch to return after 350ms
      clutch_return_time_ = millis() + 350;
    }
  
    // Check for servo return timing
    if (millis() > clutch_return_time_ && clutch_return_time_ > 0) {
      digitalWrite(CLUTCH_PIN, LOW); // Return to left position
      clutch_return_time_ = 0; // Reset timer
      blade_tighten_time_ = millis() + 150;
      LSanalogWrite(RETRACTION_MOTOR_1_PIN, 6100);
      LSanalogWrite(RETRACTION_MOTOR_2_PIN, 6200);
    }

    // Check for blade tightening
    if (millis() > blade_tighten_time_ && blade_tighten_time_ > 0) {
      LSanalogWrite(RETRACTION_MOTOR_1_PIN, 5100);
      LSanalogWrite(RETRACTION_MOTOR_2_PIN, 5200);
      blade_tighten_time_ = 0;
      blade_tension_time_ = millis() + 50;
    }
	  
    // Check for blade tensioning
    if (millis() > blade_tension_time_ && blade_tension_time_ > 0) {
      LSanalogWrite(RETRACTION_MOTOR_1_PIN, 1550);
      LSanalogWrite(RETRACTION_MOTOR_2_PIN, 1600);
      blade_tension_time_ = 0;
    }

    if (sound_off_ > 0 && millis() > sound_off_) {
      SaberBase::TurnOff(SaberBase::OFF_NORMAL);
      sound_off_ = 0; // Reset timer
    }

    // Failsafe off
    if (failsafe_off_ > 0 && millis() > failsafe_off_) {
      DeactivateSaber();

      digitalWrite(LED_STRIP_1_PIN, LOW);
      digitalWrite(LED_STRIP_2_PIN, LOW);
    
      // Turn off all motors
      LSanalogWrite(RETRACTION_MOTOR_1_PIN, 0);
      LSanalogWrite(RETRACTION_MOTOR_2_PIN, 0);
      digitalWrite(CANE_ROTATION_MOTOR_PIN, LOW);
    
      // Ensure clutch is in left position
      digitalWrite(CLUTCH_PIN, LOW);
      failsafe_off_ = 0; // Reset timer
    }


    if (millis() - last_check_time_ >= 300) { 
	last_check_time_ = millis();

    // State machine for saber control
    switch (spin_state_) {
      case STOPPED:
        if (rotation_speed > SPIN_THRESHOLD && !is_on_ && millis() > activation_buffer_) {
          // Hilt is spinning fast enough - activate lightsaber
          ActivateSaber();
          spin_state_ = SPINNING;
	  activation_buffer_ = millis() + 12000;
	  spin_speed_buffer_ = millis() + 12000;
        }
        break;
        
      case SPINNING:
        if (rotation_speed < SLOW_THRESHOLD && millis() > spin_speed_buffer_) {
          // Spinning is slowing - start retraction
          BeginRetraction();
          spin_state_ = STOPPED;
	  activation_buffer_ = millis() + 20000;
        }
        break;
        
    }
   }
  }

  // Function to check if the saber is currently activated
  bool IsOn() override {
    return is_on_;
  }

  // Helper function to get rotation speed from the IMU
  float GetRotationSpeed() {
    // Use ProffieOS's motion system to get gyroscope data
    Vec3 gyro = fusor.gyro();
    
    // Calculate the magnitude of rotation (in degrees per second)
    return sqrtf(gyro.x * gyro.x + gyro.y * gyro.y + gyro.z * gyro.z);
  }
  
  // Activate the lightsaber
  void ActivateSaber() {
    if (is_on_) return;
    is_on_ = true;
    ignite_timer_ = millis() + 8000;
  }
  
  // Begin retraction sequence when spinning slows
  void BeginRetraction() {
    // failsafe off timing
    failsafe_off_ = millis() + 5500;
    sound_off_ = millis() + 4500;
    // Turn on cane rotation motor
    digitalWrite(CANE_ROTATION_MOTOR_PIN, HIGH);
    // Turn on both retraction motors at full power
    LSanalogWrite(RETRACTION_MOTOR_1_PIN, 32700);
    LSanalogWrite(RETRACTION_MOTOR_2_PIN, 32700);
  }
  
  // Deactivate the lightsaber
  void DeactivateSaber() {
    if (!is_on_) return;
    is_on_ = false;
    // Turn off LED strips
    digitalWrite(LED_STRIP_1_PIN, LOW);
    digitalWrite(LED_STRIP_2_PIN, LOW);
    // Turn off all motors
    LSanalogWrite(RETRACTION_MOTOR_1_PIN, 0);
    LSanalogWrite(RETRACTION_MOTOR_2_PIN, 0);
    digitalWrite(CANE_ROTATION_MOTOR_PIN, LOW);
    // Ensure servo is in left position
    digitalWrite(CLUTCH_PIN, LOW);
  }
  
};

#endif // PROPS_SPINNING_LIGHTSABER_H
