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

#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include <gmodule.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define OPAQUE	0xffffffff

#include "xlyrics.h"
#include "lyrics.h"
#include "find.h"
#include "conf.h"
#include "lyrics_download.h"
#include "internal.h"

struct Song *song = NULL; /*current song name*/
struct LyricsLine *lyrics_line = NULL; /*currnet line of lyrics*/

/*****the configure*/
gint width=390, height=500; /* the main window width and height*/
gint pos_x=200, pos_y=400;  /* the window position*/
GdkColor bg_color = {0, 0, 0, 0}; /*back ground color*/
GdkColor ac_color = {0, 0xffff, 0xffff, 0}; /*activated font color*/
GdkColor ua_color = {0, 0, 0, 0xffff}; /*unactivated font color*/
gchar lyrics_font[32] = "Sans 16"; /*font used for display the lyrics*/
gchar lyrics_dir[255] = ""; /*directory store the lyrics*/
gchar plugin_name[255] = ""; /* the plugin name*/
gboolean look_in_mp3dir_mode = FALSE; /*look up in mp3 dir or not*/
gboolean is_keep_above = FALSE; /*make window keep above or not*/
gboolean have_border = FALSE; /*make window have border or not*/
gboolean is_fit_width = FALSE; /*automatic change the the widget width*/
gboolean hide_not_found = FALSE; /*hide if not found any lyrics*/
guint opacity; /*the window's opacity*/
/*****************/

gboolean is_config_update = FALSE;
gboolean hiding = FALSE;

GtkWidget *window = NULL; /* main window*/
GtkListStore *list_store = NULL; /*the list of the lyrics lines*/
GtkWidget *list_view = NULL; /* used to show the list_store*/
GtkWidget *scrolled_window = NULL; /* the scrolled window */

GtkCellRenderer *cell = NULL; /*the render of the cell*/
gchar last_song[255] = ""; /*the last song name*/
gint last_line_number = 0; /* record the line number*/
gchar lyrics_file[255] = ""; /*current lyrics file name */
char *playfile; /* the current mp3 name */
gchar mp3_dir[255] = ""; /* the mp3 dir path*/
gboolean is_lyrics_loaded = FALSE; /* the lyrics file loaded or not*/

typedef int   (*is_players_on) (void);
typedef int   (*launch_players) (void);
typedef char* (*get_players_song) (int session);
typedef int   (*get_players_time) (int session);
typedef int   (*set_players_time) (int session, int time);

GModule *module; /*the plugin module*/
int timer; /*the timer*/
int session; /*the current player session*/
int is_downloading = 0; /*downloading lyrics*/
is_players_on is_player_on; /*if player on or not*/
launch_players launch_player; /*launch a new player session*/
get_players_song get_player_song; /*get the current song name*/
get_players_time get_player_time; /*get the current song time*/
set_players_time set_player_time; /*set the current song time*/

typedef
struct Position
{
	gdouble x;
	gdouble y;
} Position;
Position mouse_position={0,0};

gboolean is_left_button_pressed = FALSE;
gboolean is_window_resized = FALSE;
void load_lyrics_file(char *playfile_full);

/* set the widget's transparency to opacity
 * opacity is guint 0x00000000-0xffffffff
 */
int my_gtk_widget_set_transparency(GtkWidget *widget, guint opacity)
{
	Display *display;
	Window window;
	Window parent_win;
	Window root_win;
	Window* child_windows;
	int num_child_windows;

	if(!GTK_IS_WIDGET(widget)){
		printf("gtk_widget_set_transparency: not a widget!\n");
		abort();
	}

	/* map the widget */
	gtk_widget_show(widget);
	/* Set the Display and Screen */
	display = (Display*)gdk_x11_get_default_xdisplay();
	/* sync, so the window manager can know the new widget */
	XSync(display, False);
	window = GDK_WINDOW_XWINDOW(widget->window);

	/* Get the cureent window's top-level window */
	while(1){
		XQueryTree(display, window,
				&root_win,
				&parent_win,
				&child_windows, &num_child_windows);
		XFree(child_windows);
		/* found the top-level window */
		if(root_win == parent_win) break;
		window = parent_win;
	}

	if(opacity == OPAQUE){
		XDeleteProperty(display, window, 
				XInternAtom(display, "_KDE_WM_WINDOW_OPACITY", False));
		XDeleteProperty(display, window, 
				XInternAtom(display, "_NET_WM_WINDOW_OPACITY", False));
	}else{
		XChangeProperty(display, window,
				XInternAtom(display, "_KDE_WM_WINDOW_OPACITY", False), 
				XA_CARDINAL, 32, PropModeReplace, 
				(unsigned char *) &opacity, 1L);
		XChangeProperty(display, window,
				XInternAtom(display, "_NET_WM_WINDOW_OPACITY", False), 
				XA_CARDINAL, 32, PropModeReplace, 
				(unsigned char *) &opacity, 1L);
	}

	XSync(display, False);

	return 0;
}

