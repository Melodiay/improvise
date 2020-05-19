// Linux libm -lm
// gcc calc.c -o calc.exe -lm 

#include <stdio.h>
#include <math.h>

int main()
{
	int i, a, b, sm;
	int num;
	double  s, x, y;

	char z;	

    while(1) {
	
	printf("\n");	

	printf("Enter 1 to calculate, use the following operations +, -, *, /, ^ \n");
	
	printf("Enter 2 to find out the remainder use the operation %% \n");
	
	printf("Enter 3 to exit!\n");

	scanf("%d", &num);

         

	switch(num)		
	{
		case 1: printf("x znak y = "); 
		{
			scanf("%lf %c %lf", &x, &z, &y);

		for (i=0; i<5; i++)

			{

			if(z == '+') s = x + y;
			     
			else if(z == '-' ) s = x - y;
				  
			else if(z == '*' ) s = x * y;
				  
			else if(z == '/' ) s = x / y;

			else if(z == '^' ) s = powl(x, y);
		  
			else (printf("Error"));

		}	printf("%lf %c %lf = %lf\n", x, z, y, s);
		
		} break;
		
		case 2: printf("x znak y = ");

		{

			scanf("%d %c %d", &a, &z, &b);

			for (i=0; i<1; i++){
	
			if(z == '%' ) sm = a % b;
		
		}  printf("%d %c %d = %d\n", a, z, b, sm);	

		
		} break;
		
		case 3: printf("Exit\n"); { return 0; } break;		
		
		default: printf("The following commands are available 1, 2, 3\n");
	}	

	}	
	
	return 0;
}


