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
GtkWidget   *button1, *button2, *button3, *button4, *button5, *button6, *button7, *button8, *button9;

GtkWidget   *radiobutton1, *radiobutton2, *radiobutton3, *radiobutton4;

GtkWidget   *area1, *area2;

GtkSpinButton   *Resis1, *Resis2, *Conden1, *Conden2;

float NumOfPoints, ConOfPoints; //количество очков

char str[1000];

#endif // MAIN_H_INCLUDED