/*exit the lyrics and save the current posion ,width, height*/
void quit(GtkWidget *widget, gpointer data)
{
	update_config("xlyrics");
	gtk_list_store_clear(list_store);
	lyrics_cleanup(song);
	song = NULL;
	g_source_remove(timer);
	g_module_close(module);
	gtk_main_quit();
}

/*keep above or not*/
void keep_above(void)
{
	is_keep_above = !is_keep_above;
	gtk_window_set_keep_above(GTK_WINDOW(window), is_keep_above);
}

/*have border or not*/
void no_border(void)
{
	have_border = !have_border;
	gtk_window_set_decorated(GTK_WINDOW(window), have_border);
}

/*fit width or not*/
void fit_width(void)
{
	is_fit_width = !is_fit_width;
	if(is_fit_width){
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				GTK_POLICY_NEVER, 
				GTK_POLICY_ALWAYS);
	}else{
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				GTK_POLICY_ALWAYS, 
				GTK_POLICY_ALWAYS);
		gtk_window_resize(GTK_WINDOW(window), width, height);
	}
}
/*move or resize the main window with the mouse*/
gboolean move_with_mouse(GtkWidget *widget, GdkEventMotion *event)
{
	gint x, y;

	if(is_left_button_pressed)
	{
		x = pos_x + (event->x_root - mouse_position.x);
		y = pos_y + (event->y_root - mouse_position.y);
		if( x != pos_x || y != pos_y)
			gtk_window_move(GTK_WINDOW(window), x, y);
	}
	if(is_window_resized)
	{
		gtk_window_resize(GTK_WINDOW(window), event->x, event->y);
	}

	return FALSE;
}

/*deal with the left button released*/
gboolean left_button_released(GtkWidget *widget, GdkEventButton *event)
{
	if (event->button == 1)
	{
		is_left_button_pressed = FALSE;
		is_window_resized = FALSE;
		gdk_window_set_cursor(event->window, gdk_cursor_new(GDK_LEFT_PTR));
		gtk_window_get_position(GTK_WINDOW(window), &pos_x, &pos_y);
		gtk_window_get_size(GTK_WINDOW(window), &width, &height);
	}

	return FALSE;	
}

/* manual download current music's lyrics */
void manual_download(void)
{
	if(is_lyrics_loaded)
		unlink(lyrics_file);
	is_downloading = 0;
	load_lyrics_file(get_player_song(session));
};

