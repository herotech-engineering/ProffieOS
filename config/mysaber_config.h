#ifdef CONFIG_TOP
#include "proffieboard_v3_config.h"
#define NUM_BLADES 1
#define NUM_BUTTONS 1
#define VOLUME 2000
#define CLASH_THRESHOLD_G 100.0
#define ENABLE_AUDIO
#define ENABLE_MOTION
#define ENABLE_SD
#define MOTION_TIMEOUT 60 * 10 * 1000  // 10 minutes before motion timeout
#define IDLE_OFF_TIME 60 * 10 * 1000   // 10 minutes idle before powering down
#endif

#ifdef CONFIG_PROP
#include "../props/project_jedi.h"  // Include our custom prop
#endif

#ifdef CONFIG_PRESETS
Preset presets[] = {
  {"Skywalker", "tracks/venus.wav", 
  StyleNormalPtr<CYAN, WHITE, 300, 800>(), "Ignition" }
};

struct myLED {
    static constexpr float MaxAmps = 1.5;
    static constexpr float MaxVolts = 18;
    static constexpr float P2Amps= 0.75;
    static constexpr float P2Volts = 9;
    static constexpr float R = 10000;
    static const int Red = 0;
    static const int Green = 0;
    static const int Blue = 255;
};

BladeConfig blades[] = {
 { 0, 
SimpleBladePtr<myLED, NoLED, NoLED, NoLED, bladePowerPin6, -1, -1, -1>(),
 CONFIGARRAY(presets)}
};

#endif

#ifdef CONFIG_BUTTONS
Button PowerButton(BUTTON_POWER, powerButtonPin, "pow");
#endif
