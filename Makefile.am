EXTRA_DIST = .build-aux/git-version-gen
ACLOCAL_AMFLAGS = -I .build-aux/m4
AM_CFLAGS = $(EVDEV_CFLAGS) $(LCM_CFLAGS)

dist_noinst_DATA = README.md \
	lcmtypes/kinematics/kinematics_twist_t.lcm
# pkgconfig_DATA = lcm-gamepad.pc

src/kinematics_%.c src/kinematics_%.h: lcmtypes/kinematics/kinematics_%.lcm
	$(LCMGEN) --c --c-hpath @builddir@/src --c-cpath @builddir@/src $^

bin_PROGRAMS = lcm-gamepad

lcm_gamepad_SOURCES = config.h \
	src/kinematics_twist_t.h \
	src/kinematics_twist_t.c \
	src/gamepad.h \
	src/gamepad-argp.h \
	src/gamepad.c