/*popup menu*/
void popup_menu(GtkWidget *widget, GdkEventButton *event)
{
	GtkWidget *menu;
	GtkWidget *item;
	gint button, event_time;

	menu = gtk_menu_new ();

	/* ... add menu items ... */
	item = gtk_check_menu_item_new_with_label(_("Keep above "));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),
			is_keep_above);
	g_signal_connect(G_OBJECT(item), "toggled",
			G_CALLBACK(keep_above), NULL );	

	item = gtk_check_menu_item_new_with_label(_("No border"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),
			!have_border);
	g_signal_connect(G_OBJECT(item), "toggled",
			G_CALLBACK(no_border), NULL );	

	item = gtk_check_menu_item_new_with_label(_("Auto adjust width"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),
			is_fit_width);
	g_signal_connect(G_OBJECT(item), "toggled",
			G_CALLBACK(fit_width), NULL );	

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_menu_item_new_with_label(_("Download this lyrics"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			G_CALLBACK(manual_download), NULL);	

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_menu_item_new_with_label(_("Interface config"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			G_CALLBACK(config), (gpointer)0);	

	item = gtk_menu_item_new_with_label(_("Generally config"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			G_CALLBACK(config), (gpointer)1);	

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_menu_item_new_with_label(_("About"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			G_CALLBACK(config), (gpointer)2);	

	item = gtk_menu_item_new_with_label(_("Exit"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(G_OBJECT(item), "activate",
			G_CALLBACK(quit), "pop");	
	gtk_widget_show_all(menu);

	if (event)
	{
		button = event->button;
		event_time = event->time;
	}
	else
	{
		button = 0;
		event_time = gtk_get_current_event_time ();
	}

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 
			button, event_time);
}

/* deal withe mouse event*/
gboolean deal_with_mouse(GtkWidget *widget, GdkEventButton *event)
{

	if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
	{
		popup_menu (widget, event);
	}

	if (event->button == 1 && event->type == GDK_BUTTON_PRESS)
	{
		if(event->x < width - 5
				||event->y < height - 5)
		{
			is_left_button_pressed = TRUE;
			gdk_window_set_cursor(event->window, gdk_cursor_new(GDK_FLEUR));
			mouse_position.x = event->x_root;
			mouse_position.y = event->y_root;
			gtk_window_get_position(GTK_WINDOW(window), &pos_x, &pos_y);
		}
		else
		{
			is_window_resized = TRUE;
			gdk_window_set_cursor(event->window, gdk_cursor_new(GDK_BOTTOM_RIGHT_CORNER));
		}
	}

	return FALSE;	
}


/* init the plugin for xlyrics*/
gboolean init_plugin(gchar *plugin_name)
{
	gchar full_plugin_name[1024];
	if(g_module_supported())
	{
		snprintf(full_plugin_name, 1024, "%s/%s", PACKAGE_LIB_DIR, plugin_name);

		module = g_module_open(full_plugin_name, G_MODULE_BIND_LAZY);
		if(module == NULL){
			printf("error while openning module: %s\n", full_plugin_name);
			printf("\t %s\n", g_module_error());
			return FALSE;
		}
		if(g_module_symbol(module, "is_on", (gpointer*)&is_player_on)
				&& g_module_symbol(module, "launch", (gpointer*)&launch_player)
				&& g_module_symbol(module, "get_song", (gpointer*)&get_player_song)
				&& g_module_symbol(module, "get_time", (gpointer*)&get_player_time)
				&& g_module_symbol(module, "set_time", (gpointer*)&set_player_time))
			return TRUE;
		return FALSE;
	}
	else
	{
		printf("gmodule is not supported\n");
		exit(-1);
	}
}

/*set a line activated or not*/
void set_line(gint row, gboolean stat)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	/*if overflow*/
	if(row >= gtk_tree_model_iter_n_children(GTK_TREE_MODEL(list_store), NULL))
		return;

	gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(list_store), &iter, NULL, row);
	gtk_list_store_set(list_store, &iter, 1, stat, -1);

	if(stat == FALSE)
		return;
	/* if active scroll to the row*/
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(list_store), &iter);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(list_view), path, NULL, TRUE, 0.5, 0.5);
	gtk_tree_path_free(path);
}

/*read lyrics and fill the list_store*/
void read_lyrics(char *file_name)
{
	GtkTreeIter iter;

	if(!strcmp(file_name, ""))
		return;

	if(!is_lyrics_loaded)
	{
		gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
		gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
				0, file_name,
				1, FALSE,
				2, 0,
				-1);
		return;
	}

	song = read_lyrics_file(file_name);
	for(lyrics_line = song->head; lyrics_line != NULL; lyrics_line = lyrics_line->next)
	{
		gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
		gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
				0, lyrics_line->buffer,
				1, FALSE,
				2, lyrics_line->line_number,
				-1);
	}
}

