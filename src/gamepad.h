#include "1804-dualsense.h"

#define OUTPUT_PERIOD 100000000 // nanoseconds
#define STICK_DEADBAND 0.2      // normalized

#define STICK_OUTPUT_CHANNEL "XJOY"
#define BUTTON_OUTPUT_CHANNEL "XJOY_BUTTON"

#define XJ_SURGE 0
#define XJ_SWAY 1
#define XJ_HEAVE 2
#define XJ_YAW 5

#define XJ_AUTO_DEPTH 0
#define XJ_AUTO_HEADING 1
#define XJ_START_TRACKING 3
