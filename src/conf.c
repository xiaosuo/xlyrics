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

#include<string.h>
#include<gtk/gtk.h>
#include<gdk/gdk.h>
#include<glib.h>
#include<stdio.h>
#include<dirent.h>
#include"xlyrics.h"
#include"conf.h"
#include"internal.h"

GtkWidget* conf_win;
GtkWidget* color_win;
GtkWidget* font_win;
GtkWidget* file_win;
GtkWidget* bg_entry;
GtkWidget* ac_entry;
GtkWidget* ua_entry;

GSList* read_config(gchar *config_name)
{
	FILE *conf_file;
	GSList *list = NULL;
	ConfItem *item = NULL;
	gchar full_name[255];
	gchar *in_index;
	gchar *in_value;
	gchar *iter;
	gchar buffer[255];

	if( config_name == NULL)
		return NULL;

	snprintf(full_name, 255, "%s/.%s", g_get_home_dir(), config_name);

	if((conf_file = fopen( full_name, "r")) == NULL)
		return NULL;

	while(1)
	{
		if(!fgets(buffer, 255, conf_file))
		{
			fclose(conf_file);
			return list;
		}

		buffer[254] = 0;
		iter = buffer + strlen(buffer) - 1;
		*iter = 0;
		
		if((iter = strstr(buffer, " = ")))
		{
			*iter = 0;
			in_index = buffer;
			in_value = iter+3;
		}
		else
		{
			in_index = buffer;
			in_value = buffer;
		}

		item = g_new(ConfItem, 1);
		item->index = (gchar*)strdup(in_index);
		item->value = (gchar*)strdup(in_value);
		list = g_slist_append(list, item);
	}
}

gboolean write_config(gchar *config_name, GSList *config_list)
{
	FILE *conf_file;
	ConfItem *item;
	GSList *list;
	gchar full_name[255];

	if( config_name == NULL || config_list == NULL)
		return FALSE;

	snprintf(full_name, 255, "%s/.%s", g_get_home_dir(), config_name);

	if((conf_file = fopen( full_name, "w+")) == NULL)
		return FALSE;

	for(list=config_list; list; list=list->next)
	{
		item = (ConfItem*)(list->data);
		fprintf(conf_file, "%s = %s\n",
				item->index, item->value);
	}

	fclose(conf_file);
	return TRUE;
}

gboolean free_config(GSList *config_list)
{
	GSList *list;
	ConfItem *item;

	if(!config_list)
		return FALSE;

	list = config_list;

	while(list)
	{
		item = (ConfItem*)(list->data);
		g_free(item->index);
		g_free(item->value);
		g_free(item);
		list = list->next;
	}

	g_slist_free(config_list);
	return TRUE;

}
gboolean get_config_str(GSList *config_list, gchar* index, gchar* value)
{
	GSList *list;
	ConfItem *item;

	for(list=config_list; list; list=list->next)
	{
		item = (ConfItem*)(list->data);
		if(!strcmp(index, item->index))
		{
			strcpy(value, item->value);
			return TRUE;
		}
	}

	return FALSE;
}

gboolean set_config_str(GSList** config_list, gchar* index, gchar* value)
{
	GSList *list;
	ConfItem *item;

	for(list=*config_list; list; list=list->next)
	{
		item = (ConfItem*)(list->data);
		if(!strcmp(index, item->index))
		{
			g_free(item->value);
			item->value = g_strdup(value);
			return TRUE;
		}
	}

	item = g_new(ConfItem, 1);
	item->index = (gchar*)strdup(index);
	item->value = (gchar*)strdup(value);
	*config_list = g_slist_append(*config_list, item);

	return FALSE;
}

gboolean get_config_int(GSList *config_list, gchar* index, gint* value)
{
	GSList *list;
	ConfItem *item;

	for(list=config_list; list; list=list->next)
	{
		item = (ConfItem*)(list->data);
		if(!strcmp(index, item->index))
		{
			*value = atoi(item->value);
			return TRUE;
		}
	}

	return FALSE;
}

gboolean set_config_int(GSList** config_list, gchar* index, gint* value)
{
	GSList *list;
	ConfItem *item;

	for(list=*config_list; list; list=list->next)
	{
		item = (ConfItem*)(list->data);
		if(!strcmp(index, item->index))
		{
			g_free(item->value);
			item->value = g_strdup_printf("%d", *value);
			return TRUE;
		}
	}

	item = g_new(ConfItem, 1);
	item->index = g_strdup(index);
	item->value = g_strdup_printf("%d", *value);
	*config_list = g_slist_append(*config_list, item);

	return FALSE;
}

