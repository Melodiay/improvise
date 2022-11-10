/**********************************************************************
* Проект: Записная книга
* Автор: Алексей Быков
**********************************************************************/
#include "main.h"

int CurBook;
int CurBookPrint;
int StrInPage = 50;  // Контактов на странице
int Pages=0;         // Число страниц
char CurFoto[256];

char last_name_data[1000];
char name_data[1000];
char otchestvo_data[1000];
char number_telefone_data[1000];
char adres_data[1000];
char primechanie_data[1000];

GtkWidget *gLast_Name;
GtkWidget *gName;
GtkWidget *gOtchestvo;
GtkWidget *gNumberTelefone;
GtkWidget *gAdres;
GtkWidget *gPrimechanie;

// Обозначение полей
enum
{
    C_ID, C_LAST_NAME, C_NAME, C_OTCHESTVO, C_NUMBER_TELEFONE, C_ADRES, C_PRIMECHANIE, C_FOTO
};

// Вывод карточки контакта на печать
static void DrawPageBook(GtkPrintOperation *op, GtkPrintContext *context, int page_nr)
{
    cairo_t *cr;
    gchar buf[1000];
    cr = gtk_print_context_get_cairo_context (context);

    strcpy(last_name_data, gtk_entry_get_text(GTK_ENTRY(gLast_Name)));
    strcpy(name_data, gtk_entry_get_text(GTK_ENTRY(gName)));
    strcpy(otchestvo_data, gtk_entry_get_text(GTK_ENTRY(gOtchestvo)));
    strcpy(number_telefone_data, gtk_entry_get_text(GTK_ENTRY(gNumberTelefone)));
    strcpy(adres_data, gtk_entry_get_text(GTK_ENTRY(gAdres)));
    strcpy(primechanie_data, gtk_entry_get_text(GTK_ENTRY(gPrimechanie)));

    // Выводим заголовок
    cairo_move_to (cr, 80.0, 10.0);
    cairo_set_font_size (cr, 5.0);
    cairo_select_font_face(cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_show_text (cr, "Карточка контакта");
    cairo_stroke(cr);

    // Выводим поле "Фамилия"
    cairo_select_font_face(cr, "sans serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (cr, 10.0, 20.0);
    cairo_set_font_size (cr, 5.0);
    sprintf(buf, "Фамилия - %s", last_name_data);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);

    // Выводим поле "Имя"
    cairo_select_font_face(cr, "sans serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (cr, 10.0, 25.0);
    cairo_set_font_size (cr, 5.0);
    sprintf(buf, "Имя - %s", name_data);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);

    // Выводим поле "Отчество"
    cairo_select_font_face(cr, "sans serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (cr, 10.0, 30.0);
    cairo_set_font_size (cr, 5.0);
    sprintf(buf, "Отчество - %s", otchestvo_data);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);

    // Выводим поле "Номер телефона"
    cairo_select_font_face(cr, "sans serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (cr, 10.0, 35.0);
    cairo_set_font_size (cr, 5.0);
    sprintf(buf, "Номер телефона - %s", number_telefone_data);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);

    // Выводим поле "Адрес"
    cairo_select_font_face(cr, "sans serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (cr, 10.0, 40.0);
    cairo_set_font_size (cr, 5.0);
    sprintf(buf, "Адрес - %s", adres_data);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);

    // Выводим поле "Примечание"
    cairo_select_font_face(cr, "sans serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (cr, 10.0, 45.0);
    cairo_set_font_size (cr, 5.0);
    sprintf(buf, "Примечание - %s", primechanie_data);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);

}

// Печать карточки контакта
void PrintBook()
{
    GtkPrintOperation *op;

    op = gtk_print_operation_new ();

    gtk_print_operation_set_n_pages (op, 1);
    gtk_print_operation_set_unit (op, GTK_UNIT_MM);
    g_signal_connect (op, "draw_page", G_CALLBACK (DrawPageBook), NULL);
    gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW (wBook), NULL);
}

