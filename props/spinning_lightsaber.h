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

  // Pin definitions
  static const int LED_STRIP_PIN = bladePowerPin1;     // LED pin for LED strip
  static const int RETRACTION_MOTOR_PIN = bladePowerPin6; // LED pin for retraction motor 
  static const int CANE_ROTATION_MOTOR_PIN = bladePowerPin2; // LED pin for cane rotation motor
  static const int CLUTCH_PIN = bladePowerPin3;  // LED pin for clutch control
  static const int CHASSIS_SPIN_PIN = bladePowerPin5; // LED pin for chassis spinning

  uint32_t pressed_counter_ = 0;
  uint32_t last_check_time_ = 0;
  uint32_t clutch_return_time_ = 0;
  uint32_t blade_tighten_time_ = 0;
  uint32_t blade_tension_time_ = 0;
  uint32_t activation_buffer_ = 0;
  uint32_t failsafe_off_ = 0;
  uint32_t ignite_timer_ = 0;
  uint32_t sound_off_ = 0;

    // State tracking
    bool is_on_ = false;
    bool retracted_ = true;

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
    LSanalogWriteSetup(CHASSIS_SPIN_PIN);
    analogWrite(CHASSIS_SPIN_PIN, 0);
  }

  // Main loop function that gets called by ProffieOS
  void Loop() override {
    PropBase::Loop();

    if (millis() > ignite_timer_ && ignite_timer_ > 0) {
      ignite_timer_ = 0;
      LSanalogWrite(CHASSIS_SPIN_PIN, 1500);
      SaberBase::TurnOn();
      LSanalogWrite(LED_STRIP_PIN, 26000);
      digitalWrite(CLUTCH_PIN, HIGH);
      clutch_return_time_ = millis() + 350;
    }
  
    // Check for clutch return timing
    if (millis() > clutch_return_time_ && clutch_return_time_ > 0) {
      digitalWrite(CLUTCH_PIN, LOW);
      clutch_return_time_ = 0;
      blade_tighten_time_ = millis() + 150;
      LSanalogWrite(RETRACTION_MOTOR_PIN, 3000);
    }

    // Check for blade tightening
    if (millis() > blade_tighten_time_ && blade_tighten_time_ > 0) {
      LSanalogWrite(RETRACTION_MOTOR_PIN, 2000);
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
      sound_off_ = 0;
    }

    // Failsafe off
    if (failsafe_off_ > 0 && millis() > failsafe_off_) {
      DeactivateSaber();
	    LSanalogWrite(CHASSIS_SPIN_PIN, 0);
      LSanalogWrite(LED_STRIP_PIN, 0);
      LSanalogWrite(RETRACTION_MOTOR_PIN, 0);
      digitalWrite(CANE_ROTATION_MOTOR_PIN, LOW);
      digitalWrite(CLUTCH_PIN, LOW);
      failsafe_off_ = 0;
    }

    if (millis() - last_check_time_ >= 300) { 
	    last_check_time_ = millis();
	    if (pressed_counter_ < 1 && is_on_ && retracted_ && millis() > activation_buffer_) {
		    activation_buffer_ = millis() + 15000;
      	DeactivateSaber();
	  } 
	    pressed_counter_ = 0;
	}

}

   bool IsOn() override {
    return is_on_;
  }

   bool Event2(enum BUTTON button, EVENT event, uint32_t modifiers) override {
    switch (EVENTID(button, event, modifiers)) {
    case EVENTID(BUTTON_POWER, EVENT_CLICK_LONG, MODE_ON):
      if (!is_on_ && retracted_ && millis() > activation_buffer_) {
      ActivateSaber(); } 

    case EVENTID(BUTTON_POWER, EVENT_CLICK_LONG, MODE_OFF):
      if (!is_on_ && retracted_ && millis() > activation_buffer_) {
      ActivateSaber(); } 

    case EVENTID(BUTTON_POWER, EVENT_DOUBLE_CLICK, MODE_ON):
      if (is_on_ && !retracted_ && millis() > activation_buffer_) {
      BeginRetraction(); }

    case EVENTID(BUTTON_POWER, EVENT_DOUBLE_CLICK, MODE_OFF):
      if (is_on_ && !retracted_ && millis() > activation_buffer_) {
      BeginRetraction(); }

    case EVENTID(BUTTON_POWER, EVENT_CLICK_SHORT, MODE_ON):
      pressed_counter_ = pressed_counter_ + 1;

    case EVENTID(BUTTON_POWER, EVENT_CLICK_SHORT, MODE_OFF):
      pressed_counter_ = pressed_counter_ + 1;

    default:
      return false;
  }
}

  void ActivateSaber() {
    if (is_on_) return;
    is_on_ = true;
    ignite_timer_ = millis() + 300;
    retracted_ = false;
    activation_buffer_ = millis() + 6000;
    failsafe_off_ = millis() + 20000;
  }
  
  void BeginRetraction() {
    failsafe_off_ = millis() + 5000;
    sound_off_ = millis() + 3000;
    digitalWrite(CANE_ROTATION_MOTOR_PIN, HIGH);
    LSanalogWrite(RETRACTION_MOTOR_PIN, 21000);
    LSanalogWrite(CHASSIS_SPIN_PIN, 3000);
    retracted_ = true;
    activation_buffer_ = millis() + 2000;
  }
  
  void DeactivateSaber() {
    if (!is_on_) return;
    is_on_ = false;
    LSanalogWrite(LED_STRIP_PIN, 0);
    LSanalogWrite(CHASSIS_SPIN_PIN, 0);
    LSanalogWrite(RETRACTION_MOTOR_PIN, 0);
    digitalWrite(CANE_ROTATION_MOTOR_PIN, LOW);
    digitalWrite(CLUTCH_PIN, LOW);
  }

};

#endif
