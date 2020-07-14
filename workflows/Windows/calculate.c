// Windows gcc calc.c -o calc.exe

#include <stdio.h>
#include <math.h>


double s;

double add( float, float );
double sub( float, float );
double mul( float, float );
double div( float, float );
double deg( int, int );
int mod( int, int );

int main()
{
	int i, x, y, mod_s = 0, num;
	
	float a, b;
	
	char z;
	
	while( (num = 1) && (num = 2)  && (num = 3) ) 

{

    printf("\n");

    printf("Enter 1 to calculate, use the following operations +, -, *, /, ^ \n");

    printf("Enter 2 to find out the remainder use the operation %% \n");

    printf("Enter 3 to exit!\n\n");

    scanf("%d", &num);

	
	switch(num)	
	
	{
		case 1:	printf("Enter a znak b: "); 
		
		{ 
		
			scanf( "%f %c %f", &a, &z, &b ); 
			
			

			

				if(z == '+') { s=add(a, b); printf("%f %c %f= %lf\n", a, z, b, s); break; }

				else if(z == '-' ) { s=sub(a, b); printf("%f %c %f= %lf\n", a, z, b, s); break; }

				else if(z == '*' ) { s = mul(a, b); printf("%f %c %f= %lf\n",  a, z, b, s); break; }

				else if(z == '/' ) 
	
					{
						
						if ( b != 0 ) 

						{	
			
							s = div( a, b );  printf("%f %c %f= %lf\n",  a, z, b, s); break;	
			
						}

						else { printf("You can't divide by zero\n");break; }

					}

				else if(z == '^' ) 
				
					{

						if ( a == 0 && b == 0 )

						{
						
							printf("Zero to the zero degree is not defined\n"); break;

						} 

						else { s = deg( a, b ); printf("%lf %c %lf = %lf\n", a, z, b, s); break; }
			
					}
				
				else { printf("Error\n"); break; }

			
		
		}
		
		case 2: printf("Enter a znak b: "); 
		
		{ 
		
			scanf( "%d %c %d", &x, &z, &y );

			if(z == '%' ) { mod_s = mod( x, y ); printf("%d %c %d = %d\n", x, z, y, mod_s); break; }
		
		}
		
		case 3: printf("\nExit\n"); { return 0; } break;
	
		default: printf("\nThe following commands are available 1, 2, 3\n");
	
	}
		
}
	
}	

double add ( float a, float b )
{

	return a+b;

}

double sub ( float a, float b )
{

	return a-b;

}

double mul ( float a, float b )
{

	return a*b;

}

double div ( float a, float b )
{

	return a/b;

}

double deg ( int a, int b )
{

	return powl( a, b );

}

int mod( int x, int y )
{
	
	return  x % y;
	
}
