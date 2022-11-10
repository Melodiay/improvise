/**********************************************************************
* Проект:
* Автор: Алексей Быков
**********************************************************************/
#include "main.h"

// Структуры данных для виджета
GtkListStore        *buffer;    // Буфер таблицы
GtkTreeView         *treeview1; // Таблица
GtkTreeViewColumn   *column;    // Отображаемая колонка
GtkTreeIter         iter;       // Итератор таблицы (текущая строка)
GtkCellRenderer     *renderer;  // Рендер таблицы (текущая ячейка)

// Обозначение полей
enum
{
    C_ID, C_LAST_NAME, C_NAME, C_OTCHESTVO, C_NUMBER_TELEFONE, C_ADRES, C_PRIMECHANIE, C_FOTO
};

int search;

int LoadBooks()
{
    char* SQL = "SELECT [Код контактов], Фамилия, Имя, Отчество, [Номер телефона], Адрес, Примечание, Фото \
                FROM Контакты ORDER BY [Код контактов]";
    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, SQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        gtk_list_store_append (buffer, &iter);
        gtk_list_store_set (buffer, &iter,
                            C_ID, sqlite3_column_int(stmt, C_ID),
                            C_LAST_NAME, sqlite3_column_text(stmt, C_LAST_NAME),
                            C_NAME, sqlite3_column_text(stmt, C_NAME),
                            C_OTCHESTVO, sqlite3_column_text(stmt, C_OTCHESTVO),
                            C_NUMBER_TELEFONE, sqlite3_column_text(stmt, C_NUMBER_TELEFONE),
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
int Reload(int book)
{
    GtkTreeIter iter;
    //GtkTreeModel *model = gtk_tree_view_get_model (treeview1);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview1);

    gtk_tree_selection_get_selected (selection, NULL, &iter);

    // Получение записи
    char* SQL = "SELECT [Код контактов], Фамилия, Имя, Отчество, [Номер телефона], Адрес, Примечание, Фото \
                FROM Контакты \
                WHERE [Код контактов]=";

    // Готовим SQL-запрос к БД
    lenstr = 0;
    lenstr+=sprintf(str, SQL);
    lenstr+=sprintf(str+lenstr, "%d", book);
    lenstr+=sprintf(str+lenstr, " ORDER BY Фамилия");
    ///ShowSQL(); // Тестовый вывод запроса
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Выполнение SQL-запроса и запись в буфер таблицы
    rc = sqlite3_step(stmt);

    gtk_list_store_set (buffer, &iter,
                            C_ID, sqlite3_column_int(stmt, C_ID),
                            C_LAST_NAME, sqlite3_column_text(stmt, C_LAST_NAME),
                            C_NAME, sqlite3_column_text(stmt, C_NAME),
                            C_OTCHESTVO, sqlite3_column_text(stmt, C_OTCHESTVO),
                            C_NUMBER_TELEFONE, sqlite3_column_text(stmt, C_NUMBER_TELEFONE),
                            C_ADRES, sqlite3_column_text(stmt, C_ADRES),
                            C_PRIMECHANIE, sqlite3_column_text(stmt, C_PRIMECHANIE),
                            C_FOTO, sqlite3_column_text(stmt, C_FOTO),
                            -1);

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Кнопка Открыть
G_MODULE_EXPORT void on_button7_clicked (GtkButton *button, gpointer label)
{
    // Определяем текущий код контактов
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model (treeview1);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview1);

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

// Добавить запись
int AddItem ()
{
    // Вставка новой записи в базу
    char* SQL="INSERT INTO Контакты DEFAULT VALUES;";
    // Записываем в БД
    if (sqlite3_exec(db, SQL, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return 1;
    }
    // Получение последней вставленной строки
    char* SQL2 = "SELECT [Код контактов], Фамилия, Имя, Отчество, [Номер телефона], Адрес, Примечание \
                FROM Контакты ORDER BY [Код контактов]  DESC LIMIT 1";

    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, SQL2, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Выполнение SQL-запроса и запись в буфер таблицы
    rc = sqlite3_step(stmt);

    gtk_list_store_append (buffer, &iter);
    gtk_list_store_set (buffer, &iter,
                        C_ID, sqlite3_column_int(stmt, C_ID),
                        C_LAST_NAME, sqlite3_column_text(stmt, C_LAST_NAME),
                        C_NAME, sqlite3_column_text(stmt, C_NAME),
                        C_OTCHESTVO, sqlite3_column_text(stmt, C_OTCHESTVO),
                        C_NUMBER_TELEFONE, sqlite3_column_text(stmt, C_NUMBER_TELEFONE),
                        C_ADRES, sqlite3_column_text(stmt, C_ADRES),
                        C_PRIMECHANIE, sqlite3_column_text(stmt, C_PRIMECHANIE),
                        C_FOTO, sqlite3_column_text(stmt, C_FOTO),
                        -1);

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Удаление записи
void DeleteItem ()
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model (treeview1);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview1);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        gint id_data;

        // Получение кода сотрудника
        gtk_tree_model_get (model, &iter,
                            C_ID, &id_data,
                            -1);

        // Удаление строки из буфера
        gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

        // Удаление записи из БД
        lenstr = 0;
        lenstr+=sprintf(str, "DELETE FROM Контакты WHERE [Код контактов]=");
        lenstr+=sprintf(str+lenstr, "%d", id_data);
        lenstr+=sprintf(str+lenstr, ";");
        ///ShowSQL(); // Тестовый вывод запроса
        // Записываем в БД
        if (sqlite3_exec(db, str, 0, 0, &err))
        {
            sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
            sqlite3_free(err);
            return;
        }
    }
}

// Кнопка Поиск
G_MODULE_EXPORT void on_button8_clicked (GtkButton *button, gpointer label)
{
    ShowSearch (search);
}

// Кнопка Добавить
G_MODULE_EXPORT void on_button6_clicked (GtkButton *button, gpointer label)
{
    AddItem ();
}

// Кнопка Удалить
G_MODULE_EXPORT void on_button5_clicked (GtkButton *button, gpointer label)
{
    DeleteItem ();
}

// Кнопка Закрыть
G_MODULE_EXPORT void on_button4_clicked (GtkButton *button, gpointer label)
{
    gtk_widget_destroy (wBooks);
}

// Вывод таблицы
void ShowBooks()
{
    bBooks = gtk_builder_new ();
    gtk_builder_add_from_file (bBooks, "book.glade", NULL);
    wBooks = GTK_WIDGET(gtk_builder_get_object(bBooks, "wBooks"));
    buffer =  GTK_LIST_STORE(gtk_builder_get_object(bBooks, "liststore1"));
    treeview1 =  GTK_TREE_VIEW(gtk_builder_get_object(bBooks, "treeview1"));

    gtk_window_set_title (GTK_WINDOW (wBooks), "Записная книга");
    gtk_window_set_default_size (GTK_WINDOW (wBooks), 700, 300);
    gtk_window_set_position(GTK_WINDOW(wBooks), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (bBooks, NULL);

    if(LoadBooks())
    {
        ShowError();
    }
    gtk_widget_show_all(wBooks);
}
