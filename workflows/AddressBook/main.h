#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdlib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <string.h>
#include <math.h>
#include "settings.h"
#include "book.h"
#include "books.h"
#include "search.h"

// Структуры данных для базы данных
sqlite3 *db;    // хэндл БД
sqlite3_stmt* stmt; // строка запроса к БД
char *err;
int rc;

// Структуры данных для окон
GtkWidget   *wMain, *wSettings, *wBook, *wBooks, *wSearch;
GtkBuilder  *bMain, *bSettings, *bBook, *bBooks, *bSearch;

GtkWidget *gFoto;

GtkWidget *radiobutton1, *radiobutton2, *radiobutton3, *radiobutton4, *radiobutton5, *radiobutton6;

char str[1000];
int lenstr;

void StartDB();
void EndDB();
void ShowError();
int ShowSQL();


#endif // MAIN_H_INCLUDED
