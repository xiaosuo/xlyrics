SUBDIRS = plugins audaciousplugin scripts
INCLUDES = $(GTK2_CFLAGS) $(DBUS_CFLAGS) 

bin_PROGRAMS = xlyrics

xlyrics_SOURCES = \
	internal.h \
	internal.c \
	conf.h \
	conf.c \
	find.h \
	find.c \
	id3.h \
	id3.c \
	lyrics.h \
	lyrics.c \
	lyrics_creator.h \
	lyrics_creator.c \
	link_editor.h \
	link_editor.c \
	lyrics_download.h \
	lyrics_download.c \
	xlyrics.h \
	xlyrics.c 

xlyrics_LDADD = $(GTK2_LIBS)

xlyricsmenudir = ${prefix}/share/xlyrics
xlyricsmenu_DATA = stylerc
EXTRA_DIST = $(xlyricsmenu_DATA)