gboolean init_config(gchar *config_name)
{
	GSList *list;

	list = read_config(config_name);

	/*have the lyrics file*/
	if(list)
	{
		get_config_int(list, "width", &width);
		get_config_int(list, "height", &height);
		get_config_int(list, "pos_x", &pos_x);
		get_config_int(list, "pos_y", &pos_y);

		get_config_int(list, "look_in_mp3dir_mode", &look_in_mp3dir_mode);
		get_config_int(list, "is_keep_above", &is_keep_above);
		get_config_int(list, "have_border", &have_border);
		get_config_int(list, "is_fit_width", &is_fit_width);
		get_config_int(list, "hide_not_found", &hide_not_found);

		get_config_str(list, "lyrics_font", lyrics_font);
		get_config_str(list, "lyrics_dir", lyrics_dir);
		get_config_str(list, "plugin_name", plugin_name);

		get_config_int(list, "opacity",(gint*)&(opacity));

		get_config_int(list, "bg_color.pixel",(gint*)&(bg_color.pixel));
		get_config_int(list, "bg_color.red", (gint*)&(bg_color.red));
		get_config_int(list, "bg_color.green", (gint*)&(bg_color.green));
		get_config_int(list, "bg_color.blue", (gint*)&(bg_color.blue));

		get_config_int(list, "ac_color.pixel", (gint*)&(ac_color.pixel));
		get_config_int(list, "ac_color.red", (gint*)&(ac_color.red));
		get_config_int(list, "ac_color.green", (gint*)&(ac_color.green));
		get_config_int(list, "ac_color.blue", (gint*)&(ac_color.blue));


		get_config_int(list, "ua_color.pixel", (gint*)&(ua_color.pixel));
		get_config_int(list, "ua_color.red", (gint*)&(ua_color.red));
		get_config_int(list, "ua_color.green", (gint*)&(ua_color.green));
		get_config_int(list, "ua_color.blue", (gint*)&(ua_color.blue));
	}
	/* else use the default*/

	free_config(list);
}

gboolean update_config(gchar *config_name)
{
	GSList *list = NULL;

	set_config_int(&list, "width", &width);
	set_config_int(&list, "height", &height);
	set_config_int(&list, "pos_x", &pos_x);
	set_config_int(&list, "pos_y", &pos_y);

	set_config_int(&list, "look_in_mp3dir_mode", &look_in_mp3dir_mode);
	set_config_int(&list, "is_keep_above", &is_keep_above);
	set_config_int(&list, "have_border", &have_border);
	set_config_int(&list, "is_fit_width", &is_fit_width);
	set_config_int(&list, "hide_not_found", &hide_not_found);

	set_config_str(&list, "lyrics_font", lyrics_font);
	set_config_str(&list, "lyrics_dir", lyrics_dir);
	set_config_str(&list, "plugin_name", plugin_name);

	set_config_int(&list, "opacity",(gint*)&(opacity));

	set_config_int(&list, "bg_color.pixel",(gint*)&(bg_color.pixel));
	set_config_int(&list, "bg_color.red", (gint*)&(bg_color.red));
	set_config_int(&list, "bg_color.green", (gint*)&(bg_color.green));
	set_config_int(&list, "bg_color.blue", (gint*)&(bg_color.blue));

	set_config_int(&list, "ac_color.pixel", (gint*)&(ac_color.pixel));
	set_config_int(&list, "ac_color.red", (gint*)&(ac_color.red));
	set_config_int(&list, "ac_color.green", (gint*)&(ac_color.green));
	set_config_int(&list, "ac_color.blue", (gint*)&(ac_color.blue));


	set_config_int(&list, "ua_color.pixel", (gint*)&(ua_color.pixel));
	set_config_int(&list, "ua_color.red", (gint*)&(ua_color.red));
	set_config_int(&list, "ua_color.green", (gint*)&(ua_color.green));
	set_config_int(&list, "ua_color.blue", (gint*)&(ua_color.blue));

	write_config(config_name, list);
	free_config(list);
}
/* the interface design*/
GtkWidget* about(void)
{
	GtkWidget *about;

	about = gtk_label_new(NULL);
	gtk_label_set_justify(GTK_LABEL(about), GTK_JUSTIFY_CENTER);
	gtk_label_set_markup(GTK_LABEL(about),
			_("<span foreground=\"red\"><big>Xlyrics</big></span>\n Xlyrics is a software \n used for display the lyrics for XMMS\n and Beep Media Player, etc.\n\n Author: xiaosuo (xiaosuo1@eyou.com)\n Homepage: gentux.blogchina.com"));
	return about;
}

