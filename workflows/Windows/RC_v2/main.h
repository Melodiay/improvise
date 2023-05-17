#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>
#include <windows.h>
#include "r.h"
#include "c.h"
#include "tsv.h"

GtkWidget   *wMain, *wResistors, *wCondens, *wTsv;
GtkBuilder  *bMain, *bResistors, *bCondens, *bTsv;
GtkWidget   *button1, *button2, *button3, *button4, *button5, *button6, *button7, *button8, *button9, *button10, *button11, *button12, *button13;
GtkWidget   *label7, *label8, *label9, *label10, *label11, *label12;
GtkWidget   *radiobutton1, *radiobutton2, *radiobutton3, *radiobutton4, *radiobutton5, *radiobutton6, *radiobutton7;

GtkWidget   *area1, *area2, *area3;

GtkSpinButton   *Resis1, *Resis2, *Conden1, *Conden2, *a1, *b2;

double NumOfPoints, ConOfPoints; //количество очков

char str[1000];

#endif // MAIN_H_INCLUDED
