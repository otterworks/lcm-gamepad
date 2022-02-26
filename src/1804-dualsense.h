#include <libevdev/libevdev.h>

#define DEV_DEFAULT                                                            \
  "/dev/input/by-id/"                                                          \
  "usb-Sony_Interactive_Entertainment_Wireless_Controller-if03-event-joystick"

#define EV_SURGE ABS_Y          // L vertical
#define EV_SWAY ABS_X           // L horizontal
#define EV_HEAVE ABS_RZ         // R vertical
#define EV_YAW ABS_Z            // R horizontal
#define EV_AUTO_DEPTH BTN_X     // Triangle
#define EV_AUTO_HEADING BTN_B   // X
#define EV_START_TRACKING BTN_C // Circle
