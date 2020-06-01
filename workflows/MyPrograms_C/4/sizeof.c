#include <stdio.h>
#include <limits.h>


int main()

{	

	printf("short int... \tsize: %ld bytes \t", sizeof(short int));

	printf("от %d до %d \n", SHRT_MAX, SHRT_MIN);

	printf("Int...\t \tsize: %ld bytes \t", sizeof(int));
	
	printf("от %d до %d \n", INT_MAX, INT_MIN);

	printf("long int... \tsize: %ld bytes \t", sizeof(long int));
	
	printf("от %ld до %ld \n", LONG_MAX, LONG_MIN);
	
	printf("char... \tsize: %ld byte \n", sizeof(char));
	
	printf("float... \tsize: %ld bytes \n", sizeof(float));

	printf("double... \tsize: %ld bytes \n", sizeof(double));

	return 0;
}
