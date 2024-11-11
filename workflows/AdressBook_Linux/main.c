#include "main.h"

/**
// Открываем БД
void StartDB()
{
    if( sqlite3_open("book3.db", &db))
    {
        sprintf(str, "Ошибка открытия БД: %s\n", sqlite3_errmsg(db));
    }
}

// Закрываем БД
void EndDB()
{
    sqlite3_close(db);
}

void ShowError ()
{
    gtk_alert_dialog_get_message (GTK_ALERT_DIALOG(wMain));



    GtkWidget *dialog = NULL;

    dialog = gtk_message_dialog_new (GTK_WINDOW (wMain), GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, str);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

}
int ShowSQL()
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                    str);

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return 0;
}


int ShowGladeError()
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                 GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                 "Не найден файл\n main.glade");

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return 0;
}
**/

void	on_activate (GtkApplication* app) {

	bMain = gtk_builder_new_from_file ("book.glade");

	wMain = GTK_WIDGET(gtk_builder_get_object (bMain, "wMain"));

	gtk_window_set_application (GTK_WINDOW (wMain), app);

	g_signal_connect (wMain, "close-request", G_CALLBACK (on_window_close_request), NULL);

	GtkBox1 = GTK_WIDGET(gtk_builder_get_object(bMain, "GtkBox1"));
	GtkBox2 = GTK_WIDGET(gtk_builder_get_object(bMain, "GtkBox2"));
	button1 = GTK_WIDGET(gtk_builder_get_object(bMain, "button1"));
	button2 = GTK_WIDGET(gtk_builder_get_object(bMain, "button2"));
    button4 = GTK_WIDGET(gtk_builder_get_object(bMain, "button4"));

	gtk_window_set_title (GTK_WINDOW (wMain), "GTK4 Window");
	gtk_window_set_default_size (GTK_WINDOW (wMain), 300, 200);
	gtk_widget_set_visible (GTK_WIDGET(wMain), true);
	}


int main(int argc, char *argv[]) {

	app = gtk_application_new (NULL, G_APPLICATION_DEFAULT_FLAGS);

	g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);

	int status = g_application_run (G_APPLICATION (app), argc, argv);

	g_object_unref (app);
	return status;

    return 0;

	}

void	on_button1_clicked (GtkButton *b) {

	}

void	on_button2_clicked (GtkButton *b) {
	//ShowSettings();
	}

void	on_window_close_request(GtkWidget *w) {
	g_application_quit(G_APPLICATION(app));
	}
void gtk_main_quit(GtkWidget *w){
    g_application_quit(G_APPLICATION(app));
}

void	on_button4_clicked (GtkButton *b) {

	}

void	on_window_destroy(GtkWidget *w) {
	}