void set_color(GtkWidget *widget, gpointer option)
{
	if((gchar*)option == NULL)
	{
		gtk_widget_destroy(GTK_WIDGET(color_win));
		return;
	}
	else if(!strcmp((gchar*)option, "bg"))
	{
		gtk_color_selection_get_current_color(
				GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(color_win)->colorsel), &bg_color);
		gtk_widget_modify_base(bg_entry, GTK_STATE_NORMAL, &bg_color);
	}
	else if(!strcmp((gchar*)option, "ac"))
	{
		gtk_color_selection_get_current_color(
				GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(color_win)->colorsel), &ac_color);
		gtk_widget_modify_base(ac_entry, GTK_STATE_NORMAL, &ac_color);
	}
	else if(!strcmp((gchar*)option, "ua"))
	{
		gtk_color_selection_get_current_color(
				GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(color_win)->colorsel), &ua_color);
		gtk_widget_modify_base(ua_entry, GTK_STATE_NORMAL, &ua_color);
	}
	is_config_update = TRUE;

	gtk_widget_destroy(GTK_WIDGET(color_win));
}
void sel_color(GtkWidget *widget, GdkEvent *event, gpointer option)
{
	gchar* title;

	if(!strcmp((gchar*)option, "bg"))
		title = g_strdup(_("Change background color"));
	else if(!strcmp((gchar*)option, "ac"))
		title = g_strdup(_("Change activated font color"));
	else if(!strcmp((gchar*)option, "ua"))
		title = g_strdup(_("Change unactivated font color"));
	else
		return;

	color_win = gtk_color_selection_dialog_new(title);
	if(!strcmp((gchar*)option, "bg"))
		gtk_color_selection_set_current_color(
			GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(color_win)->colorsel), &bg_color);
	else if(!strcmp((gchar*)option, "ac"))
		gtk_color_selection_set_current_color(
			GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(color_win)->colorsel), &ac_color);
	else if(!strcmp((gchar*)option, "ua"))
		gtk_color_selection_set_current_color(
			GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(color_win)->colorsel), &ua_color);
	gtk_window_set_modal(GTK_WINDOW(color_win), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(color_win), GTK_WINDOW(conf_win));

	g_signal_connect(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(color_win)->ok_button),
			"clicked", G_CALLBACK(set_color),
			(gpointer)option);
	g_signal_connect(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(color_win)->cancel_button),
			"clicked", G_CALLBACK(set_color),
			NULL);
	g_signal_connect(G_OBJECT(color_win), "destroy",
			G_CALLBACK(gtk_widget_destroy), (gpointer)color_win);
	gtk_widget_show_all(color_win);
	g_free(title);
}

void apply_font(GtkWidget *widget, gpointer option)
{
	strcpy(lyrics_font, gtk_font_selection_dialog_get_font_name(
				GTK_FONT_SELECTION_DIALOG(font_win)));
	is_config_update = TRUE;
}
void set_font(GtkWidget *widget, gpointer option)
{
	if((gchar*)option != NULL)
	{
		apply_font(NULL, NULL);
	}
	gtk_widget_destroy(font_win);
}
void sel_font(GtkWidget *widget, GdkEvent *event, gpointer option)
{
	font_win = gtk_font_selection_dialog_new(_("Change the font"));
	gtk_window_set_modal(GTK_WINDOW(font_win), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(font_win), GTK_WINDOW(conf_win));
	gtk_font_selection_set_font_name(
			GTK_FONT_SELECTION(GTK_FONT_SELECTION_DIALOG(font_win)->fontsel), lyrics_font);

	g_signal_connect(G_OBJECT(GTK_FONT_SELECTION_DIALOG(font_win)->ok_button),
			"clicked", G_CALLBACK(set_font),
			(gpointer)1);
	g_signal_connect(G_OBJECT(GTK_FONT_SELECTION_DIALOG(font_win)->apply_button),
			"clicked", G_CALLBACK(apply_font),
			NULL);
	g_signal_connect(G_OBJECT(GTK_FONT_SELECTION_DIALOG(font_win)->cancel_button),
			"clicked", G_CALLBACK(set_font), NULL);
	g_signal_connect(G_OBJECT(font_win), "destroy",
			G_CALLBACK(gtk_widget_destroy), font_win);

	gtk_widget_show_all(font_win);
}

