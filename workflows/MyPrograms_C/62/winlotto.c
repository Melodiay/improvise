// gcc winlotto.c -o winlotto.exe -lX11 -L/usr/X11R6/lib 
 
// ./winlotto.exe

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define X 0
#define Y 0
#define WIDTH 400
#define HEIGHT 200
#define WIDTH_MIN 50
#define HEIGHT_MIN 50
#define BORDER_WIDTH 5
#define TITLE "Lotto Number Picker"
#define ICON_TITLE "Lotto Number Picker"
#define PRG_CLASS "Lotto Number Picker"

extern int errno;

/*
* SetWindowManagerHints - функция, которая передает информацию о
* свойствах программы менеджеру окон. 
*/

static void SetWindowManagerHints ( 
 Display * display, 		/*Указатель на структуру Display */
 char * PClass, 		/*Класс программы */
 char * argv[],   		/*Аргументы программы */
 int argc,    			/*Число аргументов */
 Window window,    		/*Идентификатор окна */
 int x,     			/*Координаты левого верхнего */
 int y,	   				/*угла окна */
 int win_wdt,			/*Ширина  окна */
 int win_hgt,  			/*Высота окна */
 int win_wdt_min,			/*Минимальная ширина окна */
 int win_hgt_min, 		/*Минимальная высота окна */
 char * ptrTitle,  		/*Заголовок окна */
 char * ptrITitle,	/*Заголовок пиктограммы окна */
 Pixmap pixmap 	/*Рисунок пиктограммы */
)
{
 XSizeHints size_hints; /*Рекомендации о размерах окна*/

 XWMHints wm_hints;
 XClassHint class_hint;
 XTextProperty windowname, iconname;

 if ( !XStringListToTextProperty (&ptrTitle, 1, &windowname ) ||
    !XStringListToTextProperty (&ptrITitle, 1, &iconname ) ) {
  puts ( "No memory!\n");
  exit ( 1 );
}

size_hints.flags = PPosition | PSize | PMinSize;
size_hints.min_width = win_wdt_min;
size_hints.min_height = win_hgt_min;
wm_hints.flags = StateHint | IconPixmapHint | InputHint;
wm_hints.initial_state = NormalState;
wm_hints.input = True;
wm_hints.icon_pixmap= pixmap;
class_hint.res_name = argv[0];
class_hint.res_class = PClass;

XSetWMProperties ( display, window, &windowname,
  &iconname, argv, argc, &size_hints, &wm_hints,
  &class_hint );
}

/* main - основная функция программы */
int main( int argc, char *argv[] )

{

    int s;

    int i, r, temp, nums[50];

    char buf[4], str[50] = { "Your Six Lucky Numbers Are: " };
    	
    srand(time(NULL));

    for(i = 0; i < 50; i++) { nums[i] = i; }

    for(i = 1; i < 50; i++)

    {

	r = (rand() % 49 ) + 1;

	temp = nums[i]; nums[i] = nums[r]; nums[r] = temp;

    }

    for(i = 1; i < 7; i++)

    {

	sprintf(buf, "%d", nums[i]);

	strcat(buf, " "); strcat(str, buf);

    }

 Display * display;  /* Указатель на структуру Display */
 int screen_number;    /* Номер экрана */
 GC gc;				/* Графический контекст */
 XEvent report;
 Window window;

 /* Устанавливаем связь с сервером */
 if ( ( display = XOpenDisplay ( NULL ) ) == NULL ) {
  puts ("Can not connect to the X server!\n");
  exit ( 1 );
 }

 /* Получаем номер основного экрана */
 screen_number = DefaultScreen ( display );

 /* Создаем окно */
 window = XCreateSimpleWindow ( display,
     RootWindow ( display, screen_number ),
     X, Y, WIDTH, HEIGHT, BORDER_WIDTH,
     BlackPixel ( display, screen_number ),
     WhitePixel ( display, screen_number ) );

 /* Задаем рекомендации для менеджера окон */
 SetWindowManagerHints ( display, PRG_CLASS, argv, argc,
   window, X, Y, WIDTH, HEIGHT, WIDTH_MIN,
   HEIGHT_MIN, TITLE, ICON_TITLE, 0 );

 /* Выбираем события,  которые будет обрабатывать программа */
 XSelectInput ( display, window, ExposureMask | KeyPressMask );

 /* Покажем окно */
 XMapWindow ( display, window );

 /* Создадим цикл получения и обработки ошибок */
 while ( 1 ) {
  XNextEvent ( display, &report );

  switch ( report.type ) {
    case Expose :
     /* Запрос на перерисовку */
     if ( report.xexpose.count != 0 )
      break;

     gc = XCreateGC ( display, window, 0 , NULL );

     XSetForeground ( display, gc, BlackPixel ( display, 0) );
     XDrawString ( display, window, gc, 20,50,
       str, strlen ( str ) );
     XFreeGC ( display, gc );
     XFlush(display);
     break;

    case KeyPress :
     /* Выход нажатием клавиши клавиатуры */
     XCloseDisplay ( display );
     exit ( 0 );
  }
 }
}

