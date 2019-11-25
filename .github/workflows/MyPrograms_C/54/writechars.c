#include <stdio.h>

int main()

{

    FILE *file_ptr; int i;

    char text[50] = { "Text, one character at a time." };

    file_ptr = fopen("chars.txt", "w");

    if(file_ptr != NULL)

    {

	printf( "\nFile chars.text created\n\n" );

	for(i = 0; text[i]; i++) { fputc(text[i], file_ptr); }

	fclose(file_ptr);

	return 0;

    }

    else

    {

	printf( "Unable to create file\n" ); return 1;

    }

}