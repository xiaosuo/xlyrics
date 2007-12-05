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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "lyrics_download.h"
#include "internal.h"

static GtkListStore *store; /* the lyrics files list store */
static GtkWidget *tree; /* the tree view of the list store */
static gchar *resrc = NULL; /* the music name */
static gchar *redes = NULL; /* the lyrics filename to be saved */
static GtkWidget *download_window;
static int pid = 0; /* the pid of the script */
static int tpid = 0; /* the pid of the script */
static int wait_user  = 0; /* wait user to choose */
static int final = 0; /* the final download */

/* get the content after " - "*/
/* the return var must be freed*/
char *extract_title2(const char *s)
{
	char *c = NULL, *ss;

	ss = (char*)strdup(s);
	if((c = strrchr(ss, '-')) && *(c+1) == ' ' && *(c+2))
	{
		strncpy(ss, c+2, strlen(c+2));
		*(ss + strlen(c+2)) = '\0';
	}
	return ss;
}

/* the callback function of double click */
void choose_one(GtkTreeView *treeview,
		GtkTreePath *path,
		GtkTreeViewColumn *col,
		gpointer user_data)
{
	GtkTreeIter iter;
	gchar *resrc;

	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path)){
		gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 1, &resrc, -1);
		gtk_widget_destroy(download_window);
		tpid = fork();
		if(tpid == 0){
			execlp("/usr/lib/xlyrics/downloadlyrics.pl",
					"downloadlyrics.pl", resrc, redes, 0);
			exit(-1);
		}
		wait_user = 0;
		final = 1;
	}
}

/* read lyrics info and filled list sote with these */
void add_lyrics_info(const gchar *resrc)
{
	FILE *info;
	char buf[256];
	gchar *utf8;

	gtk_list_store_clear(GTK_LIST_STORE(store));
	snprintf(buf, 256, "/tmp/%s.lrc.info", resrc);
	info = fopen(buf, "r");
	if(info){
		GtkTreeIter iter;
		gchar *ptr;

		while(fgets(buf, 256, info)){
			gtk_list_store_append(GTK_LIST_STORE(store), &iter);
			if((ptr=strrchr(buf, '\r')) != NULL) *ptr = '\0';
			if((ptr=strrchr(buf, '\n')) != NULL) *ptr = '\0';
			utf8 = gb23122utf8(buf);
			if(!utf8) continue;
			if((ptr=strrchr(utf8, '@')) != NULL) *ptr = '\0';
			gtk_list_store_set(GTK_LIST_STORE(store), &iter,
					0, utf8,
					1, buf,
					-1);
			g_free(utf8);
		}
		fclose(info);
	}
}

void search_online(GtkButton *button, gpointer data)
{
	gchar *str;
	GtkEntry *entry;
	int child;
	char buf[256];

	entry = GTK_ENTRY(data);
	str = utf82locale(gtk_entry_get_text(entry));

	child = fork();
	if(child == 0){
		execlp("/usr/lib/xlyrics/downloadlyrics.pl",
				"downloadlyrics.pl", str, "/tmp/tmp_lyrics",
				"list", 0);
		exit(-1);
	}else if(child > 0){ 
		waitpid(child, NULL, 0);
		add_lyrics_info(str);
	}

	g_free(str);
}

void entry_activate(GtkEntry *entry, gpointer data)
{
	search_online(NULL, entry);
}

/* create the download window, so the user can choose or
 * search the lyrics manuallly
 */
GtkWidget* create_download_window(gboolean chose, char *download_dir)
{
	GtkWidget *scrolled_window;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *entry;
	GtkWidget *button;
	GtkTooltips *download_tip;
	gchar *utf8;

	/* download window */
	download_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(download_window), 
			_("Lyrics download manager"));
	gtk_window_set_default_size(GTK_WINDOW(download_window),
			220, 260);
	gtk_window_set_keep_above(GTK_WINDOW(download_window), TRUE);
	gtk_window_set_position(GTK_WINDOW(download_window), GTK_WIN_POS_CENTER);

	vbox = gtk_vbox_new(FALSE, 5);

	hbox = gtk_hbox_new(FALSE, 5);
	entry = gtk_entry_new();
	g_signal_connect(G_OBJECT(entry), "activate",
			G_CALLBACK(entry_activate), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 5);
	button = gtk_button_new_with_label(_("Search"));
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(search_online), entry);
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	/* list view widget */
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_signal_connect(G_OBJECT(tree), "row-activated",
			G_CALLBACK(choose_one), download_dir);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("lyrics", renderer,
			"text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW (tree), column);

	/* scrolled_window */
	download_tip = gtk_tooltips_new();
	gtk_tooltips_set_tip(GTK_TOOLTIPS(download_tip), tree,
			_("Double click the item to download the lyrics"),
			_("Choose the lyrics you want to download, and\
			double click on the item, the lyrics will be\
			downloaded automaticly"));

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);	
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrolled_window), tree);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 5);
	gtk_container_add(GTK_CONTAINER(download_window), vbox);

	if(chose){
		if(resrc != NULL && (utf8=locale2utf8(resrc)) != NULL){
			gtk_entry_set_text(GTK_ENTRY(entry), utf8);
			g_free(utf8);
		}
		add_lyrics_info(resrc);
		wait_user = 1;
	}

	gtk_widget_show_all(download_window);
	
	return download_window;
}

/* main function of download lyrics file 
 * src:music not music.mp3
 * des:/dir/music.lrc
 */
int lyrics_download(const char* src, const char* des)
{
	if(!src || !des) return -1;

	/* remove xxx-xxx */
	if(redes) g_free(redes);
	redes = (gchar*)strdup(des);
	if(resrc) g_free(resrc);
	resrc = extract_title2(src);

	/* start a new process to download the lyrics*/
	if(pid) kill(pid, SIGKILL);
	pid = fork();
	if(pid == 0){
		execlp("/usr/lib/xlyrics/downloadlyrics.pl",
				"downloadlyrics.pl", src, des, "first", 0);
		exit(-1);
	}else if(pid < 0)
		return -1;
	tpid = 0;
	wait_user = 0;
	final = 0;

	return 0;
}

/* get the download state, if more than one file is found
 * ask the user to choose the right one
 * Return Value:: 0:downloaded; 1:processing; -1:error
 */

int get_download_state()
{
	FILE *info;
	int i;
	char buf[256];

	if(wait_user) return 1;
	if(!pid && !tpid) return 0;
	/* check the download process */
	if(tpid){
		switch(waitpid(tpid, NULL, WNOHANG)){
			case 0: /* downloading */
				return 1;
			case -1: /* error */
				return -1;
			default:
				tpid = 0;
				break; /* process exit */
		}
	}

	/* check the download process */
	if(pid){
		switch(waitpid(pid, NULL, WNOHANG)){
			case 0: /* downloading */
				return 1;
			case -1: /* error */
				return -1;
			default:
				pid = 0;
				break; /* process exit */
		}
	}

	snprintf(buf, 256, "/tmp/%s.lrc.info", resrc);
	info = fopen(buf, "r");
	if(!info) return -1; 

	for(i=0; fgets(buf, 256, info); i++) ;

	if(i < 2) fclose(info);
	if(i == 0) return -1;
	if(i == 1) return 0;
	if(final) return 0;

	/* more file found need user choose one */
	create_download_window(TRUE, NULL);

	return 1;
}
