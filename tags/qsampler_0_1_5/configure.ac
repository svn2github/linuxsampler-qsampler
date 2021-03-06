# Process this file with autoconf to produce a configure script.
AC_INIT(Qsampler, 0.1.5, rncbc@rncbc.org, qsampler)
AC_CONFIG_SRCDIR(src/qsamplerMainForm.ui.h)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES(Makefile qsampler.pro qsampler.spec qsampler.desktop)

# Set default installation prefix.
AC_PREFIX_DEFAULT(/usr/local)
ac_prefix=$prefix
if test "x$ac_prefix" = "xNONE"; then
    ac_prefix=$ac_default_prefix
fi
AC_SUBST(ac_prefix)
AC_DEFINE_UNQUOTED(CONFIG_PREFIX, ["$ac_prefix"], [Default installation prefix.])

# Enable debugging argument option.
AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug], [enable debugging (default=no)]),
  [ac_debug="$enableval"])

if test "x$ac_debug" = "xyes"; then
   AC_DEFINE(CONFIG_DEBUG, 1, [Define if debugging is enabled.])
   ac_debug="debug"
else
   ac_debug="release"
fi
AC_SUBST(ac_debug)

# Enable libgig availability.
AC_ARG_ENABLE(libgig,
  AC_HELP_STRING([--enable-libgig], [enable libgig interface (default=yes)]),
  [ac_libgig="$enableval"],
  [ac_libgig="yes"])

# Standard installation base dirs.
ac_with_paths="/usr /usr/local"

# Some a-la-debian alternatives...
for X in /usr/lib /usr/share; do
  for Y in qt qt3; do
    if test -d $X/$Y/bin; then
      ac_with_paths="$ac_with_paths $X/$Y"
    fi
  done
done

# Set for alternate liblscp installation dir.
AC_ARG_WITH(liblscp,
  AC_HELP_STRING([--with-liblscp=PATH], [use alternate liblscp install path]),
  [ac_with_paths="$ac_with_paths $withval"])

# Set for alternate libgig installation dir.
AC_ARG_WITH(libgig,
  AC_HELP_STRING([--with-libgig=PATH], [use alternate libgig install path]),
  [ac_with_paths="$ac_with_paths $withval"])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_GCC_TRADITIONAL

# Checks for languages.
AC_LANG_C
AC_LANG_CPLUSPLUS

# Check for QTDIR environment variable.
AC_MSG_CHECKING([whether QTDIR environment variable is set])
if test "x$QTDIR" = "x"; then
  AC_MSG_RESULT([no])
else
  AC_MSG_RESULT([$QTDIR])
  ac_with_paths="$ac_with_paths $QTDIR"
fi

# Prepend alternate dependencies paths.
ac_path=$PATH
for X in $ac_with_paths; do
  if test -d $X/bin; then
    ac_path="$X/bin:$ac_path"
  fi
  if test -d $X/include; then
    for Y in qt qt3; do
      if test -d $X/include/$Y; then
        CFLAGS="-I$X/include/$Y $CFLAGS"
        CPPFLAGS="-I$X/include/$Y $CPPFLAGS"
        ac_incpath="$X/include/$Y $ac_incpath"
      fi
    done
    CFLAGS="-I$X/include $CFLAGS"
    CPPFLAGS="-I$X/include $CPPFLAGS"
    ac_incpath="$X/include $ac_incpath"
  fi
  if test -d $X/lib64; then
    LIBS="-L$X/lib64 $LIBS"
    ac_libs="-L$X/lib64 $ac_libs"
  fi
  if test -d $X/lib; then
    LIBS="-L$X/lib $LIBS"
    ac_libs="-L$X/lib $ac_libs"
  fi
done

# Checks for Qt library.
for X in qt-mt qt; do
  if test "x$ac_qtlib" = "x"; then
     AC_CHECK_LIB($X, main, [ac_qtlib=$X])
  fi
