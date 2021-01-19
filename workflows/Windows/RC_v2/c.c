/**********************************************************************
* Проект: Расчёты
* Автор: Алексей Мелодия
**********************************************************************/
#include "main.h"

// Кнопка Расчёт
G_MODULE_EXPORT void on_button8_clicked (GtkButton *button, gpointer label)
{

    if (gtk_toggle_button_get_active(radiobutton3))
    {

        double C1 = gtk_spin_button_get_value(Conden1);
        double C2 = gtk_spin_button_get_value(Conden2);

        // Отображение Результата (Ответ)
        ConOfPoints = 1/(1/C1 + 1/C2);

        gtk_widget_queue_draw(area2);

    }
    else
    {

        double C1 = gtk_spin_button_get_value(Conden1);
        double C2 = gtk_spin_button_get_value(Conden2);

        // Отображение Результата (Ответ)
        ConOfPoints = C1 + C2;

        gtk_widget_queue_draw(area2);

    }

}

void ShowPointsC(GtkWidget* widget)
{
    cairo_t * cr = gdk_cairo_create(gtk_widget_get_window(widget));

    char buf[100];
    cairo_pattern_t *pat;
    cairo_text_extents_t extents;


    sprintf(buf, "%.3f uF",ConOfPoints);

    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_paint(cr);

    cairo_select_font_face(cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 35);

    pat = cairo_pattern_create_linear(0, 15, 0, 34*0.8);
    cairo_pattern_set_extend(pat, CAIRO_EXTEND_REPEAT);
    cairo_pattern_add_color_stop_rgb(pat, 0.0, 1, 0.6, 0);
    cairo_pattern_add_color_stop_rgb(pat, 0.5, 1, 0.3, 0);

    cairo_text_extents(cr, buf, &extents);
    cairo_move_to(cr, 340 - extents.width, 34);

    cairo_text_path(cr, buf);
    cairo_set_source(cr, pat);
    cairo_fill(cr);
}

G_MODULE_EXPORT void on_draw2(GtkWidget* widget, cairo_t *cr)
{
    ShowPointsC(widget);
}

// Кнопка Очистить
G_MODULE_EXPORT void on_button9_clicked (GtkButton *button, gpointer label)
{
    gtk_spin_button_set_value(Conden1, 0);
    gtk_spin_button_set_value(Conden2, 0);

    ConOfPoints = 0.00;
    gtk_widget_queue_draw(area2);
}

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

    Conden1 = GTK_SPIN_BUTTON(gtk_builder_get_object(bCondens, "spinbutton3"));
    Conden2 = GTK_SPIN_BUTTON(gtk_builder_get_object(bCondens, "spinbutton4"));
    button8 =  GTK_WIDGET(gtk_builder_get_object(bCondens, "button8"));
    button9 =  GTK_WIDGET(gtk_builder_get_object(bCondens, "button9"));
    radiobutton3 = GTK_WIDGET(gtk_builder_get_object(bCondens, "radiobutton3"));
    radiobutton4 = GTK_WIDGET(gtk_builder_get_object(bCondens, "radiobutton4"));
    area2 = GTK_WIDGET(gtk_builder_get_object(bCondens, "drawingarea2"));

    //gtk_window_set_default_size (GTK_WINDOW (wCondens), 450, 300);
    gtk_window_set_position(GTK_WINDOW(wCondens), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (bCondens, NULL);
    g_signal_connect(G_OBJECT(area2), "draw", G_CALLBACK(on_draw2), NULL);

    gtk_widget_show_all (wCondens);

}
