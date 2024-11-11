#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <gtk-4.0/gtk/gtk.h>
#include <math.h>
#include <ctype.h>
#include <sqlite3.h>
//#include "settings.h"

// Структуры данных для базы данных
sqlite3 *db;    // хэндл БД
sqlite3_stmt* stmt; // строка запроса к БД
char *err;
int rc;

//-----------------------
//	global objects
//-----------------------

GtkWidget	*wMain, *wSettings;
GtkWidget	*GtkBox1, *GtkBox2;
GtkWidget	*button1, *button2, *button4;

GtkBuilder	*bMain, *bSettings;
GtkApplication	*app;


void		on_window_destroy(GtkWidget *w);
void		on_window_close_request(GtkWidget *w);

char str[1500];
int lenstr;

void StartDB();
void EndDB();
void ShowError();
int ShowSQL();


#endif // MAIN_H_INCLUDED