done
if test "x$ac_qtlib" = "x"; then
  AC_MSG_ERROR([Qt library not found. Maybe QTDIR isn't properly set.])
fi
AC_SUBST(ac_qtlib)

# Check for Qt multi-thread support.
if test "x$ac_qtlib" = "xqt-mt"; then
  ac_thread="thread"
fi
AC_SUBST(ac_thread)

AC_CACHE_CHECK([for Qt library version >= 3.1.1],
  ac_qtlib_version, [
  AC_TRY_LINK([#include "qglobal.h"], [
#if QT_VERSION < 0x030101
#error Qt library 3.1.1 or greater required.
#endif
    ],
    ac_qtlib_version="yes", [
      echo "no; Qt 3.1.1 or greater is required"
      exit
    ])
])

# A common error message:
ac_qtdir_errmsg="not found in current PATH. Maybe QT development environment isn't available (qt3-devel)."

# Check for Qt qmake utility.
AC_PATH_PROG(ac_qmake, qmake, [no], $ac_path)
if test "x$ac_qmake" = "xno"; then
   AC_MSG_ERROR([qmake $ac_qtdir_errmsg])
fi
AC_SUBST(ac_qmake)

# Check for Qt moc utility.
AC_PATH_PROG(ac_moc, moc, [no], $ac_path)
if test "x$ac_moc" = "xno"; then
   AC_MSG_ERROR([moc $ac_qtdir_errmsg])
fi
AC_SUBST(ac_moc)

# Check for Qt uic utility.
AC_PATH_PROG(ac_uic, uic, [no], $ac_path)
if test "x$ac_uic" = "xno"; then
   AC_MSG_ERROR([uic $ac_qtdir_errmsg])
fi
AC_SUBST(ac_uic)

# Checks for libraries.
AC_CHECK_LIB(m, main)
AC_CHECK_LIB(X11, main)
AC_CHECK_LIB(Xext, main)
AC_CHECK_LIB($ac_qtlib, main)

# Check for round math function.
AC_CHECK_LIB(m, round, [ac_round="yes"], [ac_round="no"])
if test "x$ac_round" = "xyes"; then
   AC_DEFINE(CONFIG_ROUND, 1, [Define if round is available.])
fi

# Check for mandatory libraries.
AC_CHECK_LIB(lscp, main, [ac_liblscp="yes"], [ac_liblscp="no"])
if test "x$ac_liblscp" = "xno"; then
   AC_MSG_ERROR([LSCP library not found.])
fi
ac_libs="$ac_libs -llscp"

AC_CACHE_CHECK([for instrument_name in lscp_channel_info_t],
  ac_instrument_name, [
  AC_TRY_COMPILE([#include "lscp/client.h"], [
	lscp_channel_info_t info;
	info.instrument_name = 0;
    ], ac_instrument_name="yes", ac_instrument_name="no")
])
if test "x$ac_instrument_name" = "xyes"; then
   AC_DEFINE(CONFIG_INSTRUMENT_NAME, 1, [Define if instrument_name is available.])
fi

AC_CACHE_CHECK([for mute/solo in lscp_channel_info_t],
  ac_mute_solo, [
  AC_TRY_COMPILE([#include "lscp/client.h"], [
	lscp_channel_info_t info;
	info.mute = 0;
	info.solo = 0;
    ], ac_mute_solo="yes", ac_mute_solo="no")
])
if test "x$ac_mute_solo" = "xyes"; then
   AC_CHECK_LIB(lscp, lscp_set_channel_mute, [ac_mute_solo="yes"], [ac_mute_solo="no"])
fi
if test "x$ac_mute_solo" = "xyes"; then
   AC_CHECK_LIB(lscp, lscp_set_channel_solo, [ac_mute_solo="yes"], [ac_mute_solo="no"])
fi
if test "x$ac_mute_solo" = "xyes"; then
   AC_DEFINE(CONFIG_MUTE_SOLO, 1, [Define if mute/solo is available.])
fi

AC_CHECK_LIB(lscp, lscp_map_midi_instrument, [ac_midi_instrument="yes"], [ac_midi_instrument="no"])
if test "x$ac_midi_instrument" = "xyes"; then
  AC_DEFINE(CONFIG_MIDI_INSTRUMENT, 1, [Define if MIDI instrument mapping is available.])
fi

AC_CHECK_LIB(lscp, lscp_create_fxsend, [ac_fxsend="yes"], [ac_fxsend="no"])
if test "x$ac_fxsend" = "xyes"; then
  AC_DEFINE(CONFIG_FXSEND, 1, [Define if FX sends is available.])
  AC_CACHE_CHECK([for FX send level in lscp_fxsend_info_t],
    ac_fxsend_level, [
    AC_TRY_COMPILE([#include "lscp/client.h"], [
	  lscp_fxsend_info_t info;
	  info.level = 0.0f;
      ], ac_fxsend_level="yes", ac_fxsend_level="no")
  ])
  if test "x$ac_fxsend_level" = "xyes"; then
	  AC_DEFINE(CONFIG_FXSEND_LEVEL, 1, [Define if FX send level is available.])
  fi
fi

AC_CACHE_CHECK([for audio_routing array type],
  ac_audio_routing, [
  AC_TRY_COMPILE([#include "lscp/client.h"], [
    lscp_channel_info_t info;
    char ch = info.audio_routing[0][0];
    ], ac_audio_routing="no", ac_audio_routing="yes")
])
if test "x$ac_audio_routing" = "xyes"; then
   AC_DEFINE(CONFIG_AUDIO_ROUTING, 1, [Define if audio_routing is an integer array.])
fi

AC_CHECK_LIB(lscp, lscp_set_volume, [ac_volume="yes"], [ac_volume="no"])
if test "x$ac_volume" = "xyes"; then
  AC_DEFINE(CONFIG_VOLUME, 1, [Define if global volume is available.])
fi

AC_CHECK_LIB(lscp, lscp_edit_channel_instrument, [ac_edit_instrument="yes"], [ac_edit_instrument="no"])
if test "x$ac_edit_instrument" = "xyes"; then
  AC_DEFINE(CONFIG_EDIT_INSTRUMENT, 1, [Define if instrument editing is available.])
fi

# Check for optional libraries.
if test "x$ac_libgig" = "xyes"; then
   AC_CHECK_LIB(gig, main, [ac_libgig="yes"], [ac_libgig="no"])
   if test "x$ac_libgig" = "xyes"; then
      AC_DEFINE(CONFIG_LIBGIG, 1, [Define if libgig is available.])
      ac_libs="$ac_libs -lgig"
   fi
fi

# Check for round math function.
AC_CHECK_LIB(m, lroundf, [ac_round="yes"], [ac_round="no"])
if test "x$ac_round" = "xyes"; then
   AC_DEFINE(CONFIG_ROUND, 1, [Define if round is available.])
fi

AC_SUBST(ac_libs)
AC_SUBST(ac_incpath)

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h unistd.h signal.h)

AC_CHECK_HEADER(lscp/client.h, [ac_lscp_h="yes"], [ac_lscp_h="no"])
if test "x$ac_lscp_h" = "xno"; then
   AC_MSG_ERROR([LSCP headers not found.])
fi

# Checks for typedefs, structures, and compiler characteristics.
# AC_C_CONST

# Checks for library functions.
AC_CHECK_FUNCS(system)

# Finally produce a configure header file and the makefiles.
AC_OUTPUT

# make clean > /dev/null 2>&1

echo ""
echo "Now type 'make', followed by 'make install' as root."
echo ""
