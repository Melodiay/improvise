#include <stdio.h>

int main()
{
	int i;

	float  s, x, y;

	char z;

	printf("x znak y = ");

	scanf("%f %c %f", &x, &z, &y);

	for (i=0; i<4; i++)

	{

		if(z == '+') s = x + y;
			     
		else if(z == '-' ) s = x - y;
				  
		else if(z == '*' ) s = x * y;
				  
		else if(z == '/' ) s = x / y;
				  
		else (printf("Error"));


	}	printf("%f \n", s);
	
	return 0;
}
