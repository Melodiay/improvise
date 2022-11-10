/**********************************************************************
* Проект: Записная книга
* Автор: Алексей Быков
* Настройки
**********************************************************************/
#include "main.h"

GtkWidget *gCompany, *gTelefon, *gAdres;

enum
{
    C_ID_SETTINGS, C_COMPANY, C_TELEFON, C_ADRES
};

// Загрузка строки накладной
int LoadSettings()
{

    char* SQL = "SELECT [Код настройки], Компания, Телефон, Адрес FROM Настройки";

    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, SQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        ShowSQL();
        sqlite3_finalize(stmt);
        return 3;
    }
    // Выполнение SQL-запроса
    rc = sqlite3_step(stmt);
    // Запись в виджет
    if (sqlite3_column_text(stmt, C_COMPANY)>0)
    {
        sprintf(str, "%s",sqlite3_column_text(stmt, C_COMPANY));
    }
    else
    {
        sprintf(str, " ");
    }
    gtk_entry_set_text(GTK_ENTRY(gCompany),str);

    sprintf(str, "%s",sqlite3_column_text(stmt, C_TELEFON));
    gtk_entry_set_text(GTK_ENTRY(gTelefon),str);

    sprintf(str, "%s",sqlite3_column_text(stmt, C_ADRES));
    gtk_entry_set_text(GTK_ENTRY(gAdres),str);

    sqlite3_finalize(stmt);
    return 0;
}

// Сохранение записи
void SaveSettings()
{
    // Готовим SQL-запрос
    lenstr = 0;
    lenstr+=sprintf(str, "UPDATE [Настройки] SET ");
    lenstr+=sprintf(str+lenstr, " Компания = '%s',", gtk_entry_get_text(GTK_ENTRY(gCompany)));
    lenstr+=sprintf(str+lenstr, " Телефон = '%s',", gtk_entry_get_text(GTK_ENTRY(gTelefon)));
    lenstr+=sprintf(str+lenstr, " Адрес = '%s'", gtk_entry_get_text(GTK_ENTRY(gAdres)));
    ///ShowSQL(); // Тестовый вывод запроса
    // Записываем в БД
    if (sqlite3_exec(db, str, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        ShowSQL();
        sqlite3_free(err);
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);
}

// Кнопка Закрыть
G_MODULE_EXPORT void on_button10_clicked (GtkButton *button, gpointer label)
{
    SaveSettings(); // Запись значений в БД
    gtk_widget_destroy (wSettings);
}

// Вывод окна
void ShowSettings()
{
    bSettings = gtk_builder_new ();
    gtk_builder_add_from_file (bSettings, "book.glade", NULL);
    wSettings = GTK_WIDGET(gtk_builder_get_object(bSettings, "wSettings"));

    gCompany = GTK_WIDGET(gtk_builder_get_object(bSettings, "gCompany"));
    gTelefon = GTK_WIDGET(gtk_builder_get_object(bSettings, "gTelefon"));
    gAdres = GTK_WIDGET(gtk_builder_get_object(bSettings, "gAdres"));

    gtk_window_set_title (GTK_WINDOW (wSettings), "Записная книга. Автор:  Алексей Быков");
    gtk_window_set_default_size (GTK_WINDOW (wSettings), 700, 300);
    gtk_window_set_position(GTK_WINDOW(wSettings), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (bSettings, NULL);

    LoadSettings();

    gtk_widget_show_all(wSettings);
}
