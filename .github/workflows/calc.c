// Linux libm -lm // gcc calc.c -o calc.exe -lm

// Windows gcc calc.c -o calc.exe

#include <stdio.h>
#include <math.h>

int main()
{
    int i, a, b, sm = 0;
    int num;
    long double  s = 0, x, y;

    char z;

    while((num = 1) && (num = 2) &&  (num = 3)) {

    printf("\n");

    printf("Enter 1 to calculate, use the following operations +, -, *, /, ^ \n");

    printf("Enter 2 to find out the remainder use the operation %% \n");

    printf("Enter 3 to exit!\n\n");

    scanf("%d", &num);

    switch(num)
    {
        case 1: printf("\nx znak y = ");
        {
            scanf("%Lf %c %Lf", &x, &z, &y);

        for (i=0; i<5; i++)

            {

            if(z == '+') s = x + y;

            else if(z == '-' ) s = x - y;

            else if(z == '*' ) s = x * y;

            else if(z == '/' ) s = x / y;

            else if(z == '^' ) s = powl(x, y);

            else (printf("Error"));

        }	printf("%Lf %c %Lf = %Lf\n", x, z, y, s);

        } break;

        case 2: printf("\nx znak y = ");

        {

            scanf("%d %c %d", &a, &z, &b);

            for (i=0; i<1; i++){

            if(z == '%' ) sm = a % b;

        }  printf("%d %c %d = %d\n", a, z, b, sm);


        } break;

    case 3: printf("\nExit\n"); { return 0; } break;

        default: printf("\nThe following commands are available 1, 2, 3\n");

    }

    }

}


