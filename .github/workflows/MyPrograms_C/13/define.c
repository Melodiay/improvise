#include <stdio.h>

#define LINE "_______________"
#define TITLE "C Programming in easy steps"
#define AUTHOR "Mike MCGrath"

/*
#ifdef _WIN32
#ifdef SYSTEM "Windows"
#endif
*/

#ifdef linux
#define SYSTEM "Linux"
#endif


int main()

{

	printf( "\n \t %s \n \t %s \n", LINE, TITLE );

	printf( "\t by %s \n \t %s \n", AUTHOR, LINE );

	printf( "\n Operating System: %s \n", SYSTEM );

	return 0;

}
