#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>
#include <windows.h>
#include "r.h"
#include "c.h"

GtkWidget   *wMain, *wResistors, *wCondens;
GtkBuilder  *bMain, *bResistors, *bCondens;
GtkWidget   *button1, *button2, *button3, *button4, *button5, *button6, *button7;
GtkWidget   *label3;
GtkWidget   *radiobutton1, *radiobutton2;

GtkWidget   *area2;

GtkSpinButton   *Resis1, *Resis2, *Sum;

float NumOfPoints; //количество очков

char str[1000];
char str_2[100];
char str_3[100];

#endif // MAIN_H_INCLUDED
