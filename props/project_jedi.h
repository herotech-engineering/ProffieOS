// Custom prop file for spinning-activated lightsaber with retraction motors
// For ProffieOS and Proffieboard V3.9
#ifndef PROPS_PROJECT_JEDI_H
#define PROPS_PROJECT_JEDI_H

#include "prop_base.h"
#include "../sound/hybrid_font.h"
#include "../motion/motion_util.h"

#define PROP_TYPE Jedi

class Jedi : public PROP_INHERIT_PREFIX PropBase {
public:
  Jedi() : PropBase() {}
  
  const char* name() override { return "Jedi"; }

  // State tracking
  bool is_on_ = false;
  enum PowerState {
    OFF,
    ON,
    RETRACTING,
  };
  PowerState power_state_ = OFF;
  
  // Pin definitions
  static const int LED_STRIP_PIN = bladePowerPin5;     // LED pin for LED strip
  static const int RETRACTION_MOTOR_PIN = bladePowerPin1; // LED pin for retraction motor 
  static const int CANE_ROTATION_MOTOR_PIN = bladePowerPin4; // LED pin for cane rotation motor
  static const int CLUTCH_PIN = bladePowerPin3;  // LED pin for clutch control
  static const int CHASSIS_SPIN_PIN = bladePowerPin2; // LED pin for chassis spinning

  uint32_t clutch_return_time_ = 0;
  uint32_t blade_tighten_time_ = 0;
  uint32_t blade_tension_time_ = 0;
  uint32_t activation_buffer_ = 0;
  uint32_t failsafe_off_ = 0;
  uint32_t ignite_timer_ = 0;
  uint32_t sound_off_ = 0;

  void Setup() override {
    PropBase::Setup();
    
    // Initialize pins
    pinMode(LED_STRIP_PIN, OUTPUT);
    pinMode(RETRACTION_MOTOR_PIN, OUTPUT);
    pinMode(CANE_ROTATION_MOTOR_PIN, OUTPUT);
    pinMode(CLUTCH_PIN, OUTPUT);
	pinMode(CHASSIS_SPIN_PIN, OUTPUT);

    // Turn everything off initially
    LSanalogWriteSetup(LED_STRIP_PIN);
	analogWrite(LED_STRIP_PIN, 0);
    LSanalogWriteSetup(RETRACTION_MOTOR_PIN);
    analogWrite(RETRACTION_MOTOR_PIN, 0);
    digitalWrite(CANE_ROTATION_MOTOR_PIN, LOW);
    digitalWrite(CLUTCH_PIN, LOW);
	digitalWrite(CHASSIS_SPIN_PIN, LOW);
  }

  // Main loop function that gets called by ProffieOS
  void Loop() override {
    PropBase::Loop();

    if (millis() > ignite_timer_ && ignite_timer_ > 0) {
      ignite_timer_ = 0;
      SaberBase::TurnOn();
    // Turn on LED strip (simple on/off, no PWM)
      analogWrite(LED_STRIP_PIN, 26000);
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
      LSanalogWrite(RETRACTION_MOTOR_PIN, 3000);
    }

    // Check for blade tightening
    if (millis() > blade_tighten_time_ && blade_tighten_time_ > 0) {
      LSanalogWrite(RETRACTION_MOTOR_PIN, 4000);
      blade_tighten_time_ = 0;
      blade_tension_time_ = millis() + 50;
    }

    // Check for blade tensioning
    if (millis() > blade_tension_time_ && blade_tension_time_ > 0) {
      LSanalogWrite(RETRACTION_MOTOR_PIN, 1500);
      blade_tension_time_ = 0;
    }

    if (sound_off_ > 0 && millis() > sound_off_) {
      SaberBase::TurnOff(SaberBase::OFF_NORMAL);
      sound_off_ = 0; // Reset timer
    }

    // Failsafe off
    if (failsafe_off_ > 0 && millis() > failsafe_off_) {
      DeactivateSaber();

      LSanalogWrite(LED_STRIP_PIN, 0);
    
      // Turn off all motors
      LSanalogWrite(RETRACTION_MOTOR_PIN, 0);
      digitalWrite(CANE_ROTATION_MOTOR_PIN, LOW);
    
      // Ensure clutch is in left position
      digitalWrite(CLUTCH_PIN, LOW);
      failsafe_off_ = 0; // Reset timer
    }

    // State machine for saber control
    switch (power_state_) {
      case OFF:
        if (!is_on_ && millis() > activation_buffer_) {
          ActivateSaber();
          power_state_ = ON;
	  activation_buffer_ = millis() + 8000;
        }
        break;
        
      case ON:
        if ( && millis() > activation_buffer_) {
          BeginRetraction();
          power_state_ = RETRACTING;
	  activation_buffer_ = millis() + 2000;
        }
        break;

      case RETRACTING:
        if ( && millis() > activation_buffer_) {
          DeactivateSaber();
          power_state_ = OFF;
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

  // Activate the lightsaber
  void ActivateSaber() {
    if (is_on_) return;
    is_on_ = true;
    ignite_timer_ = millis() + 500;
  }
  
  // Begin retraction sequence when spinning slows
  void BeginRetraction() {
    // failsafe off timing
    failsafe_off_ = millis() + 5500;
    sound_off_ = millis() + 4500;
    // Turn on cane rotation motor
    digitalWrite(CANE_ROTATION_MOTOR_PIN, HIGH);
    // Turn on both retraction motors at full power
    LSanalogWrite(RETRACTION_MOTOR_PIN, 21000);
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
