// gcc winlotto.c -o winlotto.exe -lX11 
// ./winlotto.exe

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <X11/Xlib.h>

extern int errno;

int main( void )

{

    Display *d; 
    Window w; 
    XEvent e; 
     
    int s;

    int i, r, temp, nums[50];

    char buf[4], str[50] = { "Your Six Lucky Numbers Are: " };
    
    char *name = "Lotto Number Picker";	
	
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

    if( ( d = XOpenDisplay( getenv("DISPLAY" ) ) ) == NULL ) {  // Соединиться с X сервером, 
      printf( "Can't connect X server: %s\n", strerror( errno ) ); 
      exit( 1 ); 
   } 
   s = DefaultScreen( d ); 
   w = XCreateSimpleWindow( d, RootWindow( d, s ),     // Создать окно 
                            10, 10, 400, 200, 1, 
                            BlackPixel( d, s ), WhitePixel( d, s ) ); 
   XSelectInput(  d, w, ExposureMask | KeyPressMask );  // На какие события будем реагировать? 
   XMapWindow( d, w );                                 // Вывести окно на экран 
   while( 1 ) {                                        // Бесконечный цикл обработки событий 
      XNextEvent( d, &e ); 
      if( e.type == Expose ) {                         // Перерисовать окно 
         XFillRectangle( d, w, DefaultGC( d, s ), 20, 20, 10, 10 ); 
	 XStoreName(d, w, name);
         XDrawString( d, w, DefaultGC( d, s ), 50, 50, str, strlen( str ) ); 
      } 
      if( e.type == KeyPress )                         // При нажатии кнопки - выход 
         break; 
   }
   XCloseDisplay( d );                                 // Закрыть соединение с X сервером 
   return 0; 
} 



