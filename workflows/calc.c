// Linux libm -lm // gcc calc.c -o calc.exe -lm

#include <stdio.h>
#include <math.h>

int main()
{
    int i, a, b, sm = 0;

    int num;

    long double  s = 0, x, y;

    long double sqrtl (long double x);

    char z;

    while( (num = 1) && (num = 2) && (num = 3) && (num = 4)) {

    printf("\n");

    printf("Enter 1 to calculate, use the following operations +, -, *, /, ^ \n");

    printf("Enter 2 to find out the remainder use the operation %% \n");

    printf("Enter 3 to calculate the root of x (√x) \n");

    printf("Enter 4 to exit!\n\n");

    scanf("%d", &num);

    switch(num)
    {
        case 1: printf("\nx znak y = ");

        {
            scanf("%Lf %c %Lf", &x, &z, &y);

        for (i=0; i<5; i++)

            {

            if(z == '+') { s = x + y; printf("%Lf %c %Lf = %Lf\n", x, z, y, s); break; }

            else if(z == '-' ) { s = x - y; printf("%Lf %c %Lf = %Lf\n", x, z, y, s); break; }

            else if(z == '*' ) { s = x * y; printf("%Lf %c %Lf = %Lf\n", x, z, y, s); break; }

            else if(z == '/' ) 

		 if ( y != 0 ) 

		 {	
			
			s = x / y;  printf("%Lf %c %Lf = %Lf\n", x, z, y, s); break;	
			
		 }

		 else { printf("You can't divide by zero\n");break; }

            else if(z == '^' ) 

		 if ( x == 0 && y == 0 )

		 {
			printf("Zero to the zero degree is not defined\n");break;

		 }

		 else { s = powl(x, y); printf("%Lf %c %Lf = %Lf\n", x, z, y, s); break; }

            else { printf("Error\n"); break; }

        }

        } break;

        case 2: printf("\nx znak y = ");

        {

            scanf("%d %c %d", &a, &z, &b);

            for (i=0; i<1; i++){

            if(z == '%' ) sm = a % b;

        }  printf("%d %c %d = %d\n", a, z, b, sm);


        } break;

	case 3: printf("\nsqrt x = ");
        {
            scanf("%Lf", &x);

	    for (i=0; i<1; i++)

            {

		 s = sqrtl(x);

	    }  printf("%Lf = %Lf\n", x, s);

	} break;

    case 4: printf("\nExit\n"); { return 0; } break;

        default: printf("\nThe following commands are available 1, 2, 3\n");

    }

    }

}
