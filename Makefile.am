EXTRA_DIST = .build-aux/git-version-gen
ACLOCAL_AMFLAGS = -I .build-aux/m4
AM_CFLAGS = $(EVDEV_CFLAGS) $(LCM_CFLAGS) $(M_FLAGS)

dist_noinst_DATA = README.md \
	lcmtypes/mx/mx_joy6_t.lcm
	lcmtypes/mx/mx_joyButtonNumber_t.lcm
# pkgconfig_DATA = lcm-gamepad.pc

mx_%.c mx_%.h: lcmtypes/mx/mx_%.lcm
	$(LCMGEN) --c --c-hpath @builddir@ --c-cpath @builddir@ $^

bin_PROGRAMS = lcm-gamepad

lcm_gamepad_SOURCES = config.h \
	mx_joy6_t.h \
	mx_joy6_t.c \
	mx_joyButtonNumber_t.h \
	mx_joyButtonNumber_t.c \
	src/gamepad.h \
	src/gamepad-argp.h \
	src/gamepad.c
