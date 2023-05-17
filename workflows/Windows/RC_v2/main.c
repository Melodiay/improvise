/**********************************************************************
* Проект: Расчёты
* Автор: Алексей Мелодия
**********************************************************************/

#include <gtk/gtk.h>
#include "main.h"

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

G_MODULE_EXPORT void on_button2_clicked (GtkButton *button, gpointer label)
{
    ShowR();
}

G_MODULE_EXPORT void on_button3_clicked (GtkButton *button, gpointer label)
{
    ShowC();
}
G_MODULE_EXPORT void on_button10_clicked (GtkButton *button, gpointer label)
{
    ShowTSV();
}

int ShowMainWindow()
{


    bMain = gtk_builder_new ();
    if (gtk_builder_add_from_file (bMain, "main.glade", NULL))
    {
        wMain = GTK_WIDGET(gtk_builder_get_object(bMain, "wMain"));
        button1 =  GTK_WIDGET(gtk_builder_get_object(bMain, "button1"));
        button2 =  GTK_WIDGET(gtk_builder_get_object(bMain, "button2"));
        button10 =  GTK_WIDGET(gtk_builder_get_object(bMain, "button10"));

        //gtk_window_set_default_size (GTK_WINDOW (window1), 450, 250);
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
    if (ShowMainWindow ())
    {
        gtk_main ();
        return 0;
    }
    else
    {
        ShowGladeError();
        return 1;
    }
}