// Вывод страницы из списка печать
int DrawPageBooks(GtkPrintOperation *op, GtkPrintContext *context, int page_nr)
{
    cairo_t *cr;
    gchar buf[1000];

    cr = gtk_print_context_get_cairo_context (context);

    char* SQL = "SELECT [Код контактов], Фамилия, Имя, Отчество, [Номер телефона] FROM Контакты LIMIT ";

    // Готовим SQL-запрос
    sprintf(str, "%s%d OFFSET %d;",SQL, StrInPage, page_nr*StrInPage);
    ///ShowSQL(); // Тестовый вывод запроса
    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Выводим заголовок
    cairo_move_to (cr, 80.0, 10.0);
    cairo_set_font_size (cr, 5.0);
    cairo_select_font_face(cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    sprintf(buf, "Страница № %d из %d", ++page_nr, Pages);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);

    // Выводим шапку таблицы
    cairo_set_line_width (cr, 1.0);
    cairo_move_to (cr, 2.0, 20.0);
    cairo_line_to (cr, 196, 20);
    cairo_stroke (cr);
    cairo_move_to (cr, 6.0, 26.0);
    sprintf(buf, "Фамилия               Имя               Отчество                   Номер телефона ");
    cairo_show_text (cr, buf);
    cairo_stroke(cr);
    cairo_move_to (cr, 2.0, 28.0);
    cairo_line_to (cr, 196.0, 28.0);
    cairo_stroke (cr);

    double CurStr = 35.0;
    // Цикл по SQL-запросу и печать строк
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        // Выводим поле "Фамилия"
        cairo_select_font_face(cr, "sans serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_move_to (cr, 4.0, CurStr);
        cairo_set_font_size (cr, 5.0);
        sprintf(buf, "%s", sqlite3_column_text(stmt, C_LAST_NAME));
        cairo_show_text (cr, buf);
        cairo_stroke(cr);

        // Выводим поле "Имя"
        cairo_move_to (cr, 44.0, CurStr);
        sprintf(buf, "%s", sqlite3_column_text(stmt, C_NAME));
        cairo_show_text (cr, buf);
        cairo_stroke(cr);

        // Выводим поле "Отчество"
        cairo_move_to (cr, 80.0, CurStr);
        sprintf(buf, "%s", sqlite3_column_text(stmt, C_OTCHESTVO));
        cairo_show_text (cr, buf);
        cairo_stroke(cr);

        // Выводим поле "Номер телефона"
        cairo_move_to (cr, 130.0, CurStr);
        sprintf(buf, "%s", sqlite3_column_text(stmt, C_NUMBER_TELEFONE));
        cairo_show_text (cr, buf);
        cairo_stroke(cr);

        CurStr += 5.0; // Переходим на следующую строку
    }

    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }


    sqlite3_finalize(stmt);
    return 0;
}


// Начало печати списка  - подсчет страниц
void BeginPrint(GtkPrintOperation *oper, GtkPrintContext *context)
{
    Pages=0;


    char* SQL = "SELECT count(Фамилия) FROM Контакты";


    if(sqlite3_prepare_v2(db, SQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    rc = sqlite3_step(stmt);
    Pages = sqlite3_column_int(stmt, 0)/StrInPage;
    if (sqlite3_column_int(stmt, 0)%StrInPage)
    {
        Pages++;
    }

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);

    gtk_print_operation_set_n_pages(oper, Pages);
}

// Печать списка товаров
void PrintBooks()
{
    GtkPrintOperation *op;

    op = gtk_print_operation_new ();

    gtk_print_operation_set_unit (op, GTK_UNIT_MM);
    g_signal_connect (op, "begin_print", G_CALLBACK (BeginPrint), NULL);
    g_signal_connect (op, "draw_page", G_CALLBACK (DrawPageBooks), NULL);
    gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW (wBook), NULL);
}

// Загрузка записи
int LoadRecord(int book)
{

    char* SQL = "SELECT [Код контактов], Фамилия, Имя, Отчество, [Номер телефона], Адрес, Примечание, Фото \
                FROM Контакты WHERE [Код контактов]=";

    // Готовим SQL-запрос
    sprintf(str, "%s%d;",SQL, book);
    ///ShowSQL(); // Тестовый вывод запроса
    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }
    // Выполнение SQL-запроса
    rc = sqlite3_step(stmt);

    // Фамилия
    // Запись в виджеты
    if (sqlite3_column_text(stmt, C_LAST_NAME)>0)
    {
        sprintf(str, "%s",sqlite3_column_text(stmt, C_LAST_NAME));
    }
    else
    {
        sprintf(str, " ");
    }
    gtk_entry_set_text(GTK_ENTRY(gLast_Name),str);

    // Имя
    // Запись в виджеты
    if (sqlite3_column_text(stmt, C_NAME)>0)
    {
        sprintf(str, "%s",sqlite3_column_text(stmt, C_NAME));
    }
    else
    {
        sprintf(str, " ");
    }
    gtk_entry_set_text(GTK_ENTRY(gName),str);

    //Отчество
    // Запись в виджеты
    if (sqlite3_column_text(stmt, C_OTCHESTVO)>0)
    {
        sprintf(str, "%s",sqlite3_column_text(stmt, C_OTCHESTVO));
    }
    else
    {
        sprintf(str, " ");
    }
    gtk_entry_set_text(GTK_ENTRY(gOtchestvo),str);

    // Номер телефона
    // Запись в виджеты
    if (sqlite3_column_text(stmt, C_NUMBER_TELEFONE)>0)
    {
        sprintf(str, "%s",sqlite3_column_text(stmt, C_NUMBER_TELEFONE));
    }
    else
    {
        sprintf(str, " ");
    }
    gtk_entry_set_text(GTK_ENTRY(gNumberTelefone),str);

    // Адрес
    // Запись в виджеты
    if (sqlite3_column_text(stmt, C_ADRES)>0)
    {
        sprintf(str, "%s",sqlite3_column_text(stmt, C_ADRES));
    }
    else
    {
        sprintf(str, " ");
    }
    gtk_entry_set_text(GTK_ENTRY(gAdres),str);

    // Примечание
    // Запись в виджеты
    if (sqlite3_column_text(stmt, C_PRIMECHANIE)>0)
    {
        sprintf(str, "%s",sqlite3_column_text(stmt, C_PRIMECHANIE));
    }
    else
    {
        sprintf(str, " ");
    }
    gtk_entry_set_text(GTK_ENTRY(gPrimechanie),str);

    // Фото
    // Выбор фото
    if (sqlite3_column_text(stmt, C_FOTO) > 0)
    {
        sprintf(CurFoto,"%s", sqlite3_column_text(stmt, C_FOTO));
        gtk_image_set_from_file (GTK_IMAGE(gFoto), CurFoto);
    }
    else
    {
        strcpy(CurFoto,"");
    }

    sqlite3_finalize(stmt);
    return 0;
}

