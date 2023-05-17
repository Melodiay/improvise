/**********************************************************************
* Проект: Расчёты
* Автор: Алексей Мелодия
**********************************************************************/
#include "main.h"

// Кнопка Расчёт
G_MODULE_EXPORT void on_button12_clicked (GtkButton *button, gpointer label)
{
        double H = 0.00;
        double M = 0.00;
        double S = 0.00;
        char strh[100];
        char strm[100];
        char strs[100];
        int hh = 0;
        int mm = 0;
        int ss = 0;

    if (gtk_toggle_button_get_active(radiobutton5))
    {

        double A = gtk_spin_button_get_value(a1);
        double B = gtk_spin_button_get_value(b2);


        if (B!=0)
        {
            // Отображение Результата (Ответ)
            ConOfPoints = (A/B);
            H = ConOfPoints;
            hh = H;
            sprintf(strh,"%i Ч", hh);
            H = H - (int)H;
            M = H * 60;
            mm = M;
            sprintf(strm,"%i Мин", mm);
            M = M - (int)M;
            S = M * 60;
            ss = S;
            sprintf(strs,"%i Сек", ss);

            gtk_label_set_label(GTK_LABEL(label10), strh);
            gtk_label_set_label(GTK_LABEL(label11), strm);
            gtk_label_set_label(GTK_LABEL(label12), strs);
            gtk_label_get_label(GTK_LABEL(label10));
            gtk_label_get_label(GTK_LABEL(label11));
            gtk_label_get_label(GTK_LABEL(label12));

            gtk_widget_queue_draw(area3);
        }


    }
    if (gtk_toggle_button_get_active(radiobutton6))
    {

        double A = gtk_spin_button_get_value(a1);
        double B = gtk_spin_button_get_value(b2);

        // Отображение Результата (Ответ)
            ConOfPoints = (B * A);

        gtk_widget_queue_draw(area3);

    }
     if (gtk_toggle_button_get_active(radiobutton7))
     {



        double A = gtk_spin_button_get_value(a1);
        double B = gtk_spin_button_get_value(b2);

        // Отображение Результата (Ответ)
        ConOfPoints = (A / B);

        gtk_widget_queue_draw(area3);

    }

}

