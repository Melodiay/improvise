/**********************************************************************
* Проект: Расчёты
* Автор: Алексей Мелодия
**********************************************************************/
#include "main.h"

// Кнопка Расчёт
G_MODULE_EXPORT void on_button6_clicked (GtkButton *button, gpointer label)
{

    if (gtk_toggle_button_get_active(radiobutton1))
    {

        double R1 = gtk_spin_button_get_value(Resis1);
        double R2 = gtk_spin_button_get_value(Resis2);

        // Отображение Результата (Ответ)
        NumOfPoints = R1 + R2;

        gtk_widget_queue_draw(area1);

    }
    else
    {
        double R1 = gtk_spin_button_get_value(Resis1);
        double R2 = gtk_spin_button_get_value(Resis2);

        // Отображение Результата (Ответ)
        NumOfPoints = 1/(1/R1+1/R2);

        gtk_widget_queue_draw(area1);

    }

}

void ShowPoints(GtkWidget* widget)
{
    cairo_t * cr = gdk_cairo_create(gtk_widget_get_window(widget));

    char buf[100];
    cairo_pattern_t *pat;
    cairo_text_extents_t extents;


    sprintf(buf, "%.3f Om",NumOfPoints);

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

G_MODULE_EXPORT void on_draw1(GtkWidget* widget, cairo_t *cr)
{
    ShowPoints(widget);
}

// Кнопка Очистить
G_MODULE_EXPORT void on_button7_clicked (GtkButton *button, gpointer label)
{
    gtk_spin_button_set_value(Resis1, 0);
    gtk_spin_button_set_value(Resis2, 0);

    NumOfPoints = 0.00;
    gtk_widget_queue_draw(area1);
}

// Кнопка Закрыть
G_MODULE_EXPORT void on_button4_clicked (GtkButton *button, gpointer label)
{
    gtk_widget_destroy (wResistors);
}

int ShowR()
{

    bResistors = gtk_builder_new ();
    gtk_builder_add_from_file (bResistors, "main.glade", NULL);

    wResistors = GTK_WIDGET(gtk_builder_get_object(bResistors, "wResistors"));

    Resis1 = GTK_SPIN_BUTTON(gtk_builder_get_object(bResistors, "spinbutton1"));
    Resis2 = GTK_SPIN_BUTTON(gtk_builder_get_object(bResistors, "spinbutton2"));
    button6 =  GTK_WIDGET(gtk_builder_get_object(bResistors, "button6"));
    button7 =  GTK_WIDGET(gtk_builder_get_object(bResistors, "button7"));
    radiobutton1 = GTK_WIDGET(gtk_builder_get_object(bResistors, "radiobutton1"));
    radiobutton2 = GTK_WIDGET(gtk_builder_get_object(bResistors, "radiobutton2"));
    area1 = GTK_WIDGET(gtk_builder_get_object(bResistors, "drawingarea1"));


    //gtk_window_set_default_size (GTK_WINDOW (wResistors), 450, 300);
    gtk_window_set_position(GTK_WINDOW(wResistors), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (bResistors, NULL);
    g_signal_connect(G_OBJECT(area1), "draw", G_CALLBACK(on_draw1), NULL);

    gtk_widget_show_all (wResistors);

}