/* define the func to render the data*/
void line_color_func(GtkTreeViewColumn *column,
		GtkCellRenderer *cell,
		GtkTreeModel *tree_model,
		GtkTreeIter *iter,
		gpointer data)
{
	gchar *lyrics_line_buffer;
	gint stat;
	gchar *utf8;

	gtk_tree_model_get(tree_model, iter,
			0, &lyrics_line_buffer,
			1, &stat,
			-1);
	if(stat == TRUE)/* active color*/
		g_object_set(cell, "foreground-gdk", &ac_color, 
				"foreground-set", TRUE,
				NULL);
	else
		g_object_set(cell, "foreground-gdk", &ua_color, 
				"foreground-set", TRUE,
				NULL);
	cell->xalign = 0.5;
	utf8 = locale2utf8(lyrics_line_buffer);
	g_object_set(cell, "text", utf8, NULL);
	if(utf8) g_free(utf8);
}

void redraw_list(void)
{
	PangoFontDescription *desc;

	/* set the font*/
	desc = pango_font_description_from_string(lyrics_font);
	gtk_widget_modify_font(list_view, desc);
	/* set the bg color*/
	gtk_widget_modify_base(list_view, GTK_STATE_NORMAL, &bg_color);
	gtk_widget_modify_base(list_view, GTK_STATE_SELECTED, &bg_color);
	gtk_widget_modify_base(list_view, GTK_STATE_ACTIVE, &bg_color);
	g_object_set(cell, "background-gdk", &bg_color,
			"background-set", TRUE,
			NULL);
	gtk_widget_modify_bg(GTK_SCROLLED_WINDOW(scrolled_window)->vscrollbar,
			GTK_STATE_ACTIVE, &bg_color);
	/* set transparency */
	if(!hiding) my_gtk_widget_set_transparency(window, opacity);
	/* cleanup the list_store and lyrics*/
	gtk_list_store_clear(list_store);
	lyrics_cleanup(song);
	song = NULL;
	/* reload the lyrics */
	if(is_fit_width)
		gtk_window_resize(GTK_WINDOW(window), 10, height);
	
	read_lyrics(lyrics_file);
	set_line(last_line_number, TRUE);
}

/*set the player time according the line*/
void set_line_time(GtkTreeView *treeview,
		GtkTreePath *path,
		GtkTreeViewColumn *col,
		gpointer user_data)
{
	GtkTreeIter iter;
	gint line_number;
	struct LyricsLine *line = NULL;

	if(song && gtk_tree_model_get_iter(GTK_TREE_MODEL(list_store), &iter, path))
	{
		gtk_tree_model_get(GTK_TREE_MODEL(list_store), &iter, 2, &line_number, -1);
		for(line = song->head; line != NULL; line = line->next)
			if(line->line_number == line_number)
				break;
		if((session = is_player_on()) >= 0 && line != NULL)
			set_player_time(session, line->line_time);
	}
}
/*create a blank list */
GtkWidget *create_list( void )
{
	GtkTreeViewColumn *column;

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	if(is_fit_width){
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				GTK_POLICY_NEVER, 
				GTK_POLICY_ALWAYS);
	}else{
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				GTK_POLICY_ALWAYS, 
				GTK_POLICY_ALWAYS);
	}
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window),
			GTK_SHADOW_NONE);

	list_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_INT);
	list_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL(list_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list_view), FALSE);
	g_signal_connect(G_OBJECT(list_view), "button-press-event",
			G_CALLBACK(deal_with_mouse), NULL);
	g_signal_connect(G_OBJECT(list_view), "button-release-event",
			G_CALLBACK(left_button_released), NULL);
	g_signal_connect(G_OBJECT(list_view), "row-activated",
			G_CALLBACK(set_line_time), NULL);
	g_signal_connect(G_OBJECT(list_view), "motion_notify_event",
			G_CALLBACK(move_with_mouse), NULL);

	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, "Xlyrics");
	gtk_tree_view_append_column (GTK_TREE_VIEW (list_view),
			GTK_TREE_VIEW_COLUMN (column));
	gtk_tree_view_column_pack_start(column, cell, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, cell,
			line_color_func,
			NULL, NULL);

	gtk_container_add(GTK_CONTAINER(scrolled_window), list_view);
	gtk_widget_show_all(scrolled_window);
	return scrolled_window;
}


