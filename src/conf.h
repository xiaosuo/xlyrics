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

#ifndef _CONF_H_
#define _CONF_H_

typedef
struct ConfItem
{
	gchar *index;
	gchar *value;
} ConfItem;

extern gboolean init_config(gchar *config_name);
extern gboolean update_config(gchar *config_name);

GSList* read_config(gchar *config_name);
gboolean write_config(gchar *config_name, GSList *config_list);
gboolean free_config(GSList *config_list);
gboolean get_config_str(GSList *config_list, gchar* index, gchar* value);
gboolean get_config_int(GSList *config_list, gchar* index, gint* value);
gboolean set_config_str(GSList** config_list, gchar* index, gchar* value);
gboolean set_config_int(GSList** config_list, gchar* index, gint* value);

GtkWidget* about(void);
GtkWidget* interface(void);
GtkWidget* common(void);

void config(GtkWidget *widget, gpointer layer);
#endif
