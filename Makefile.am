EXTRA_DIST = .build-aux/git-version-gen
ACLOCAL_AMFLAGS = -I .build-aux/m4
AM_CFLAGS = $(EVDEV_CFLAGS) $(LCM_CFLAGS) $(M_FLAGS)

dist_noinst_DATA = README.md \
	lcmtypes/mx/mx_joy6_t.lcm
	lcmtypes/mx/mx_joyButtonNumber_t.lcm
# pkgconfig_DATA = lcm-gamepad.pc

src/mx_%.c src/mx_%.h: lcmtypes/mx/mx_%.lcm
	$(LCMGEN) --c --c-hpath @builddir@/src --c-cpath @builddir@/src $^

bin_PROGRAMS = lcm-gamepad

lcm_gamepad_SOURCES = config.h \
	src/mx_joy6_t.h \
	src/mx_joy6_t.c \
	src/mx_joyButtonNumber_t.h \
	src/mx_joyButtonNumber_t.c \
	src/gamepad.h \
	src/gamepad-argp.h \
	src/gamepad.c