/* confirm exit*/
gint confirm(void)
{
	gint response;
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_OK_CANCEL,
			_("You will close the XLyrics \n Are you sure?"));

	gtk_widget_show_all (dialog);
	response = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(response)
	{
		case GTK_RESPONSE_OK:
			return FALSE;
		default:
			gtk_widget_destroy(dialog);
			return TRUE;
	}
}


/* convert ascii generated by IE,Netscape */
void conv_ascii_to_nomal(char *s)  
{
	char num[3];
	int n;

	if(!s) return;
	num[2]='\0';
	for(;*s;s++)
	{
		if(*s=='+') *s=' ';
		else if(*s=='%') {
			num[0]=*(s+1);
			num[1]=*(s+2);
			sscanf(num,"%x",&n);
			*s=n;
			strcpy(s+1,s+3);
		}
	}
}

/*load lyrics file*/
void load_lyrics_file(char *playfile_full)
{
	char *lyricsfile = NULL;
	char *filename_begin;

	strcpy(last_song, get_player_song(session));
	gtk_list_store_clear(list_store);
	lyrics_cleanup(song);
	song = NULL;

	conv_ascii_to_nomal(playfile_full);
	filename_begin = rindex(playfile_full, '/'); /* get the last "/" */
	if(filename_begin)
	{
		playfile = filename_begin + 1; /* now the playfile point to the file name*/
		int n = strlen(playfile_full);
		n -= strlen(playfile);
		strncpy(mp3_dir, playfile_full, n);
		mp3_dir[n] = '\0'; /*get the mp3 directory*/
	}
	else
	{
		strcpy(mp3_dir, "./");
		playfile = playfile_full; /*set the mp3 directory is current direcrory*/
	}

	/*chop space*/
	while(*playfile == ' ' || *playfile == '\t') playfile ++;

	if(strlen(playfile) != 0)
	{
		/* first try to find file in cache, this will speed up the load*/
		lyricsfile = find_lyrics_from_cache(playfile);
		if(!lyricsfile)/*find the file in directory*/
		{
			int i;
			gchar buf[1024];
			for(i=0; i<2; i++)
			{
				if(look_in_mp3dir_mode
						&& (lyricsfile = find_file_in_dir(mp3_dir, playfile, playfile_full, i)))
					break;
				else if((lyricsfile = find_file_in_dir(lyrics_dir, playfile, playfile_full, i)))
					break;
			}

#if 1 /* if you think this is not good enough, just remove it */
			if(!lyricsfile && !is_downloading)
			{ /* no lyrics is found in the local directories, try to download it */
				char *tmp, *ptr;
				tmp = (char*)strdup(playfile);
				ptr = strrchr(tmp, '.');
				if(ptr) *ptr = '\0';
				snprintf(buf, 1024, "%s/%s.lrc", lyrics_dir, tmp);
				ptr = locale2gb2312(tmp);
				g_free(tmp);
				if(ptr){
					if(lyrics_download(ptr, buf) == 0)
						is_downloading = 1;
					g_free(ptr);
				}
			}else if(is_downloading)
				is_downloading =  0;
#endif

			if(lyricsfile)
				add_item_to_cache(playfile, lyricsfile);
		}
	}

	if(lyricsfile)/* found the lyrics file*/
	{
		if(hiding)
		{
			hiding = FALSE;
			gtk_widget_show_all(window);
			gtk_window_move(GTK_WINDOW(window), pos_x, pos_y);
			if(is_keep_above)
				gtk_window_set_keep_above(GTK_WINDOW(window), is_keep_above);
		}
		strcpy(lyrics_file, lyricsfile);
		is_lyrics_loaded = TRUE;
		redraw_list(); 
		g_free(lyricsfile);
	}
	else /* not find the lyrics file*/
	{
		if(hide_not_found)
		{
			hiding = TRUE;
			gtk_widget_hide_all(window);
		}

		if(strlen(playfile) == 0)
			strcpy(lyrics_file, "NULL file name");
		else
		{
			lyricsfile = (char*)remove_ext(playfile);
			strcpy(lyrics_file, lyricsfile);
			g_free(lyricsfile);
		}
		is_lyrics_loaded = FALSE;
		redraw_list();
	}
}

