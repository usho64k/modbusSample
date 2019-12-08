#include <stdio.h>
#include <stdlib.h>

int num[30];
int sameCount = 0;
FILE *fp;

int xor(int *p,int len)
{
	int i;
	int ret = 0;
	for(i = 0; i < len; i++)
	{
		ret ^= *p;
		p++;
	}
	return ret;
}

int sum(int *p,int len)
{
	int i;
	int ret,_ret = 0;
	for(i = 0; i < len; i++)
	{
		_ret += *p;
		p++;
	}
	ret = ~_ret;
	ret++;
	return ret;
}

void writeFile(int len,int sr)
{
	int i;

	fprintf(fp,"%d : ",sameCount);
	for(i = 0; i < len; i++)
	{
		fprintf(fp,"%02x,",num[i]);
	}
	fprintf(fp,"--->%02x \r\n",sr);
}

void displayData(int len, int sr)
{
	int i;

	printf("%d : ",sameCount);
	for(i = 0; i < len; i++)
	{
		printf("%02x,",num[i]);
	}
	printf("--->%02x \r\n",sr);
}

void substackCall(int n,int l)
{
	int i;
	if(n < l)
	{
		for(num[n] = 1; num[n] <= num[n + 1]; num[n]++)
		{
			if(n == 0)
			{
				int	xorRet = xor(num,l) & 0xFF;
				int sumRet = sum(num,l) & 0xFF;
				if(sumRet == xorRet)
				{
					writeFile(l,sumRet);
					displayData(l,sumRet);
					sameCount++;
				}
			}
			else if(n > 0)
			{
				substackCall(n-1,l);
			}
			else
			{
				printf("ERROR");
			}
		}
	}
	else
	{
		num[n] = 255;
		if(n > 0)
		{
			substackCall(n-1,l);
		}
		else
		{
			printf("error");
		}
	}
}

void main(void)
{
	int i=0,k = 0;
	char str[4];
	char filename[16] = "./matrixXX.csv";


	printf("値の個数を入力してください。\r\n");
	while(i < 4)
	{
		str[i] = fgetc(stdin);
		if(str[i] < 0x30 || str[i] > 0x39)
		{
			break;
		}
		i++;
	}
	
	k = atoi(str);
	
	if(k > 30)
	{
		printf("多すぎ\r\n");
		return;
	}
	filename[8] = k / 10 + 0x30;
	filename[9] = k % 10 + 0x30;

	fp = fopen(filename,"w");
	if(fp == NULL)
	{
		printf("Failure to open file");
		return;
	}
	/*
	
	for(i = 0; i < k; i++)
	{
		printf("数値を入れてください.\r\n");
		j = 0;
		while(j < 4)
		{
			str[j] = fgetc(stdin);
			if(str[j] < 0x30 || str[j] > 0x39)
			{
				break;
			}
			j++;
		}
		num[i] = atoi(str);
	}
	*/
	substackCall(k,k);
	/*
	num[0] = 0;
	for(num[0] = 0; num[0] < 1; num[0]++)
	{
		for(num[1] = 0; num[1] <= 255; num[1]++)
		{
			for(num[2] = 0; num[2] <= num[1]; num[2]++)
			{
				for(num[3] = 0; num[3] <= num[2]; num[3]++)
				{
					xorRet = xor(num,4) & 0xFF;
					//printf("XOR : %2x ",xorRet & 0xFF);
					sumRet = sum(num,4) & 0xFF;
					//printf("SUM : %2x \r\n",sumRet & 0xFF);
					if(xorRet == sumRet)
					{
						fprintf(fp,"%d : %02x,%02x,%02x,%02x--->%02x \r\n",sameCount,num[3],num[2],num[1],num[0],sumRet);
						printf("%d : %02x,%02x,%02x,%02x--->%02x \r\n",sameCount,num[3],num[2],num[1],num[0],sumRet);
						sameCount++;
					}
				}
			}
		}
	}
	*/
	fclose(fp);
}


