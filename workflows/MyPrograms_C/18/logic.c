#include <stdio.h>


int main()

{

	int yes = 1, no = 0; 
/* Автор перепутал пишет yes = 0, no = 1; Хотя в книге и есть не точности где-то автор не внимательный. Ещё автор пишет "В программировании на языке C 0 представляет собой значение false,
а любое ненулевое значение, например, 1 — значение false." Но насамом деле 1 и выше имеет значение true*/

	
	printf( "AND (no&&no): %d \n", no && no );

	printf( "AND (yes&&no): %d \n", yes && no );

	printf( "AND (yes&&yes): %d \n", yes && yes );

	printf( "OR (no||no): %d \n", no || no );

	printf( "OR (yes||no): %d \n", yes || no );

	printf( "OR (yes||yes): %d \n", yes || yes );

	printf( "NOT (yes !yes): %d %d\n", yes, !yes);

	printf( "NOT (no !no): %d %d\n", no, !no );


	return 0; 

}
