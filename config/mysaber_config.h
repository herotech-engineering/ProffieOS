#ifdef CONFIG_TOP
#include "proffieboard_v3_config.h"
#define NUM_BLADES 1
#define NUM_BUTTONS 1
#define VOLUME 2600
const unsigned int maxLedsPerStrip = 144;
#define CLASH_THRESHOLD_G 1.0
#define ENABLE_AUDIO
#define ENABLE_MOTION
#define ENABLE_SD
#define DYNAMIC_BLADE_DIMMING
#define MOTION_TIMEOUT 60 * 10 * 1000  // 10 minutes before motion timeout
#define IDLE_OFF_TIME 60 * 10 * 1000   // 10 minutes idle before powering down
#endif

#ifdef CONFIG_PROP
#include "../props/spinning_lightsaber.h"  // Include our custom prop
#endif
//REPLACE "Vader" WITH YOUR SOUNDFONT FOLDER NAME ON THE SD CARD
#ifdef CONFIG_PRESETS
Preset presets[] = {
  {"Vader", "tracks/venus.wav", 
  StyleNormalPtr<BLUE, WHITE, 800, 800>(), "Ignition" }
};

struct myLED {
    static constexpr float MaxAmps = 1.0; //1.0
    static constexpr float MaxVolts = 4.5; //4.5
    static constexpr float P2Amps= 0.75; //0.75
    static constexpr float P2Volts = 4.2; //4.2
    static constexpr float R = 0;
    static const int Red = 0;
    static const int Green = 0;
    static const int Blue = 255;
};
// REPLACE bladePowerPinX with the pin number for your LED strip
BladeConfig blades[] = {
 { 0, 
DimBlade(20.0, SimpleBladePtr<myLED, NoLED, NoLED, NoLED, bladePowerPin2, -1, -1, -1>()), 
 CONFIGARRAY(presets)}
};

#endif

#ifdef CONFIG_BUTTONS
Button PowerButton(BUTTON_POWER, powerButtonPin, "pow");
#endif
