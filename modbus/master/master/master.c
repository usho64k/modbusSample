#include <stdio.h>
#include <stdlib.h>

char numToAscii(int data,int bit)
{
	int _data = 0;
	if(bit < 8)
	{
		_data = data >> (bit * 4);
		_data &= 0x0F;
		switch(_data)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				return (_data + 0x30);
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				return (_data + 0x41 - 10);
		}
	}
	return -1;
}

void getNumToAscii(char *input,int data)
{
	//input は2以上の長さがあること
	int i = 2;
	while(i >= 0)
	{
		i--;
		*input =numToAscii(data,i);
		input++;
	}
}

int getModbusMsg(char *buf,int fCode,int referenceNum,int readL,int writeL,int *wData)
{
	char LRC = 0;
	int i = 0;
	char *_buf = buf;
	int *_wData = wData;
	//ヘッダ
	*buf = 0x3A;
	buf++;
	//スレーブアドレス
	getNumToAscii(buf,0x01);
	LRC ^= 0x01;
	buf += 2;
	//ファンクションコード
	getNumToAscii(buf,fCode);
	LRC ^= fCode;
	buf += 2;
	//リファレンス番号
	getNumToAscii(buf, (referenceNum >> 8) & 0xFF);
	buf += 2;
	LRC ^= ((referenceNum >> 8) & 0xFF);
	getNumToAscii(buf,referenceNum & 0xFF);
	LRC ^= (referenceNum & 0xFF);
	buf += 2;

	switch(fCode)
	{
	case 3:
		//リード長
		getNumToAscii(buf, readL & 0xFF);
		buf += 2;
		LRC ^= (readL & 0xFF);
		break;
	case 4:
		//リード長
		getNumToAscii(buf, readL & 0xFF);
		buf += 2;
		LRC ^= (readL & 0xFF);
		break;
	case 5:
		//書き込みデータ
			//FF00 か 0000のみ
		getNumToAscii(buf, (*wData >> 8) & 0xFF);
		LRC ^= (*wData >> 8) & 0xFF;
		buf += 2;
		getNumToAscii(buf, *wData & 0xFF);
		LRC ^= (*wData & 0xFF);
		buf += 2;
		break;
	case 16:
		//ライト長
		getNumToAscii(buf, (writeL >> 8) & 0xFF);
		buf += 2;
		LRC ^= ((writeL >> 8) & 0xFF);
		getNumToAscii(buf, writeL & 0xFF);
		buf += 2;
		LRC ^= (writeL & 0xFF);
		//ライト長(バイト単位)
		getNumToAscii(buf, (writeL + writeL) & 0xFF);
		buf += 2;
		LRC ^= ((writeL + writeL) & 0xFF);
		//書き込みデータ
		while(wData < (_wData + writeL))
		{
			int d = 0x2345;
			getNumToAscii(buf,(*wData >> 8) & 0xFF);
			buf += 2;
			LRC ^= ((*wData >> 8) & 0xFF);
			getNumToAscii(buf, *wData & 0xFF);
			buf += 2;
			LRC ^= (*wData & 0xFF);
			wData++;
		}
		break;
	case 23:
		//リード長
		getNumToAscii(buf, (readL >> 8) & 0xFF);
		buf += 2;
		LRC ^= ((readL >> 8) & 0xFF);
		getNumToAscii(buf, readL & 0xFF);
		buf += 2;
		LRC ^= (readL & 0xFF);
		//ライト長(バイト長)
		getNumToAscii(buf, writeL & 0xFF);
		buf += 2;
		LRC ^= (writeL & 0xFF);
		//書き込みデータ
		while(i < (writeL / 2))
		{
			getNumToAscii(buf, (*(wData+i) >> 8) & 0xFF);
			buf += 2;
			LRC ^= ((*(wData+i) >> 8) & 0xFF);
			getNumToAscii(buf, *(wData+i) & 0xFF);
			buf += 2;
			LRC ^= (*(wData+i) & 0xFF);
			i++;
		}
		break;
	}
	//フッター
	getNumToAscii(buf,LRC);
	buf+=2;
	*buf = 0x0D;
	buf++;
	*buf = 0x0A;
	buf++;
	
	return (buf - _buf);
}

int send_03read(char *buf,int reference,int length)
{
	return getModbusMsg(buf,3,reference,length,0,NULL);
}

int send_04read(char *buf,int reference,int length)
{
	return getModbusMsg(buf,4,reference,length,0,NULL);
}

int send_05write(char *buf,int reference,char data)
{
	int set = 0;
	if(data == 0)
	{
		set = 0x0000;
	}
	else
	{
		set = 0xFF00;
	}
	return getModbusMsg(buf,5,reference,0,1,&set);
}

int send_16write(char *buf, int reference,int *data,int length)
{
	int set[length];
	int i;
	
	for(i = 0; i < length; i++)
	{
		set[i] = *(data + i);
	}
	
	return getModbusMsg(buf, 16, reference, 0, length, set);
}

int send_23write(char *buf, char *data)
{
	int set[8];
	int i;
	
	for(i = 0; i < 8; i++)
	{
		set[i] = (*(data + (i << 1))) << 8;
		set[i] = *(data + (i << 1) + 1);
	}
	
	return getModbusMsg(buf, 23, 7000, 22, 16, set);
}



void main(void)
{
	char mazo[65];
	char input[10];
	int func,ref,len,data;
	int data_c[65];
	int i = 0;
	for(i = 0; i < 65; i++)
	{
		mazo[i] = 0x00;
		data_c[i] = 0x00;
	}
	printf("please put me !");
	fgets(input,sizeof(input),stdin);

	func = (int)(strtol(input,NULL,10));
	
	switch(func)
	{
	case 3:
		printf("put referenceNo:");
		fgets(input,sizeof(input),stdin);
		ref = (int)(strtol(input,NULL,10));
		printf("put length:");
		fgets(input,sizeof(input),stdin);
		len = (int)(strtol(input,NULL,10));
		send_03read(mazo,ref,len);
		break;
	case 4:
		printf("put referenceNo:");
		fgets(input,sizeof(input),stdin);
		ref = (int)(strtol(input,NULL,10));
		printf("put length:");
		fgets(input,sizeof(input),stdin);
		len = (int)(strtol(input,NULL,10));
		send_04read(mazo,ref,len);
		break;
	case 5:
		printf("put referenceNo:");
		fgets(input,sizeof(input),stdin);
		ref = (int)(strtol(input,NULL,10));
		printf("bit on ? off ? (1...on 0...off):");
		fgets(input,sizeof(input),stdin);
		data = (int)(strtol(input,NULL,10));
		send_05write(mazo,ref,data);
		break;
	case 16:
		printf("put referenceNo:");
		fgets(input,sizeof(input),stdin);
		ref = (int)(strtol(input,NULL,10));
		printf("put length:");
		fgets(input,sizeof(input),stdin);
		len = (int)(strtol(input,NULL,10));
		for(i = 0; i < len; i++)
		{
			if(i > 32)
			{
				break;
			}
			printf("put data for hex No.%d:", i);
			fgets(input,sizeof(input),stdin);
			data_c[i] = (int)(strtol(input,NULL,16));
		}
		send_16write(mazo,ref,data_c,len);
		break;
	}

	printf("superdata is %s",mazo);
}