/*
void set_file(GtkWidget *widget, gpointer data) 
{
	const gchar *file;

	file = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_win));
	if(g_file_test(file, G_FILE_TEST_IS_DIR))
		return;
	g_strlcpy(plugin_name, file, 255);
	is_config_update = TRUE;
	if(data != NULL && file != NULL)
		gtk_entry_set_text(GTK_ENTRY(data), (gchar*)locale2utf8(file));
	gtk_widget_destroy(file_win);
}

void sel_file(GtkWidget *widget, gpointer data) 
{

	file_win = gtk_file_selection_new ("Please select a file.");
	gtk_window_set_modal(GTK_WINDOW(file_win), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(file_win), GTK_WINDOW(conf_win));

	g_signal_connect (GTK_FILE_SELECTION (file_win)->ok_button,
			"clicked",
			G_CALLBACK (set_file),
			data);
	g_signal_connect_swapped (GTK_FILE_SELECTION (file_win)->cancel_button,
			"clicked",
			G_CALLBACK (gtk_widget_destroy),
			file_win); 

	gtk_widget_show_all (file_win);
}	
*/

void set_dir(GtkWidget *widget, gpointer data) 
{
	const gchar *file, *utf8;

	file = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_win));
	if(!g_file_test(file, G_FILE_TEST_IS_DIR))
		return;
	g_strlcpy(lyrics_dir, file, 255);
	is_config_update = TRUE;
	if(data != NULL && file != NULL){
		utf8 = locale2utf8(file);
		gtk_entry_set_text(GTK_ENTRY(data), utf8);
		g_free(utf8);
	}
	gtk_widget_destroy(file_win);
}

void sel_dir(GtkWidget *widget, gpointer data) 
{

	file_win = gtk_file_selection_new (_("Please select a dir."));
	gtk_window_set_modal(GTK_WINDOW(file_win), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(file_win), GTK_WINDOW(conf_win));
	g_signal_connect (GTK_FILE_SELECTION (file_win)->ok_button,
			"clicked",
			G_CALLBACK (set_dir),
			data);
	g_signal_connect_swapped (GTK_FILE_SELECTION (file_win)->cancel_button,
			"clicked",
			G_CALLBACK (gtk_widget_destroy),
			file_win); 

	gtk_widget_show_all (file_win);
}	

void selected_plugin(GtkComboBox *combo_box, gpointer data)
{
	g_strlcpy(plugin_name, gtk_entry_get_text (GTK_ENTRY (GTK_BIN (combo_box)->child)), 255);
	is_config_update = TRUE;
}

GtkWidget* create_combo_box(void)
{
	GtkWidget *combo_box;
	DIR *dir;
	struct dirent *dirents;

	combo_box = gtk_combo_box_entry_new_text();
	gtk_entry_set_editable(GTK_ENTRY(GTK_BIN(combo_box)->child), FALSE);
	gtk_entry_set_text(GTK_ENTRY(GTK_BIN(combo_box)->child), plugin_name); 
	g_signal_connect(G_OBJECT(combo_box), "changed",
			G_CALLBACK(selected_plugin), NULL);

	dir = opendir(PACKAGE_LIB_DIR);
	if(dir)
	{
		while(dirents = readdir(dir))
		{
			if(strstr(dirents->d_name, ".so"))
					gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), dirents->d_name);
		}
		closedir(dir);
	}
	
	gtk_widget_show_all(combo_box);
	return combo_box;
}

void update_checkbox(GtkWidget *widget, gpointer data)
{
	if(GTK_TOGGLE_BUTTON(widget)->active)
        *(gint *)data = 1;
    else
        *(gint *)data = 0;
	is_config_update = TRUE;
}

void set_opacity(GtkAdjustment *adj, gpointer data)
{
	opacity = gtk_adjustment_get_value(adj);
	is_config_update = TRUE;
}

