// Linux libm -lm
// gcc calc.c -o calc.exe -lm 

#include <stdio.h>
#include <math.h>

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
		
		else if(z == '^' ) s = powl(x, y);
				
		else (printf("Error"));

	}	printf("%lf %c %lf = %lf\n", x, z, y, s);
	
	return 0;
}
