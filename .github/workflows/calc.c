#include <stdio.h>

int main()
{
	int i;

	double  s, x, y;

	char z;

	printf("x znak y = ");

	scanf("%lf %c %lf", &x, &z, &y);

	for (i=0; i<4; i++)

	{

		if(z == '+') s = x + y;
			     
		else if(z == '-' ) s = x - y;
				  
		else if(z == '*' ) s = x * y;
				  
		else if(z == '/' ) s = x / y;
				  
		else (printf("Error"));


	}	printf("%lf \n", s);
	
	return 0;
}
