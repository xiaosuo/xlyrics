
# chose the plugin to complie
if HAVE_AUDACIOUS
lib_LTLIBRARIES  = libaudacious_xlyrics.la
endif

# configure the audacious options
libaudacious_xlyrics_la_SOURCES = audacious_xlyrics.c
libaudacious_xlyrics_la_LIBADD = ${AUDACIOUS_LIBS} ${DBUS_LIBS}
libaudacious_xlyrics_la_CFLAGS = ${AUDACIOUS_CFLAGS} ${DBUS_CFLAGS}
libaudacious_xlyrics_la_LDFLAGS = -module -avoid-version  -nostdlib

libdir = ${AUDACIOUS_GENERAL_PLUGIN_DIR}
