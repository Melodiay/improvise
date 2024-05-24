#include <stdio.h>
#include <math.h>
#include <stdlib.h>


double R = 0.00;
double H = 0.00;
double M = 0.00;
double S = 0.00;
int hh = 0;
int mm = 0;
int ss = 0;
char q = 'q';

int main()
{
    double A;
    double B;

    printf("Введите растояние: S = ");
    scanf("%lf", &A);
    printf("Введите скорость:  V = ");
    scanf("%lf", &B);

    if (B!=0)
    {
            // Отображение Результата (Ответ)
            R = (A/B);
            H = R;
            hh = H;
            //printf("%i Ч", hh);
            H = H - (int)H;
            M = H * 60;
            mm = M;
            //printf("%i Мин", mm);
            M = M - (int)M;
            S = M * 60;
            ss = S;
            //printf("%i Сек", ss);
    printf("Время: T = %i Ч %i Мин %i Сек \n", hh, mm, ss);
    printf("Для выхода введите q: Продолжить нажмите Enter:");
    scanf("%c", &q);
    }
while(getchar() != 'q')
{
    printf("Введите растояние: S = ");
    scanf("%lf", &A);
    printf("Введите скорость:  V = ");
    scanf("%lf", &B);

    if (B!=0)
    {
            // Отображение Результата (Ответ)
            R = (A/B);
            H = R;
            hh = H;
            //printf("%i Ч", hh);
            H = H - (int)H;
            M = H * 60;
            mm = M;
            //printf("%i Мин", mm);
            M = M - (int)M;
            S = M * 60;
            ss = S;
            //printf("%i Сек", ss);
    printf("Время: T = %i Ч %i Мин %i Сек \n", hh, mm, ss);
    printf("Для выхода введите q: Продолжить нажмите Enter:");
    scanf("%c", &q);
    }
	}
    return 0;
}
