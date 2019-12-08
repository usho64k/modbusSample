#include "stdio.h"
#include "stdlib.h"

char message[10000];

int getnum(void)
{
	char str[5];
	int ret;
	int i;
	
	for(i = 0; i < 5; i++)
	{
		str[i] = fgetc(stdin);
		if(str[i] < 0x30 || str[i] > 0x39)
		{
			if(str[i] < 0x41 || str[i] > 0x46)
			{
				break;
			}
		}
	}
	printf("%s\r\n",str);
	ret = (int)(strtol(str,NULL,16));
	while((str[0] = getchar()) != '\n');
	return ret;
}

void main(void)
{
	char lrc = 0;
	int i;
	int ketasu;
	
	printf("Ba-ka");
	ketasu = getnum();
	
	
	for(i = 0; i < ketasu; i++)
	{
		printf("Input number %d :",i);
		message[i] = (char)(getnum());
	}
	
	
	for(i = 0; i < ketasu; i++)
	{
		lrc += message[i];
	}
	
	printf("SUM:%02x\r\n",lrc);
	
	lrc = (256 - lrc) & 0xFF;
	printf("LRC:%02x\r\n",lrc);
}
