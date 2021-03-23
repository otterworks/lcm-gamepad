AC_INIT([lcm-gamepad],
	m4_esyscmd([.build-aux/git-version-gen .tarball-version]),
	[https://github.com/otterworks/lcm-gamepad/issues],
	[],
	[https://github.com/otterworks/lcm-gamepad],
)
AC_CONFIG_AUX_DIR([.build-aux])
AC_CONFIG_MACRO_DIRS([.build-aux/m4])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects no-dist-gzip dist-xz -Wall -Werror foreign])
AC_CONFIG_FILES([Makefile])

AC_SEARCH_LIBS([argp_parse],[argp])

PKG_CHECK_MODULES([LCM], [lcm >= 1.3.1])
AC_CHECK_PROG([LCMGEN], [lcm-gen], [lcm-gen], [/bin/false])
AS_IF([test "${LCMGEN}" == "x/bin/false"],
      AC_MSG_ERROR([LCM generator `lcm-gen` not found])])

AC_SEARCH_LIBS([lcm_create],[lcm])

AC_PROG_CC
AC_OUTPUT