GtkWidget* interface(void)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *checkbox;
	GtkObject *adj;
	GtkWidget *scale;

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	hbox = gtk_hbox_new(FALSE, 5);
	label = gtk_label_new(_("Background Color:"));
	bg_entry = gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(bg_entry), FALSE);
	gtk_widget_modify_base(bg_entry, GTK_STATE_NORMAL, &bg_color);
	gtk_widget_set_size_request(bg_entry, 50, -1);
	gtk_widget_set_events(bg_entry, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(G_OBJECT(bg_entry), "button-press-event",
			G_CALLBACK(sel_color), (gpointer)"bg");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(hbox), bg_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 5);
	label = gtk_label_new(_("Activated Font Color:"));
	ac_entry = gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(ac_entry), FALSE);
	gtk_widget_modify_base(ac_entry, GTK_STATE_NORMAL, &ac_color);
	gtk_widget_set_size_request(ac_entry, 50, -1);
	gtk_widget_set_events(ac_entry, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(G_OBJECT(ac_entry), "button-press-event",
			G_CALLBACK(sel_color), (gpointer)"ac");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(hbox), ac_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 5);
	label = gtk_label_new(_("Unactivated Font Color:"));
	ua_entry = gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(ua_entry), FALSE);
	gtk_widget_modify_base(ua_entry, GTK_STATE_NORMAL, &ua_color);
	gtk_widget_set_size_request(ua_entry, 50, -1);
	gtk_widget_set_events(ua_entry, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(G_OBJECT(ua_entry), "button-press-event",
			G_CALLBACK(sel_color), (gpointer)"ua");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(hbox), ua_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 5);
	label = gtk_label_new(_("Main Font:"));
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), "aA");
	gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
	gtk_widget_set_size_request(entry, 50, -1);
	gtk_widget_set_events(entry, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(G_OBJECT(entry), "button-press-event",
			G_CALLBACK(sel_font), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 5);
	label = gtk_label_new(_("Opacity:"));
	adj = gtk_adjustment_new(opacity, 0, 4294967295.0, 10, 100, 100);
	scale = gtk_hscale_new(GTK_ADJUSTMENT(adj));
	gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
	g_signal_connect(G_OBJECT(adj), "value-changed",
			G_CALLBACK(set_opacity), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(hbox), scale, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	gtk_widget_show_all(vbox);
	return  vbox;
}

GtkWidget* common(void)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *entry;
	GtkWidget *checkbox;
	GtkWidget *combo_box;
	gchar *utf8;

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	label = gtk_label_new(_("Set plugin file(need restart):"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
	combo_box = create_combo_box();
	gtk_box_pack_start(GTK_BOX(vbox), label , TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), combo_box, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 5);
	label = gtk_label_new(_("Set lyrics directory:"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
	entry = gtk_entry_new();
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	utf8 = locale2utf8(lyrics_dir);
	gtk_entry_set_text(GTK_ENTRY(entry), utf8);
	g_free(utf8);
	button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(button),
			gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_BUTTON));
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(sel_dir), entry);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label , TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	checkbox = gtk_check_button_new_with_label(_("Look for lyrics in mp3 directory"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), 
			look_in_mp3dir_mode);
	g_signal_connect(G_OBJECT(checkbox), "toggled",
			G_CALLBACK(update_checkbox), (gpointer)&look_in_mp3dir_mode);
	gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 0);

	checkbox = gtk_check_button_new_with_label(_("hide widow if lyrics was't found"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), 
			hide_not_found);
	g_signal_connect(G_OBJECT(checkbox), "toggled",
			G_CALLBACK(update_checkbox), (gpointer)&hide_not_found);
	gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 0);

	gtk_widget_show_all(vbox);
	return vbox;
}
void save_and_quit(GtkWidget *widget, gpointer data)
{
	update_config("xlyrics");
	gtk_main_quit();
}
void config(GtkWidget *widget, gpointer layer)
{
	GtkWidget *notebook;
	GtkWidget *tabs;
	GtkWidget *frame;

	conf_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(conf_win), FALSE);
	gtk_window_set_position(GTK_WINDOW(conf_win), GTK_WIN_POS_CENTER);
	if((gint)layer == -1) /*first run*/
		g_signal_connect(G_OBJECT(conf_win), "delete_event",
				G_CALLBACK(save_and_quit), (gpointer)conf_win);
	else
		g_signal_connect(G_OBJECT(conf_win), "delete_event",
				G_CALLBACK(gtk_widget_destroy), (gpointer)conf_win);

	if(window)
	{
		gtk_window_set_modal(GTK_WINDOW(conf_win), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(conf_win), GTK_WINDOW(window));
	}

	notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_LEFT);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), TRUE);

	tabs = gtk_label_new(_("Interface"));
	frame = gtk_frame_new(_("Interface"));
	gtk_widget_show(tabs);
	gtk_widget_show(frame);
	gtk_container_add(GTK_CONTAINER(frame), interface());
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame, tabs);

	tabs = gtk_label_new(_("Common"));
	frame = gtk_frame_new(_("Common"));
	gtk_widget_show(tabs);
	gtk_widget_show(frame);
	gtk_container_add(GTK_CONTAINER(frame), common());
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame, tabs);

	tabs = gtk_label_new(_("About"));
	frame = gtk_frame_new(_("About"));
	gtk_widget_show(tabs);
	gtk_widget_show(frame);
	gtk_container_add(GTK_CONTAINER(frame), about());
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame, tabs);

	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), (gint)layer);
	gtk_container_add(GTK_CONTAINER(conf_win), notebook);
	gtk_widget_show_all(conf_win);
};

