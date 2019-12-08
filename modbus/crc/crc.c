#include <stdio.h>
#include <stdlib.h>

void calc_crc(unsigned char *buf,int length);
int accessor(int value1,int value2);

void main(void)
{
	//��`
	int i;
	unsigned char sendData[256];
	int d_length = 0;
	
	printf("*******CRC�`�F�b�N�T���A�v���P�[�V���� v0.0.1*******");
	while(d_length == 0)
	{
		printf("���͂���f�[�^�̒�������͂��Ă�������(length < 256)�F");
		scanf("%d",&d_length);
		if(d_length > 256)
		{
			printf("�����B\r\n");
			d_length = 0;
		}
	}
	
	for(i = 0; i < d_length; i++)
	{
		printf("�f�[�^[%d]�F",i);
		scanf("%d",&sendData[i]);
	}
	
	//��������������Ǝv��
	calc_crc(&sendData[0],d_length);
	printf("�f�[�^���F%d",d_length);
}

void calc_crc(unsigned char *buf,int length)
{
	unsigned short ret = 0xFFFF;
	int i,j;
	unsigned char exFlag;
	
	for(i = 0; i < length; i++)
	{
		//XOR���Z
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
���ǎg��Ȃ�
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