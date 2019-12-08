#include <stdio.h>
#include <stdlib.h>

void calc_crc(unsigned char *buf,int length);
int accessor(int value1,int value2);

void main(void)
{
	//定義
	int i;
	unsigned char sendData[256];
	int d_length = 0;
	
	printf("*******CRCチェックサムアプリケーション v0.0.1*******");
	while(d_length == 0)
	{
		printf("入力するデータの長さを入力してください(length < 256)：");
		scanf("%d",&d_length);
		if(d_length > 256)
		{
			printf("無理。\r\n");
			d_length = 0;
		}
	}
	
	for(i = 0; i < d_length; i++)
	{
		printf("データ[%d]：",i);
		scanf("%d",&sendData[i]);
	}
	
	//こちらも正しいと思う
	calc_crc(&sendData[0],d_length);
	printf("データ長：%d",d_length);
}

void calc_crc(unsigned char *buf,int length)
{
	unsigned short ret = 0xFFFF;
	int i,j;
	unsigned char exFlag;
	
	for(i = 0; i < length; i++)
	{
		//XOR演算
		ret ^= buf[i];
		for(j = 0;j < 8;j++)
		{
			exFlag = ret & 0x01;
			ret = ret >> 1;
			
			if(exFlag)
			{
				ret ^= 0xA001;
			}
			
		}
	}
	printf("%x\r\n",ret);
	
	
}

/*
結局使わない
int accessor(int value1,int value2)
{
	int i;
	int temp,temp2;
	int maxShift;
	int mnshift = 5;
	
	while(value1 > value2)
	{
		i = 0;
		while((value1 >> i) > 0)
		{
			maxShift = i;
			i++;
		}
		
		temp = value2 <<(maxShift - mnshift + 1);
		for(i = maxShift; i >= 0; i--)
		{
			temp2 = 0x01 << i;
			if((value1 &temp2) == (value2 & temp2))
			{
				value1 &= ~temp2;
				
			}
			else
			{
				 value1 |= temp2;
			}
			
		}
	}
	printf("%2x",value1);
	return value1;
}
*/