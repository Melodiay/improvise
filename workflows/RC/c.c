/**********************************************************************
* Проект: Расчёты
* Автор: Алексей Мелодия
**********************************************************************/
#include "main.h"



// Кнопка Закрыть
G_MODULE_EXPORT void on_button5_clicked (GtkButton *button, gpointer label)
{
    gtk_widget_destroy (wCondens);
}

int ShowC()
{

    bCondens = gtk_builder_new ();
    gtk_builder_add_from_file (bCondens, "main.glade", NULL);

    wCondens = GTK_WIDGET(gtk_builder_get_object(bCondens, "wCondens"));


    //gtk_window_set_default_size (GTK_WINDOW (wCondens), 450, 300);
    gtk_window_set_position(GTK_WINDOW(wCondens), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (bCondens, NULL);

    gtk_widget_show_all (wCondens);

}
