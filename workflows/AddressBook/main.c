/*********************************************************************
* Записная книга 
* автор Быков Алексей
* Можете учиться разработке в CodeBlocks работе с базой данных SQLite
* Продовать програму запрещено, распространяется бесплатно
**********************************************************************/
#include "main.h"

// Открываем БД
void StartDB()
{
    if( sqlite3_open("book.db", &db))
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

G_MODULE_EXPORT void on_button1_clicked (GtkButton *button, gpointer label)
{
    ShowBooks();
}

G_MODULE_EXPORT void on_button2_clicked (GtkButton *button, gpointer label)
{
    ShowSettings();
}

int ShowMainWindow()
{

	bMain = gtk_builder_new ();
	if (gtk_builder_add_from_file (bMain, "book.glade", NULL))
	{
        wMain = GTK_WIDGET(gtk_builder_get_object(bMain, "wMain"));
        gtk_window_set_title (GTK_WINDOW (wMain), "Записная книга");
        gtk_window_set_default_size (GTK_WINDOW (wMain), 500, 300);
        gtk_window_set_position(GTK_WINDOW(wMain), GTK_WIN_POS_CENTER);

	    gtk_builder_connect_signals (bMain, NULL);
        g_signal_connect(G_OBJECT(wMain), "destroy", G_CALLBACK(gtk_main_quit), NULL);

        gtk_widget_show (wMain);
        return TRUE;
	}
    else
	{
	    return FALSE;
	}
}

int main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);
    StartDB();
    if (ShowMainWindow ())
    {
       gtk_main ();
       EndDB();
       return 0;
    }
    else
    {
       ShowGladeError();
       EndDB();
       return 1;
    }
}
