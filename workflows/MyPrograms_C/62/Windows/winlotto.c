// gcc winlotto.c -o winlotto.exe 
// winlotto.exe

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <windows.h> 

extern int errno;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)

{
	
    int s;

    int i, r, temp, nums[50];

    char buf[4], str[50] = { "Your Six Lucky Numbers Are: " };

    srand(time(NULL));

    for(i = 0; i < 50; i++) { nums[i] = i; }

    for(i = 1; i < 50; i++)

    {

	r = (rand() % 49 ) + 1;

	temp = nums[i]; nums[i] = nums[r]; nums[r] = temp;

    }

    for(i = 1; i < 7; i++)

    {

	sprintf(buf, "%d", nums[i]);

	strcat(buf, " "); strcat(str, buf);

    }
	
	MessageBox(NULL, str, "Lotto Number Picker", MB_OK | MB_ICONEXCLAMATION);

  return 0; 
} 



