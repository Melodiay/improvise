#include <stdio.h>
#include <string.h>

int main()

{

	char s1[] = "Larger text string", s2[] = "Smaller string";

	printf( "\n%s: %ld element", s1, sizeof(s1) );

	printf( ", %ld characters\n", strlen(s1) );

	strcpy(s1, s2);

	printf( "\n%s: %ld element", s1, sizeof(s1) );

	printf( ", %ld characters\n", strlen(s1) );

	strncpy( s1, s2, 5 ); s1[5] = '\0';

	printf( "\n%s: %ld element", s1, sizeof(s1) );

	printf( ", %ld characters\n", strlen(s1) );
	
	return 0;
	
}
