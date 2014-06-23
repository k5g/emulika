#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([emulika], [0.2.0], [])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile src/misc/Makefile src/cpu/Makefile])

AC_CANONICAL_HOST

# Debug
#AC_ARG_ENABLE([debug],
#[  --enable-debug    Turn on debugging],
#[case "${enableval}" in
#  yes) debug=true ;;
#  no)  debug=false ;;
#  *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
#esac],[debug=false])

#AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])
#AM_CONDITIONAL([NDEBUG], [test x$debug = xno])
#AM_CXXFLAGS = -DNDEBUG

# Force CFLAGS/CPPFLAGS
#CFLAGS="-O2 -s"
CPPFLAGS="$CPPFLAGS -DNDEBUG"

case "$host" in
	*-*-linux*)
		raspberrypi=`cat /proc/cpuinfo | sed -n '/^Hardware/s/^.*: \(.*\)/\1/p'`
		if test "x$raspberrypi" = xBCM2708; then
			# CPPFLAGS="$CPPFLAGS -DRASPBERRYPI"
			AC_DEFINE([RASPBERRYPI], [1], [Define to 1 on the Raspberry Pi platform])
		fi
		;;
	*-*-mingw32)
		# CPPFLAGS="$CPPFLAGS -DWIN32"
		AC_DEFINE([WIN32], [1], [Define to 1 on the Windows platform])
		;;
esac

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Checks for headers.
AC_CHECK_HEADERS([endian.h], , )

# Checks for libraries.
# SDL
AC_CHECK_PROG(HAVE_SDL_CONFIG, sdl2-config, yes, no)

if test "$HAVE_SDL_CONFIG" = no; then
	AC_MSG_ERROR(sdl2-config doesn't exist. Try to check the PATH enviroment.)
fi

AC_CHECK_LIB([SDL2], [SDL_Init],[
		SDL_CFLAGS=`sdl2-config --cflags`
		SDL_LIBS=`sdl2-config --libs`],
		AC_MSG_ERROR([ *** Unable to find SDL library]),
		[`sdl2-config --libs`])

LIBS="$LIBS $SDL_LIBS"
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"

AC_CHECK_HEADERS([SDL2/SDL.h], ,
		AC_MSG_ERROR([ *** Unable to find SDL headers]))


AC_CHECK_FUNC([SDL_GetPrefPath], ,
		AC_MSG_ERROR([ *** SDL2 version >= 2.0.1 required]))

# xml2
AC_CHECK_PROG(HAVE_XML2_CONFIG, xml2-config, yes, no)

if test "$HAVE_XML2_CONFIG" = no; then
	AC_MSG_ERROR(xml2-config doesn't exist. Try to check the PATH enviroment.)
fi

AC_CHECK_LIB([xml2], [xmlDocGetRootElement],[
		XML2_CFLAGS=`xml2-config --cflags`
		XML2_LIBS=`xml2-config --libs`],
		AC_MSG_ERROR([ *** Unable to find libxml2 library]),
		[`xml2-config --libs`])

LIBS="$LIBS $XML2_LIBS"
CPPFLAGS="$CPPFLAGS $XML2_CFLAGS"

AC_CHECK_HEADERS([libxml/xmlwriter.h], ,
		AC_MSG_ERROR([ *** Unable to find libxml2 headers]))

#AM_PATH_XML2(2.7.2,[
#		CPPFLAGS="$CPPFLAGS $XML_CPPFLAGS"
#		LIBS="$LIBS $XML_LIBS"
#		],AC_MSG_ERROR([ *** libxml2 not detected]))


# zip
AC_CHECK_LIB([zip], [zip_open], ,
		AC_MSG_ERROR([ *** Unable to find libzip library]))
AC_CHECK_HEADERS([zip.h], ,
		AC_MSG_ERROR([ *** Unable to find libzip headers]))

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([atexit memmove memset mkdir strcasecmp _strcmpi strrchr strtol strtoul])

AC_OUTPUT

AC_MSG_RESULT([
$PACKAGE-$VERSION configuration summary:

    Install prefix .................. : ${prefix}
    C preprocessor .................. : ${CPP}
    C compiler ...................... : ${CC}
    C preprocessor flags ............ : ${CPPFLAGS}
    C compiler flags ................ : ${CFLAGS}
    Extra libraries ................. : ${LIBS}
])
