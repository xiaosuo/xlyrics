/*  XLyrics by xiaosuo <xiaosuo1@eyou.com>
 *  Homepage gentux.blogchina.com
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef _XLYRICS_H_
#define _XLYRICS_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

extern struct Song *song; /*current song name*/
extern struct LyricsLine *lyrics_line; /*currnet line of lyrics*/

extern GtkWidget *window; /*the main window*/

extern gint width, height; /* the main window width and height*/
extern gint pos_x, pos_y; /*the window position*/
extern GdkColor bg_color; /*back ground color*/
extern GdkColor ac_color; /*activated font color*/
extern GdkColor ua_color; /*unactivated font color*/
extern gchar lyrics_font[32]; /*font used for display the lyrics*/
extern gchar lyrics_dir[255]; /*directory store the lyrics*/
extern gchar plugin_name[255]; /* the plugin name*/
extern gboolean look_in_mp3dir_mode; /*look up in mp3 dir or not*/
extern gboolean is_keep_above; /*make window keep above or not*/
extern gboolean have_border; /*make window have border or not*/
extern gboolean is_fit_width; /*automatic chage width or not*/
extern gboolean hide_not_found; /*hide if not found any lyrics*/
extern guint opacity; /*the window's opacity*/

extern gboolean is_config_update;


extern void redraw_list(void);
extern gchar* locale2utf8(const gchar* data);

#endif
