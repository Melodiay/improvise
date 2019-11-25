#include <stdio.h>
#include <string.h>

int main()

{

    FILE *file_ptr;

    char text[50];

    file_ptr = fopen("farewell.txt", "r+a");

    if(file_ptr != NULL)

    {

	printf( "\nFile farewell.text opened\n\n" );

	while( fgets(text, 50, file_ptr) )	{ printf("%s", text); }

	strcpy( text, "...by Lord Alfred Tennyson\n" );

	printf("\n");

	fputs(text, file_ptr);

	fclose(file_ptr);

	return 0;

    }

    else { printf( "Unable to open file\n" ); return 1; }

}