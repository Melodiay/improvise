#include <stdio.h>

int main()
{

	if ( 5 > 1 ) { printf( "Yes, 5 is greater than 1\n" ); }

	if ( 5 > 1 )
	{
	 	if ( 7 > 2 )

		{ printf( "5 is greater than 1 and 7 is greater than 2\n" ); }
	}

	if ( 1 > 2 )

	{ printf( "1st Expression is true\n" ); }

	else if( 1 > 3 )
	
	{ printf( "2nd expression is true\n" ); }

	else

	{ printf( "Both expression are false\n" ); }
	
	return 0;
}
