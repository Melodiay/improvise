/**********************************************************************
* Проект: Записная книга
* Автор: Алексей Быков
**********************************************************************/
#include "main.h"

// Структуры данных для виджета
GtkListStore        *buffer2;    // Буфер таблицы
GtkTreeView         *treeview2; // Таблица
GtkTreeViewColumn   *column;    // Отображаемая колонка
GtkTreeIter         iter;       // Итератор таблицы (текущая строка)
GtkCellRenderer     *renderer;  // Рендер таблицы (текущая ячейка)

// Обозначение полей
enum
{
    C_ID, C_DATE, C_LAST_NAME, C_NAME, C_OTCHESTVO, C_NUMBER_TELEFONE, C_EMAIL, C_ADRES, C_PRIMECHANIE, C_FOTO
};

GtkWidget *gLast_Name;
GtkWidget *gName;
GtkWidget *gOtchestvo;
GtkWidget *gNumberTelefone;
GtkWidget *gAdres;
GtkWidget *gPrimechanie;

int CurBook;
int search;
int book;

char last_name_data[1000];
char name_data[1000];
char otchestvo_data[1000];
char number_telefone_data[1000];

int LoadSearchLN()
{
    strcpy(last_name_data, gtk_entry_get_text(GTK_ENTRY(gLast_Name)));

    char* SQL = "SELECT [Код контактов], strftime('%%d.%%m.%%Y',Дата), Фамилия, Имя, Отчество, [Номер телефона], [Электронная почта], Адрес, Примечание, Фото \
                FROM Контакты WHERE Фамилия IN";

    // Готовим SQL-запрос к БД
    lenstr = 0;
    lenstr+=sprintf(str, SQL);
    lenstr+=sprintf(str+lenstr, " ('%s')", last_name_data);
    lenstr+=sprintf(str+lenstr, " ORDER BY [Код контактов]");
    ///ShowSQL(); // Тестовый вывод запроса
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {

        gtk_list_store_append (buffer2, &iter);
        gtk_list_store_set (buffer2, &iter,
                            C_ID, sqlite3_column_int(stmt, C_ID),
                            C_DATE, sqlite3_column_text(stmt, C_DATE),
                            C_LAST_NAME, sqlite3_column_text(stmt, C_LAST_NAME),
                            C_NAME, sqlite3_column_text(stmt, C_NAME),
                            C_OTCHESTVO, sqlite3_column_text(stmt, C_OTCHESTVO),
                            C_NUMBER_TELEFONE, sqlite3_column_text(stmt, C_NUMBER_TELEFONE),
                            C_EMAIL, sqlite3_column_text(stmt, C_EMAIL),
                            C_ADRES, sqlite3_column_text(stmt, C_ADRES),
                            C_PRIMECHANIE, sqlite3_column_text(stmt, C_PRIMECHANIE),
                            C_FOTO, sqlite3_column_text(stmt, C_FOTO),
                            -1);
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

int LoadSearchN()
{
    strcpy(name_data, gtk_entry_get_text(GTK_ENTRY(gName)));

    char* SQL = "SELECT [Код контактов], strftime('%%d.%%m.%%Y',Дата), Фамилия, Имя, Отчество, [Номер телефона], [Электронная почта], Адрес, Примечание, Фото \
                FROM Контакты WHERE Имя IN";

    // Готовим SQL-запрос к БД
    lenstr = 0;
    lenstr+=sprintf(str, SQL);
    lenstr+=sprintf(str+lenstr, " ('%s')", name_data);
    lenstr+=sprintf(str+lenstr, " ORDER BY [Код контактов]");
    ///ShowSQL(); // Тестовый вывод запроса
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {

        gtk_list_store_append (buffer2, &iter);
        gtk_list_store_set (buffer2, &iter,
                            C_ID, sqlite3_column_int(stmt, C_ID),
                            C_DATE, sqlite3_column_text(stmt, C_DATE),
                            C_LAST_NAME, sqlite3_column_text(stmt, C_LAST_NAME),
                            C_NAME, sqlite3_column_text(stmt, C_NAME),
                            C_OTCHESTVO, sqlite3_column_text(stmt, C_OTCHESTVO),
                            C_NUMBER_TELEFONE, sqlite3_column_text(stmt, C_NUMBER_TELEFONE),
                            C_EMAIL, sqlite3_column_text(stmt, C_EMAIL),
                            C_ADRES, sqlite3_column_text(stmt, C_ADRES),
                            C_PRIMECHANIE, sqlite3_column_text(stmt, C_PRIMECHANIE),
                            C_FOTO, sqlite3_column_text(stmt, C_FOTO),
                            -1);
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

int LoadSearchO()
{
    strcpy(otchestvo_data, gtk_entry_get_text(GTK_ENTRY(gOtchestvo)));

    char* SQL = "SELECT [Код контактов], strftime('%%d.%%m.%%Y',Дата), Фамилия, Имя, Отчество, [Номер телефона], [Электронная почта], Адрес, Примечание, Фото \
                FROM Контакты WHERE Отчество IN";

    // Готовим SQL-запрос к БД
    lenstr = 0;
    lenstr+=sprintf(str, SQL);
    lenstr+=sprintf(str+lenstr, " ('%s')", otchestvo_data);
    lenstr+=sprintf(str+lenstr, " ORDER BY [Код контактов]");
    ///ShowSQL(); // Тестовый вывод запроса
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {

        gtk_list_store_append (buffer2, &iter);
        gtk_list_store_set (buffer2, &iter,
                            C_ID, sqlite3_column_int(stmt, C_ID),
                            C_DATE, sqlite3_column_text(stmt, C_DATE),
                            C_LAST_NAME, sqlite3_column_text(stmt, C_LAST_NAME),
                            C_NAME, sqlite3_column_text(stmt, C_NAME),
                            C_OTCHESTVO, sqlite3_column_text(stmt, C_OTCHESTVO),
                            C_NUMBER_TELEFONE, sqlite3_column_text(stmt, C_NUMBER_TELEFONE),
                            C_EMAIL, sqlite3_column_text(stmt, C_EMAIL),
                            C_ADRES, sqlite3_column_text(stmt, C_ADRES),
                            C_PRIMECHANIE, sqlite3_column_text(stmt, C_PRIMECHANIE),
                            C_FOTO, sqlite3_column_text(stmt, C_FOTO),
                            -1);
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

int LoadSearchLNN()
{
    strcpy(last_name_data, gtk_entry_get_text(GTK_ENTRY(gLast_Name)));
    strcpy(name_data, gtk_entry_get_text(GTK_ENTRY(gName)));

    char* SQL = "SELECT [Код контактов], strftime('%%d.%%m.%%Y',Дата), Фамилия, Имя, Отчество, [Номер телефона], [Электронная почта], Адрес, Примечание, Фото \
                FROM Контакты WHERE Фамилия IN";

    // Готовим SQL-запрос к БД
    lenstr = 0;
    lenstr+=sprintf(str, SQL);
    lenstr+=sprintf(str+lenstr, " ('%s') AND Имя IN ('%s')", last_name_data, name_data);
    lenstr+=sprintf(str+lenstr, " ORDER BY [Код контактов]");
    ///ShowSQL(); // Тестовый вывод запроса
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {

        gtk_list_store_append (buffer2, &iter);
        gtk_list_store_set (buffer2, &iter,
                            C_ID, sqlite3_column_int(stmt, C_ID),
                            C_DATE, sqlite3_column_text(stmt, C_DATE),
                            C_LAST_NAME, sqlite3_column_text(stmt, C_LAST_NAME),
                            C_NAME, sqlite3_column_text(stmt, C_NAME),
                            C_OTCHESTVO, sqlite3_column_text(stmt, C_OTCHESTVO),
                            C_NUMBER_TELEFONE, sqlite3_column_text(stmt, C_NUMBER_TELEFONE),
                            C_EMAIL, sqlite3_column_text(stmt, C_EMAIL),
                            C_ADRES, sqlite3_column_text(stmt, C_ADRES),
                            C_PRIMECHANIE, sqlite3_column_text(stmt, C_PRIMECHANIE),
                            C_FOTO, sqlite3_column_text(stmt, C_FOTO),
                            -1);
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

int LoadSearchLNNO()
{
    strcpy(last_name_data, gtk_entry_get_text(GTK_ENTRY(gLast_Name)));
    strcpy(name_data, gtk_entry_get_text(GTK_ENTRY(gName)));
    strcpy(otchestvo_data, gtk_entry_get_text(GTK_ENTRY(gOtchestvo)));

    char* SQL = "SELECT [Код контактов], strftime('%%d.%%m.%%Y',Дата), Фамилия, Имя, Отчество, [Номер телефона], [Электронная почта], Адрес, Примечание, Фото \
                FROM Контакты WHERE Фамилия IN";

    // Готовим SQL-запрос к БД
    lenstr = 0;
    lenstr+=sprintf(str, SQL);
    lenstr+=sprintf(str+lenstr, " ('%s') AND Имя IN ('%s') AND Отчество IN ('%s')", last_name_data, name_data, otchestvo_data);
    lenstr+=sprintf(str+lenstr, " ORDER BY [Код контактов]");
    ///ShowSQL(); // Тестовый вывод запроса
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {

        gtk_list_store_append (buffer2, &iter);
        gtk_list_store_set (buffer2, &iter,
                            C_ID, sqlite3_column_int(stmt, C_ID),
                            C_DATE, sqlite3_column_text(stmt, C_DATE),
                            C_LAST_NAME, sqlite3_column_text(stmt, C_LAST_NAME),
                            C_NAME, sqlite3_column_text(stmt, C_NAME),
                            C_OTCHESTVO, sqlite3_column_text(stmt, C_OTCHESTVO),
                            C_NUMBER_TELEFONE, sqlite3_column_text(stmt, C_NUMBER_TELEFONE),
                            C_EMAIL, sqlite3_column_text(stmt, C_EMAIL),
                            C_ADRES, sqlite3_column_text(stmt, C_ADRES),
                            C_PRIMECHANIE, sqlite3_column_text(stmt, C_PRIMECHANIE),
                            C_FOTO, sqlite3_column_text(stmt, C_FOTO),
                            -1);
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

int LoadSearchNT()
{
    strcpy(number_telefone_data, gtk_entry_get_text(GTK_ENTRY(gNumberTelefone)));

    char* SQL = "SELECT [Код контактов], strftime('%%d.%%m.%%Y',Дата), Фамилия, Имя, Отчество, [Номер телефона], [Электронная почта], Адрес, Примечание, Фото \
                FROM Контакты WHERE [Номер телефона] IN";

    // Готовим SQL-запрос к БД
    lenstr = 0;
    lenstr+=sprintf(str, SQL);
    lenstr+=sprintf(str+lenstr, " ('%s')", number_telefone_data);
    lenstr+=sprintf(str+lenstr, " ORDER BY [Код контактов]");
    ///ShowSQL(); // Тестовый вывод запроса
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {

        gtk_list_store_append (buffer2, &iter);
        gtk_list_store_set (buffer2, &iter,
                            C_ID, sqlite3_column_int(stmt, C_ID),
                            C_DATE, sqlite3_column_text(stmt, C_DATE),
                            C_LAST_NAME, sqlite3_column_text(stmt, C_LAST_NAME),
                            C_NAME, sqlite3_column_text(stmt, C_NAME),
                            C_OTCHESTVO, sqlite3_column_text(stmt, C_OTCHESTVO),
                            C_NUMBER_TELEFONE, sqlite3_column_text(stmt, C_NUMBER_TELEFONE),
                            C_EMAIL, sqlite3_column_text(stmt, C_EMAIL),
                            C_ADRES, sqlite3_column_text(stmt, C_ADRES),
                            C_PRIMECHANIE, sqlite3_column_text(stmt, C_PRIMECHANIE),
                            C_FOTO, sqlite3_column_text(stmt, C_FOTO),
                            -1);
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Обновить запись
int ReloadSearch(int book)
{
    GtkTreeIter iter;
    //GtkTreeModel *model = gtk_tree_view_get_model (treeview1);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview2);

    gtk_tree_selection_get_selected (selection, NULL, &iter);

    // Получение записи
    char* SQL = "SELECT [Код контактов], strftime('%%d.%%m.%%Y',Дата), Фамилия, Имя, Отчество, [Номер телефона], [Электронная почта], Адрес, Примечание, Фото \
                FROM Контакты \
                WHERE [Код контактов]=";

    // Готовим SQL-запрос к БД
    lenstr = 0;
    lenstr+=sprintf(str, SQL);
    lenstr+=sprintf(str+lenstr, "%d", book);
    lenstr+=sprintf(str+lenstr, " ORDER BY [Код контактов]");
    ///ShowSQL(); // Тестовый вывод запроса
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Выполнение SQL-запроса и запись в буфер таблицы
    rc = sqlite3_step(stmt);

    gtk_list_store_set (buffer2, &iter,
                            C_ID, sqlite3_column_int(stmt, C_ID),
                            C_DATE, sqlite3_column_text(stmt, C_DATE),
                            C_LAST_NAME, sqlite3_column_text(stmt, C_LAST_NAME),
                            C_NAME, sqlite3_column_text(stmt, C_NAME),
                            C_OTCHESTVO, sqlite3_column_text(stmt, C_OTCHESTVO),
                            C_NUMBER_TELEFONE, sqlite3_column_text(stmt, C_NUMBER_TELEFONE),
                            C_EMAIL, sqlite3_column_text(stmt, C_EMAIL),
                            C_ADRES, sqlite3_column_text(stmt, C_ADRES),
                            C_PRIMECHANIE, sqlite3_column_text(stmt, C_PRIMECHANIE),
                            C_FOTO, sqlite3_column_text(stmt, C_FOTO),
                            -1);

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Кнопка Поиск
G_MODULE_EXPORT void on_button15_clicked (GtkButton *button, gpointer label)
{
    if (gtk_toggle_button_get_active(radiobutton1))
    {
        LoadSearchLN();
        ReloadSearch(book); // Обновление записи в окне Поиск
    }
    if (gtk_toggle_button_get_active(radiobutton2))
    {
        LoadSearchN();
        ReloadSearch(book); // Обновление записи в окне Поиск
    }
    if (gtk_toggle_button_get_active(radiobutton3))
    {
        LoadSearchO();
        ReloadSearch(book); // Обновление записи в окне Поиск
    }
    if (gtk_toggle_button_get_active(radiobutton4))
    {
        LoadSearchLNN();
        ReloadSearch(book); // Обновление записи в окне Поиск
    }
    if (gtk_toggle_button_get_active(radiobutton5))
    {
        LoadSearchLNNO();
        ReloadSearch(book); // Обновление записи в окне Поиск
    }
    if (gtk_toggle_button_get_active(radiobutton6))
    {
        LoadSearchNT();
        ReloadSearch(book); // Обновление записи в окне Поиск
    }
}

// Кнопка Открыть
G_MODULE_EXPORT void on_button14_clicked (GtkButton *button, gpointer label)
{
    // Определяем текущий код контактов
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model (treeview2);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview2);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        gint id_data;
        // Получение кода контактов
        gtk_tree_model_get (model, &iter,
                            C_ID, &id_data,
                            -1);
        ShowBook(id_data);
    }
}

// Кнопка Очистить
G_MODULE_EXPORT void on_button12_clicked (GtkButton *button, gpointer label)
{
    gtk_widget_destroy (wSearch);
    ShowSearch (search);
}

// Кнопка Закрыть
G_MODULE_EXPORT void on_button11_clicked (GtkButton *button, gpointer label)
{
    ReloadSearch(CurBook);
    gtk_widget_destroy (wSearch);
}

// Вывод таблицы
void ShowSearch(int search)
{
    CurBook = search;

    bSearch = gtk_builder_new ();
    gtk_builder_add_from_file (bSearch, "book.glade", NULL);

    wSearch = GTK_WIDGET(gtk_builder_get_object(bSearch, "wSearch"));
    buffer2 =  GTK_LIST_STORE(gtk_builder_get_object(bSearch, "liststore2"));
    treeview2 =  GTK_TREE_VIEW(gtk_builder_get_object(bSearch, "treeview2"));
    gLast_Name = GTK_WIDGET(gtk_builder_get_object(bSearch, "entry7"));
    gName = GTK_WIDGET(gtk_builder_get_object(bSearch, "entry8"));
    gOtchestvo = GTK_WIDGET(gtk_builder_get_object(bSearch, "entry9"));
    gNumberTelefone = GTK_WIDGET(gtk_builder_get_object(bSearch, "entry10"));
    radiobutton1 = GTK_WIDGET(gtk_builder_get_object(bSearch, "radiobutton1"));
    radiobutton2 = GTK_WIDGET(gtk_builder_get_object(bSearch, "radiobutton2"));
    radiobutton3 = GTK_WIDGET(gtk_builder_get_object(bSearch, "radiobutton3"));
    radiobutton4 = GTK_WIDGET(gtk_builder_get_object(bSearch, "radiobutton4"));
    radiobutton5 = GTK_WIDGET(gtk_builder_get_object(bSearch, "radiobutton5"));
    radiobutton6 = GTK_WIDGET(gtk_builder_get_object(bSearch, "radiobutton6"));

    gtk_window_set_title (GTK_WINDOW (wSearch), "Записная книга 2");
    gtk_window_set_default_size (GTK_WINDOW (wSearch), 700, 400);
    gtk_window_set_position(GTK_WINDOW(wSearch), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (bSearch, NULL);

    gtk_widget_show_all(wSearch);

}