// Сохранение записи
void SaveRecord()
{
    strcpy(last_name_data, gtk_entry_get_text(GTK_ENTRY(gLast_Name)));
    strcpy(name_data, gtk_entry_get_text(GTK_ENTRY(gName)));
    strcpy(otchestvo_data, gtk_entry_get_text(GTK_ENTRY(gOtchestvo)));
    strcpy(number_telefone_data, gtk_entry_get_text(GTK_ENTRY(gNumberTelefone)));
    strcpy(adres_data, gtk_entry_get_text(GTK_ENTRY(gAdres)));
    strcpy(primechanie_data, gtk_entry_get_text(GTK_ENTRY(gPrimechanie)));

    // Готовим SQL-запрос
    lenstr = 0;
    lenstr+=sprintf(str, "UPDATE Контакты SET Фамилия=");
    lenstr+=sprintf(str+lenstr, "'%s', Имя ='%s', Отчество ='%s'", last_name_data, name_data, otchestvo_data);
    lenstr+=sprintf(str+lenstr, ", [Номер телефона] ='%s', Адрес ='%s', Примечание ='%s', Фото ='%s'", number_telefone_data, adres_data, primechanie_data, CurFoto);
    lenstr+=sprintf(str+lenstr, " WHERE [Код контактов]=%d", CurBook);
    ///ShowSQL(); // Тестовый вывод запроса
    // Записываем в БД
    if (sqlite3_exec(db, str, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return;
    }
}

// Кнопка Печать
G_MODULE_EXPORT void on_button17_clicked (GtkButton *button, gpointer label)
{
    PrintBook();
}

// Кнопка Печать списка
G_MODULE_EXPORT void on_button16_clicked (GtkButton *button, gpointer label)
{
    PrintBooks();
}

// Кнопка Выбрать фото
G_MODULE_EXPORT void on_button13_clicked (GtkButton *button, gpointer label)
{
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          GTK_WINDOW(wBook),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    // Устанавливаем текущий каталог
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "./images");
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        strcpy(CurFoto, filename);
        gtk_image_set_from_file (GTK_IMAGE(gFoto), CurFoto);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
}

// Кнопка Закрыть
G_MODULE_EXPORT void on_button9_clicked (GtkButton *button, gpointer label)
{
    SaveRecord(); // Запись значений в БД
    Reload(CurBook); // Обновление записи в окне Сотрудники
    ReloadSearch(CurBook);
    gtk_widget_destroy (wBook);
    gtk_widget_destroy (wBooks);
    ShowBooks();

}

// Вывод окна
void ShowBook(int book)
{
    CurBook = book;

    bBook = gtk_builder_new ();
    gtk_builder_add_from_file (bBook, "book.glade", NULL);
    wBook = GTK_WIDGET(gtk_builder_get_object(bBook, "wBook"));

    gLast_Name = GTK_WIDGET(gtk_builder_get_object(bBook, "entry1"));
    gName = GTK_WIDGET(gtk_builder_get_object(bBook, "entry2"));
    gOtchestvo = GTK_WIDGET(gtk_builder_get_object(bBook, "entry3"));
    gNumberTelefone = GTK_WIDGET(gtk_builder_get_object(bBook, "entry4"));
    gAdres = GTK_WIDGET(gtk_builder_get_object(bBook, "entry5"));
    gPrimechanie = GTK_WIDGET(gtk_builder_get_object(bBook, "entry6"));
    gFoto = GTK_WIDGET(gtk_builder_get_object(bBook, "image1"));

    gtk_window_set_title (GTK_WINDOW (wBook), "Записная книга");
    gtk_window_set_default_size (GTK_WINDOW (wBook), 700, 300);
    gtk_window_set_position(GTK_WINDOW(wBook), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (bBook, NULL);

    LoadRecord(book);

    gtk_widget_show_all(wBook);
}
