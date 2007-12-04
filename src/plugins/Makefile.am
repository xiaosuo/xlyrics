##makefile.am by xiaosuo<xiaosuo@mail.nankai.edu.cn>
##chose the plugin to complie

if HAVE_AUDACIOUS
tAUDACIOUS = liblyrics_audacious.la
else
tAUDACIOUS =
endif
if HAVE_AMAROK
tAMAROK = liblyrics_amarok.la
else
tAMAROK =
endif

lib_LTLIBRARIES  = ${tAUDACIOUS} ${tAMAROK}

##configure the audacious media player options
liblyrics_audacious_la_SOURCES = lyrics_audacious.c
liblyrics_audacious_la_LIBADD = ${AUDACIOUS_LIBS} ${DBUS_LIBS}
liblyrics_audacious_la_CFLAGS = ${AUDACIOUS_CFLAGS} ${DBUS_CFLAGS}
liblyrics_audacious_la_LDFLAGS = -module -avoid-version -nostdlib

##configure the amarok media player options
liblyrics_amarok_la_SOURCES = lyrics_amarok.cpp
liblyrics_amarok_la_LIBADD = ${KDE_LDFLAGS} ${QT_LDFLAGS} -lkdecore
liblyrics_amarok_la_CXXFLAGS = ${KDE_CXXFLAGS} ${QT_CXXFLAGS}
liblyrics_amarok_la_LDFLAGS = -module -avoid-version -nostdlib

libdir = ${prefix}/lib/xlyrics