/*timeout function*/
gboolean timeout(gpointer data)
{
	static char *curr_song = NULL;

	if(is_config_update == TRUE)
	{
		is_config_update = FALSE;
		update_config("xlyrics");
		redraw_list();
	}

	if((session = is_player_on()) < 0)
	{
		quit(window, "nop");
		return FALSE;
	}

	if((curr_song = get_player_song(session)) == NULL)
	{
		/* no song is in playlist*/
		return TRUE;
	}

	if(strcmp(last_song, curr_song) != 0)
	{	/* FIXME: the downloading process may be running,
		 * if then, this action will make a mistake
		 */
		is_downloading = 0;
		load_lyrics_file(curr_song);
		if(is_lyrics_loaded)
		{
			gchar *utf8;
			set_line(last_line_number, FALSE);
			last_line_number = 0;
			set_line(last_line_number, TRUE);
			if(song->head)
			{
				utf8 = locale2utf8(song->head->buffer);
				gtk_window_set_title(GTK_WINDOW(window), utf8);
				g_free(utf8);
			}
			else
			{/* the lyrics file is not a right one */
				utf8 = locale2utf8(playfile);
				gtk_window_set_title(GTK_WINDOW(window), utf8);
				g_free(utf8);
			}
		}
	}
	
	if(is_downloading)
	{/* check the downloading state */
		switch(get_download_state()){
			case 1:
				return TRUE;
			case -1:
				is_downloading = 0;
				break;
			default:
				load_lyrics_file(curr_song);
				break;
		}
	}

	if(is_lyrics_loaded)
	{
		static int time;
		time = get_player_time(session);

		lyrics_line = song->head;
		while (lyrics_line)
		{
			if (lyrics_line->line_time == time)
				break;
			lyrics_line = lyrics_line->next;
		}
		if(lyrics_line &&
				(last_line_number != lyrics_line->line_number || last_line_number == 0))
		{
			gchar *utf8;
			set_line(last_line_number, FALSE);
			last_line_number = lyrics_line->line_number;
			utf8 = locale2utf8(lyrics_line->buffer);
			gtk_window_set_title(GTK_WINDOW(window), utf8);
			g_free(utf8);
			set_line(last_line_number, TRUE);
		}
	}else
		gtk_window_set_title(GTK_WINDOW(window), "Xlyrics");

	return TRUE;
}

int main(int argc, char* argv[])
{
	const gchar *lang;
	gchar stylerc[1024];

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	init_config("xlyrics");
	if(argc > 1)
		strncpy(plugin_name, argv[1], 255);

	/*first run or initial plugin failed*/
	if(!strcmp(plugin_name, "") || !init_plugin(plugin_name)) 
	{
		gtk_init(&argc, &argv);
		config(NULL, (gpointer)-1);
		gtk_main();
		return 0;
	}

	if((session = is_player_on()) < 0)
		session = launch_player();

	gtk_set_locale();

	snprintf(stylerc, 1024, "%s/stylerc", PACKAGE_MENU_DIR);
	gtk_rc_parse(stylerc);/*get style from stylerc file*/
	gtk_init(&argc, &argv);


	/* create the main window*/
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_name(window, "main window");
	gtk_window_set_title(GTK_WINDOW(window), "XLyrics");
	gtk_window_set_default_size(GTK_WINDOW(window), width, height);
	gtk_window_move(GTK_WINDOW(window), pos_x, pos_y);
	gtk_window_set_decorated(GTK_WINDOW(window), have_border);
	gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
	gtk_window_set_keep_above(GTK_WINDOW(window), is_keep_above);
	g_signal_connect(G_OBJECT(window), "delete_event",
			G_CALLBACK(confirm), NULL);
	g_signal_connect(G_OBJECT(window), "destroy",
			G_CALLBACK(quit), "win");

	/* add the lyrics list*/
	gtk_container_add(GTK_CONTAINER(window), create_list());

	timer = g_timeout_add(300, &timeout, NULL);

	gtk_widget_show(window);
	my_gtk_widget_set_transparency(window, opacity);
	gtk_main();

	return 0;
}
