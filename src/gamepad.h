#include <libevdev/libevdev.h>

#define INI_PATH "/etc/lcm/gamepad.ini"

#define DEV_DEFAULT                                                            \
  "/dev/input/by-id/"                                                          \
  "usb-Sony_Computer_Entertainment_Wireless_Controller-event-joystick"

#define OUTPUT_PERIOD 100000000 // nanoseconds

#define STICK_OUTPUT_CHANNEL "XJOY"
#define BUTTON_OUTPUT_CHANNEL "XJOY_BUTTON"

// on DualSense
#define EV_SURGE ABS_X // L horizontal
#define EV_SWAY ABS_Y  // L vertical
#define EV_HEAVE ABS_Z // R vertical
#define EV_YAW ABS_RZ  // R horizontal

#define EV_AUTO_DEPTH BTN_X     // Triangle
#define EV_AUTO_HEADING BTN_B   // X
#define EV_START_TRACKING BTN_C // Circle

#define XJ_SURGE 1
#define XJ_SWAY 2
#define XJ_HEAVE 5
#define XJ_YAW 0

#define XJ_AUTO_DEPTH 0
#define XJ_AUTO_HEADING 1
#define XJ_START_TRACKING 3