void ShowPointsTSV(GtkWidget* widget)
{
    cairo_t * cr = gdk_cairo_create(gtk_widget_get_window(widget));

    char buf[100];
    cairo_pattern_t *pat;
    cairo_text_extents_t extents;


    sprintf(buf, "%.5f",ConOfPoints);

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

G_MODULE_EXPORT void on_draw3(GtkWidget* widget, cairo_t *cr)
{
    ShowPointsTSV(widget);
}

// Кнопка Очистить
G_MODULE_EXPORT void on_button13_clicked (GtkButton *button, gpointer label)
{
    gtk_spin_button_set_value(a1, 0);
    gtk_spin_button_set_value(b2, 0);

    ConOfPoints = 0.00;
    gtk_label_set_label(GTK_LABEL(label10), "Часы");
    gtk_label_set_label(GTK_LABEL(label11), "Минуты");
    gtk_label_set_label(GTK_LABEL(label12), "Секунды");
    gtk_label_get_label(GTK_LABEL(label10));
    gtk_label_get_label(GTK_LABEL(label11));
    gtk_label_get_label(GTK_LABEL(label12));

    gtk_widget_queue_draw(area3);
}
// Кнопка Радиокнопка
G_MODULE_EXPORT void on_radiobutton5_clicked (GtkButton *button, gpointer label)
{
        gtk_label_set_label(GTK_LABEL(label7), "S");
        gtk_label_set_label(GTK_LABEL(label8), "V");
        gtk_label_set_label(GTK_LABEL(label9), "T");
        gtk_label_get_label(GTK_LABEL(label7));
        gtk_label_get_label(GTK_LABEL(label8));
        gtk_label_get_label(GTK_LABEL(label9));
}
// Кнопка Радиокнопка
G_MODULE_EXPORT void on_radiobutton6_clicked (GtkButton *button, gpointer label)
{
        gtk_label_set_label(GTK_LABEL(label7), "V");
        gtk_label_set_label(GTK_LABEL(label8), "T");
        gtk_label_set_label(GTK_LABEL(label9), "S");
        gtk_label_get_label(GTK_LABEL(label7));
        gtk_label_get_label(GTK_LABEL(label8));
        gtk_label_get_label(GTK_LABEL(label9));
}

// Кнопка Радиокнопка
G_MODULE_EXPORT void on_radiobutton7_clicked (GtkButton *button, gpointer label)
{
        gtk_label_set_label(GTK_LABEL(label7), "S");
        gtk_label_set_label(GTK_LABEL(label8), "T");
        gtk_label_set_label(GTK_LABEL(label9), "V");
        gtk_label_get_label(GTK_LABEL(label7));
        gtk_label_get_label(GTK_LABEL(label8));
        gtk_label_get_label(GTK_LABEL(label9));
}

// Кнопка Закрыть
G_MODULE_EXPORT void on_button11_clicked (GtkButton *button, gpointer label)
{
    gtk_widget_destroy (wTsv);
}

int ShowTSV()
{
    bTsv = gtk_builder_new ();
    gtk_builder_add_from_file (bTsv, "main.glade", NULL);

    wTsv = GTK_WIDGET(gtk_builder_get_object(bTsv, "wTsv"));

    a1 = GTK_SPIN_BUTTON(gtk_builder_get_object(bTsv, "spinbutton5"));
    b2 = GTK_SPIN_BUTTON(gtk_builder_get_object(bTsv, "spinbutton6"));
    button12 =  GTK_WIDGET(gtk_builder_get_object(bTsv, "button12"));
    button13 =  GTK_WIDGET(gtk_builder_get_object(bTsv, "button13"));
    radiobutton5 = GTK_WIDGET(gtk_builder_get_object(bTsv, "radiobutton5"));
    radiobutton6 = GTK_WIDGET(gtk_builder_get_object(bTsv, "radiobutton6"));
    radiobutton7 = GTK_WIDGET(gtk_builder_get_object(bTsv, "radiobutton7"));
    area3 = GTK_WIDGET(gtk_builder_get_object(bTsv, "drawingarea3"));
    label7 = GTK_WIDGET(gtk_builder_get_object(bTsv, "label7"));
    label8 = GTK_WIDGET(gtk_builder_get_object(bTsv, "label8"));
    label9 = GTK_WIDGET(gtk_builder_get_object(bTsv, "label9"));
    label10 = GTK_WIDGET(gtk_builder_get_object(bTsv, "label10"));
    label11 = GTK_WIDGET(gtk_builder_get_object(bTsv, "label11"));
    label12 = GTK_WIDGET(gtk_builder_get_object(bTsv, "label12"));

    //gtk_window_set_default_size (GTK_WINDOW (wCondens), 450, 300);
    gtk_window_set_position(GTK_WINDOW(wTsv), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (bTsv, NULL);
    g_signal_connect(G_OBJECT(area3), "draw", G_CALLBACK(on_draw3), NULL);

        gtk_label_set_label(GTK_LABEL(label7), "S");
        gtk_label_set_label(GTK_LABEL(label8), "V");
        gtk_label_set_label(GTK_LABEL(label9), "T");
        gtk_label_get_label(GTK_LABEL(label7));
        gtk_label_get_label(GTK_LABEL(label8));
        gtk_label_get_label(GTK_LABEL(label9));
        gtk_label_set_label(GTK_LABEL(label10), "Часы");
        gtk_label_set_label(GTK_LABEL(label11), "Минуты");
        gtk_label_set_label(GTK_LABEL(label12), "Секунды");
        gtk_label_get_label(GTK_LABEL(label10));
        gtk_label_get_label(GTK_LABEL(label11));
        gtk_label_get_label(GTK_LABEL(label12));

    gtk_widget_show_all (wTsv);
}
