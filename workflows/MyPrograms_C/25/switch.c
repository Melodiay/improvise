#include <stdio.h>

int main()

{

	int num = 2; char letter = 'b';

	switch(num)

	{

		case 1 : printf( "Nuber is one\n" ); break;
		
		case 2 : printf( "Nuber is two\n" ); break;
	
		case 3 : printf( "Nuber is fhree\n" ); break;

		default : printf( "Nuber is unrecognized\n" ); 	
	
	}

	switch(letter)

	{

		case 'a' : case 'b' : case 'c' :

		printf( "Letter is %c\n", letter ); break;

		default : printf( "Letter is unrecognized\n" );	

	}

	return 0;

}
