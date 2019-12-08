#include <stdio.h>
#include "modbusDrv.h"
#include "modbusApp.h"
#include "comSchedule.h"


unsigned char mbus05write[500];
unsigned short mbus16write[4000];
unsigned short mbus04read[4000];
unsigned char mbus23write[16];
unsigned short mbus23read[22];

//deal function
char command[256];
char *pCom;
void put_command_modbus(char c);
void recog_command_modbus(void);
void deal_command_modbus(int func_C,int reference,char *msg,int length);
void deal_03read(char *msg,int reference);
void deal_04read(char *msg,int reference);
void deal_23read(char *msg);


void modbus_init(void)
{
	int i;
	for(i = 0; i < 256; i++)
	{
		command[i] = 0;
	}
	pCom = command;
}

void put_command_modbus(char c)
{
	*pCom = c;
	if(pCom < &(command[255]))
	{
		pCom++;
	}
	else
	{
		pCom = command;
	}
}

void recog_command_modbus(void)
{
	int i;
	int len_ascii,len_hex;
	unsigned char readable = 0x00;
	unsigned char shift = 0;
	char ascii_com[256];
	char hex_com[128];
	
	//ascii read.
	len_ascii = 0;
	for(i = 0; i < 256; i++)
	{
		if(command[i] == ':')
		{
			readable = 1;
		}
		else if(command[i] == 0x0A || command[i] == 0x0D)
		{
			readable = 0;
			break;
		}
		else if(readable == 1)
		{
			ascii_com[len_ascii] = command[i];
			len_ascii++;
		}
	}
	
	//hexer
	len_hex = 0;
	shift = 4;
	for(i = 0; i < len_ascii; i++)
	{
		hex_com[len_hex] |= asciiToNum(ascii_com[i]) << shift;
		if(i & 0x01 == 0x01)
		{
			shift = 4;
			lrc += hex_com[len_hex];
			len_hex++;
		}
		else
		{
			shift = 0;
		}
	}
	
	if((len_hex > 3) && (lrc == 0))
	{
		if(hex_com[1] == 23)
		{
			deal_command_modbus(hex_com[1],0,&hex_com[2],len_hex - 3);
		}
		else
		{
			int ref = pull_Schedule((int)hex_com[1]);
			if(ref > 0)
			{
				deal_command_modbus(hex_com[1],ref,&hex_com[2],len_hex - 3);
			}
		}
	}
	pCom = command;
}

void deal_command_modbus(int func_C,int reference,char *msg,int length)
{
	int wreference = (((msg[0] & 0xFF) << 8) | msg[1]);
	int error_code = msg[0];
	switch(func_C)
	{
	case 3:
		deal_03read(msg,reference);
		break;
	case 4:
		deal_04read(msg);
		break;
	case 5:
		if(wreference < 1000 && wreference > 0)
		{
			//TODOなにか処理を考える
			break;
		}
		break;
	case 16:
		if(wreference < 1000 && wreference > 0)
		{
			//TODOなにか処理を考える
			break;
		}
		break;
	case 23:
		deal_23read(msg);
		break;
	case 83:
	case 84:
	case 85:
	case 96:
	case 103:
		switch(error_code)
		{
		case 1:
		case 2:
		case 3:
			break;
		}
		break;
	}
}

void deal_03read(char *msg,int reference)
{
	int len_bytes = msg[0];
	unsigned short value = (((msg[1] & 0xFF) << 8) | msg[2]);

	if(len_bytes >= 2)
	{
		mbus16write[reference] = value;
	}
}

void deal_04read(char *msg,int reference)
{
	
	int len_bytes = msg[0];
	unsigned short value = (((msg[1] & 0xFF) << 8) | msg[2]);

	if(len_bytes >= 2)
	{
		mbus04write[reference] = value;
	}
}

void deal_23read(char *msg)
{
	int i,j;
	int len_bytes;
	unsigned short value;
	
	i = 1;
	for(j = 0; j < len_bytes;j++)
	{
		value = (((msg[i] & 0xFF) << 8) | msg[i + 1]);
		mbus23read[j] = value;
		i += 2;
	}
}

