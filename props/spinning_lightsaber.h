// Custom prop file for herotech lightsaber
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
  static const int RETRACTION_MOTOR_PIN = bladePowerPin5; // LED pin for retraction motor 
  static const int CLUTCH_PIN = bladePowerPin1; // LED pin for clutch control
  static const int CHASSIS_SPIN_PIN = bladePowerPin4; // LED pin for chassis spinning
  static const int CANE_ROTATION_DIR_PIN = blade5Pin; // Free 1 pin
  static const int CANE_ROTATION_PWM_PIN = blade7Pin; // Free 3 pin

  uint32_t pressed_counter_ = 0;
  uint32_t last_check_time_ = 0;
  uint32_t clutch_return_time_ = 0;
  uint32_t blade_tighten_time_ = 0;
  uint32_t blade_tension_time_ = 0;
  uint32_t activation_buffer_ = 0;
  uint32_t failsafe_off_ = 0;
  uint32_t time_up_ = 0;
  uint32_t ignite_timer_ = 0;
  uint32_t sound_off_ = 0;
  uint32_t power_down_ = 0;
  uint32_t pwm_level_ = 20000;
  uint32_t swing_speed_check = 0;
  uint32_t wobble_check_time = 0;
  uint32_t ramp_time_ = 0;
  uint32_t current_pwm_ = 0;
  uint32_t target_pwm_ = 0;
  uint32_t headstart_tighten_time_ = 0;
  float swing_threshold_tighten = 90.0; // TUNE THRESHOLD FOR TIGHTENING DURING SWINGS
  float anchor_angle_ = 0;
  float wobble_arc_ = 0.9; // TUNE FOR SABER WOBBLE, LOWER NUMBERS FOR INCREASED SENSITIVITY
  int last_direction_ = 1; 
  int oscillation_count_ = 0;
  int oscillation_limit_ = 15; // TUNE FOR SABER WOBBLE, LOWER NUMBERS FOR INCREASED SENSITIVITY
  uint32_t oscillation_increment_time_ = 0;

    // State tracking
    bool is_on_ = false;
    bool retracted_ = true;

  void Setup() override {
    PropBase::Setup();
    
    // Initialize pins
    pinMode(RETRACTION_MOTOR_PIN, OUTPUT);
    pinMode(CANE_ROTATION_DIR_PIN, OUTPUT);
    pinMode(CANE_ROTATION_PWM_PIN, OUTPUT);
    pinMode(CLUTCH_PIN, OUTPUT);
    pinMode(CHASSIS_SPIN_PIN, OUTPUT);

    //Turn everything off initially
    LSanalogWriteSetup(RETRACTION_MOTOR_PIN);
    analogWrite(RETRACTION_MOTOR_PIN, 0);
    digitalWrite(CANE_ROTATION_DIR_PIN, LOW);
    digitalWrite(CANE_ROTATION_PWM_PIN, LOW);
    //LSanalogWriteSetup(CANE_ROTATION_PWM_PIN);
    //analogWrite(CANE_ROTATION_PWM_PIN, 0);
    LSanalogWriteSetup(CLUTCH_PIN);
    analogWrite(CLUTCH_PIN, 0);
    LSanalogWriteSetup(CHASSIS_SPIN_PIN);
    analogWrite(CHASSIS_SPIN_PIN, 0);
  }

  // Main loop function that gets called by ProffieOS
  void Loop() override {
    PropBase::Loop();

    if (millis() - swing_speed_check > 50 && is_on_ && !retracted_ && millis() > activation_buffer_) {
    float swing_speed_speed = fusor.swing_speed();
    //float swing_accel_accel = fusor.swing_accel();
    //STDOUT.println(swing_accel_accel);
    if (swing_speed_speed > swing_threshold_tighten) {
      //digitalWrite(CANE_ROTATION_DIR_PIN, HIGH);
      //digitalWrite(CANE_ROTATION_PWM_PIN, HIGH);
      //LSanalogWrite(CANE_ROTATION_PWM_PIN, 11000);
      LSanalogWrite(RETRACTION_MOTOR_PIN, 5000);
      LSanalogWrite(CHASSIS_SPIN_PIN, 27000); // TUNE TO NON-CRITICAL SPEED FOR YOUR SABER
      swing_speed_check = millis() + 600;
      swing_threshold_tighten = 85.0;
    } else {
      //digitalWrite(CANE_ROTATION_DIR_PIN, HIGH);
      //digitalWrite(CANE_ROTATION_PWM_PIN, LOW);
      //LSanalogWrite(CANE_ROTATION_PWM_PIN, 0);
      LSanalogWrite(RETRACTION_MOTOR_PIN, 3400); 
      LSanalogWrite(CHASSIS_SPIN_PIN, 8000); // TUNE TO NON-CRITICAL SPEED FOR YOUR SABER
      swing_threshold_tighten = 90.0;
      swing_speed_check = millis();
    }
    
    }

    if (ramp_time_ > 0 && millis() > ramp_time_ && is_on_ && !retracted_ && millis() > activation_buffer_) {
      if (target_pwm_ < current_pwm_) {
      current_pwm_ -= 100;
      } 
      if (target_pwm_ > current_pwm_) {
      current_pwm_ += 100;
      }
      
      //LSanalogWrite(CHASSIS_SPIN_PIN, current_pwm_);

      if (current_pwm_ == target_pwm_) { 
      ramp_time_ = 0; 
      } else {
      ramp_time_ = millis() + 10; // schedule next reduction
      }
}

    if (millis() - wobble_check_time > 50 && is_on_ && !retracted_) {
    float current_angle = fusor.pov_angle();
    float diff = current_angle - anchor_angle_;
    bool direction_reversed = false;

    if (diff > wobble_arc_) {
      if (last_direction_ == -1) {
        direction_reversed = true;
      }
      last_direction_ = 1;
      anchor_angle_ = current_angle;
    } 
    else if (diff < -wobble_arc_) {
      if (last_direction_ == 1) {
        direction_reversed = true; 
      }
      last_direction_ = -1;
      anchor_angle_ = current_angle;
    }

    anchor_angle_ = current_angle;

    if (direction_reversed) {
      oscillation_count_++;
    }

    wobble_check_time = millis();

    }

    if (millis() - oscillation_increment_time_ > 1000 && is_on_ && !retracted_) {
      oscillation_increment_time_ = millis();
      if (oscillation_count_ > oscillation_limit_) {
      //LSanalogWrite(CHASSIS_SPIN_PIN, 0); // TUNE TO SECOND NON-CRITICAL SPEED FOR YOUR SABER, BACKUP NON-CRITICAL SPEED
      }
      oscillation_count_ = 0;
    }
    
    if (millis() > ignite_timer_ && ignite_timer_ > 0) {
      ignite_timer_ = 0;
      SaberBase::TurnOn();
      LSanalogWrite(CLUTCH_PIN, 6000);
      headstart_tighten_time_ = millis() + 140;
      clutch_return_time_ = millis() + 280;
      anchor_angle_ = fusor.pov_angle();
    }
    
    if (millis() > headstart_tighten_time_ && headstart_tighten_time_ > 0) {
      headstart_tighten_time_ = 0;
      digitalWrite(CANE_ROTATION_DIR_PIN, HIGH);
      digitalWrite(CANE_ROTATION_PWM_PIN, HIGH);
      //LSanalogWrite(CANE_ROTATION_PWM_PIN, 32768);
    }
  
    // Check for clutch return timing
    if (millis() > clutch_return_time_ && clutch_return_time_ > 0) {
      LSanalogWrite(CLUTCH_PIN, 0);
      clutch_return_time_ = 0;
      blade_tighten_time_ = millis() + 2000;
      LSanalogWrite(CHASSIS_SPIN_PIN, 4000); // TUNE TO NON-CRITICAL SPEED FOR YOUR SABER, TIGHTENING SEQUENCE
      LSanalogWrite(RETRACTION_MOTOR_PIN, 4500);
      digitalWrite(CANE_ROTATION_DIR_PIN, HIGH);
      digitalWrite(CANE_ROTATION_PWM_PIN, HIGH);
      //LSanalogWrite(CANE_ROTATION_PWM_PIN, 32768);
    }

    // Check for blade tightening
    if (millis() > blade_tighten_time_ && blade_tighten_time_ > 0) {
      LSanalogWrite(RETRACTION_MOTOR_PIN, 6000);
      blade_tighten_time_ = 0;
      blade_tension_time_ = millis() + 1500;
    }

    // Check for blade tensioning
    if (millis() > blade_tension_time_ && blade_tension_time_ > 0) {
      LSanalogWrite(RETRACTION_MOTOR_PIN, 3400);
      LSanalogWrite(CHASSIS_SPIN_PIN, 8000); // TUNE TO NON-CRITICAL SPEED FOR YOUR SABER 
      digitalWrite(CANE_ROTATION_PWM_PIN, LOW);
      //LSanalogWrite(CANE_ROTATION_PWM_PIN, 0);
      blade_tension_time_ = 0;
    }

    if (sound_off_ > 0 && millis() > sound_off_) {
      SaberBase::TurnOff(SaberBase::OFF_NORMAL);
      sound_off_ = 0;
    }

    if (time_up_ > 0 && millis() > time_up_) {
      if (is_on_ && !retracted_ && millis() > activation_buffer_) {
      BeginRetraction(); }
      time_up_ = 0;
    }

    if (power_down_ > 0 && millis() > power_down_) {
      LSanalogWrite(RETRACTION_MOTOR_PIN, pwm_level_);
      pwm_level_ -= 1000;
      if (pwm_level_ < 0) { 
      pwm_level_ = 0; 
      }

      if (pwm_level_ == 0) {
      power_down_ = 0;
      pwm_level_ = 20000; // reset for next use
      } else {
      power_down_ = millis() + 100; // schedule next reduction
      }
}

    // Failsafe off
    if (failsafe_off_ > 0 && millis() > failsafe_off_) {
      DeactivateSaber();
      target_pwm_ = 0;
      current_pwm_ = 0;
      ramp_time_ = 0;
	    LSanalogWrite(CHASSIS_SPIN_PIN, 0);
      LSanalogWrite(RETRACTION_MOTOR_PIN, 0);
      digitalWrite(CANE_ROTATION_DIR_PIN, LOW);
      digitalWrite(CANE_ROTATION_PWM_PIN, LOW);
      //LSanalogWrite(CANE_ROTATION_PWM_PIN, 0);
      LSanalogWrite(CLUTCH_PIN, 0);
      failsafe_off_ = 0;
      power_down_ = 0;
      pwm_level_ = 20000;
    }

    if (millis() - last_check_time_ >= 250) { 
	    last_check_time_ = millis();
	    if (pressed_counter_ < 1 && is_on_ && retracted_ && millis() > activation_buffer_) {
      	DeactivateSaber();
        power_down_ = 0;
        pwm_level_ = 20000;
        target_pwm_ = 0;
        current_pwm_ = 0;
        ramp_time_ = 0;
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
    LSanalogWrite(CHASSIS_SPIN_PIN, 4000); // TUNE TO NON-CRITICAL SPEED FOR YOUR SABER, IGNITION
    retracted_ = false;
    activation_buffer_ = millis() + 4200;
    time_up_ = millis() + 16000;
  }
  
  void BeginRetraction() {
    failsafe_off_ = millis() + 5000;
    sound_off_ = millis() + 2000;
    digitalWrite(CANE_ROTATION_DIR_PIN, LOW);
    digitalWrite(CANE_ROTATION_PWM_PIN, HIGH);
    //LSanalogWrite(CANE_ROTATION_PWM_PIN, 32768);
    LSanalogWrite(RETRACTION_MOTOR_PIN, 24000);
    power_down_ = millis() + 2100;
    LSanalogWrite(CHASSIS_SPIN_PIN, 8000);
    retracted_ = true;
    activation_buffer_ = millis() + 200;
  }
  
  void DeactivateSaber() {
    if (!is_on_) return;
    is_on_ = false;
    target_pwm_ = 0;
    current_pwm_ = 0;
    ramp_time_ = 0;
    LSanalogWrite(CHASSIS_SPIN_PIN, 0);
    LSanalogWrite(RETRACTION_MOTOR_PIN, 0);
    digitalWrite(CANE_ROTATION_DIR_PIN, LOW);
    digitalWrite(CANE_ROTATION_PWM_PIN, LOW);
    //LSanalogWrite(CANE_ROTATION_PWM_PIN, 0);
    LSanalogWrite(CLUTCH_PIN, 0);
    power_down_ = 0;
    pwm_level_ = 20000;
    activation_buffer_ = millis() + 60000;
  }

};

#endif
