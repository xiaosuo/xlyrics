#include <gtk/gtk.h>

guint c_timer;	
gboolean is_saved = FALSE;
GtkWidget *creator_window;
GtkWidget *c_list_view;
GtkListStore *c_list_store;

/* open a lyrics to edit*/
void open_call()
{
	GtkWidget *file_sel;

	file_sel = gtk_file_selection_new("Select a lyrics to edit");
	gtk_window_set_modal(GTK_WINDOW(file_sel), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(file_sel), GTK_WINDOW(creator_window));

	gtk_widget_show_all(file_sel);
}

/* save the lyrics*/
void save_call()
{
}

/* quit the program*/
void quit_call()
{
	gtk_widget_destroy(creator_window);
}

/* the toolbar of creator*/
GtkWidget *creator_toolbar()
{
	GtkWidget *toolbar;
	GtkToolItem *item;

	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolbar), FALSE);

	item = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect(G_OBJECT(item), "clicked",
			G_CALLBACK(open_call), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, 0);

	item = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
	g_signal_connect(G_OBJECT(item), "clicked",
			G_CALLBACK(save_call), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, 1);

	return toolbar;
}

/* if left button press set_time,right start editting*/
gboolean  mouse_press_call(GtkWidget *widget, GdkEventButton *event)
{

	return FALSE;
}
void row_activated_call()
{
}
/* the scrolled window contains the content*/ 
GtkWidget *creator_content()
{

	GtkWidget *scrolled_window;
	GtkCellRenderer *render;
	GtkTreeViewColumn *col;

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	c_list_store = gtk_list_store_new(1, GTK_TYPE_STRING);
	c_list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(c_list_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(c_list_view), FALSE);
	g_signal_connect(G_OBJECT(c_list_view), "button-press-event",
			G_CALLBACK(mouse_press_call), NULL);
	g_signal_connect(G_OBJECT(c_list_view), "row-activated",
			G_CALLBACK(row_activated_call), NULL);

	render = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("sightless",
			render, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(c_list_view),
			col);

	gtk_container_add(GTK_CONTAINER(scrolled_window), c_list_view);
	gtk_widget_show_all(c_list_view);	

	return scrolled_window;
}

/* lyrics creator's main window*/
void lyrics_creator()
{
	GtkWidget *vbox;

	vbox = gtk_vbox_new(FALSE, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), creator_toolbar(), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), creator_content(), TRUE, TRUE, 0);

	creator_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(creator_window), 300, 450);
	gtk_window_set_title(GTK_WINDOW(creator_window), "Xlyrics lyrics creator");
	gtk_window_set_position(GTK_WINDOW(creator_window), GTK_WIN_POS_MOUSE);
	g_signal_connect(G_OBJECT(creator_window), "delete_event",
			G_CALLBACK(quit_call), NULL);
	gtk_container_add(GTK_CONTAINER(creator_window), vbox);

	gtk_widget_show_all(creator_window);

}