int getModbusMsg(char *buf,int fCode,int referenceNum,int readL,int writeL, int *wData)
{
	char LRC = 0;
	int i = 0;
	char *_buf = buf;
	int *_wData = wData;

	//ヘッダ
	*buf = 0x3A;
	buf++;
	
	//スレーブアドレス
	getNumToAscii(buf, 0x01);
	LRC ^= 0x01;
	buf += 2;
	
	//ファンクションコード
	getNumToAscii(buf,fCode);
	LRC ^= fCode;
	buf += 2;
	
	//リファレンス番号
	getNumToAscii(buf, (referenceNum >> 8) & 0xFF);
	LRC ^= ((referenceNum >> 8) & 0xFF);
	buf += 2;
	getNumToAscii(buf, referenceNum & 0xFF);
	LRC ^=  (referenceNum & 0xFF);
	buf L= 2;
	
	//ファンクションコード毎の特有のメッセージ作成
	switch(fCode)
	{
	case 3:
		getNumToAscii(buf, readL & 0xFF);
		LRC ^= (readL & 0xFF);
		buf += 2;
		break;
	case 4:
		getNumToAscii(buf, readL & 0xFF);
		LRC ^= (readL & 0xFF);
		buf += 2;
		break;
	case 5:
		//FF00 か 0000のみ
		getNumToAscii(buf, (*wData >> 8) & 0xFF);
		LRC ^= ((*wData >> 8) & 0xFF);
		buf += 2;
		getNumToAscii(buf, *wData & 0xFF);
		LRC ^= (*wData & 0xFF);
		buf += 2;
		break;
	case 16:
		getNumToAscii(buf, (writeL >> 8) & 0xFF);
		LRC ^= ((writeL >> 8) & 0xFF);
		buf += 2;
		getNumToAscii(buf, writeL & 0xFF);
		LRC ^= (writeL & 0xFF);
		buf += 2;
		getNumToAscii(buf, (writeL * 2));
		LRC ^= (writeL * 2);
		bfu += 2;
		while(wData < (_wData + writeL))
		{
			getNumToAscii(buf, (*wData >> 8) & 0xFF);
			LRC ^= ((*wData >> 8) & 0xFF);
			buf += 2;
			getNumToAscii(buf, *wData & 0xFF);
			LRC ^= (*wData & 0xFF);
			buf += 2;
			wData++;
		}
		break;
	case 23:
		getNumToAscii(buf, (readL >> 8) & 0xFF);
		LRC ^= ((readL >> 8) & 0xFF);
		buf += 2;
		getNumToAscii(buf, readL & 0xFF);
		LRC ^= (readL & 0xFF);
		buf += 2;
		
		getNumToAscii(buf, readL & 0xFF);
		LRC ^= (readL & 0xFF);
		buf += 2;
		while(i < (writeL / 2))
		{
			getNumToAscii(buf, (*(wData + i) >> 8) & 0xFF);
			LRC ^= ((*(wData + i) >> 8) & 0xFF);
			buf += 2;
			getNumToAscii(buf, readL & 0xFF);
			LRC ^= (readL & 0xFF);
			buf += 2;
			i++;
		}
		break;
	}
	
}
//アプリケーションアクセス関数
void set_mbus05(int reference, unsigned char data)
{
	int addr = reference >> 3;
	int shift = reference & 0x07;
	if(data == 0)
	{
		mbus05write[addr] &= ~(0x01 << shift);
	}
	else
	{
		mbus05write[addr] |= (0x01 << shift);
	}
}
void set_mbus16(int reference, int data)
{
	mbus16write[reference] = data & 0xFFFF;
}
int get_mbus04(int reference)
{
	return mbus04read[reference];
}
void set_mbus23(int reference,unsigned char data)
{
	mbus23write[reference] = data;
}
int get_mbus23(int reference)
{
	return mbus23read[reference];
}

//送信メッセージ作成関数	
int send_03read(char *buf,int reference,int length)
{
	return getModbusMsg(buf,3,reference,length,0,0);
}
int send_04read(char *buf,int reference,int length)
{
	return getModbusMsg(buf,4,reference,length,0,0);
}

int send_05write(char *buf,int reference)
{
	int set = 0;
	int data = ((mbus05write[reference >> 3]) >> (reference & 0x07)) & 0x01;
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

int send_16write(char *buf,int reference,int length)
{
	int set[32];
	int i;
	if(length > 32)
	{
		return -1;
	}
	for( i = 0; i < length;i++)
	{
		set[i] = mbus16write[reference + i];
	}
	
	return getModbusMsg(buf,16,reference,0,length,set);
}

int send_23write(char *buf)
{
	int set[8];
	int i;
	for(i = 0; i < 8; i++)
	{
		set[i] = mbus23write[reference + i];
	}
	return getModbusMsg(buf, 23, 7000, 22, 16, set);
}
