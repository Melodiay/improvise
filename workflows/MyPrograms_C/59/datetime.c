#include <stdio.h>
#include <time.h>

int main()

{

    char buffer[100];

    time_t elapsed;

    struct tm *now;

    elapsed = time(NULL);

    now = localtime(&elapsed);

    printf( "\n" );

    printf( "%s\n", asctime(now) );

    strftime ( buffer, 100, "Today is %A, %B %d.\n", now );

    printf( "%s", buffer );

    strftime ( buffer, 100, "The time is %I:%M %p.\n", now );

    printf( "%s", buffer );

    printf( "\n" );

    return 0;

}