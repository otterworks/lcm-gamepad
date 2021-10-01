#include <libevdev/libevdev.h>

#define DEV_DEFAULT                                                            \
  "/dev/input/by-id/"                                                          \
  "usb-Sony_Computer_Entertainment_Wireless_Controller-event-joystick"

#define EV_SURGE ABS_X          // L horizontal
#define EV_SWAY ABS_Y           // L vertical
#define EV_HEAVE ABS_RY         // R vertical
#define EV_YAW ABS_RX           // R horizontal
#define EV_AUTO_DEPTH BTN_X     // Triangle
#define EV_AUTO_HEADING BTN_B   // X
#define EV_START_TRACKING BTN_C // Circle
