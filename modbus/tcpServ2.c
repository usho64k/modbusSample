#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <event.h>

unsigned char coil_o_memories[8192];
unsigned char coil_i_memories[8192];
unsigned short reg_o_memories[65536];
unsigned short reg_i_memories[65536];

struct event ev = NULL;
struct event_base *pEv = NULL;

int sd;
int acc_sd;

unsigned short calc_crc(unsigned char *buf,int length)
{
	unsigned short crc = 0xFFFF;
	int i,j;
	unsigned char carreyFlag;

	for(i = 0; i < length; i++)
	{
		crc ^= buf[i];
		for(j = 0; j < 8;j++)
		{
			carreyFlag = crc & 0x01;
			crc = crc >> 1;
			if(carreyFlag)
			{
				crc ^= 0xA001;
			}
		}
	}
	return crc;
}

void init_memmap(void)
{
	//64KBmemset.
	memset(reg_o_memories,0,sizeof(reg_o_memories));
	memset(reg_i_memories,0,sizeof(reg_i_memories));
	memset(coil_o_memories,0,sizeof(coil_o_memories));
	memset(coil_i_memories,0,sizeof(coil_i_memories));
}

int get_OutCoil(unsigned char *buf,unsigned int s_addr,unsigned int num)
{
	//bytes
	int b_len;
	int i;
	int shift;
	int wshift;

	unsigned int readAddr;
	unsigned char *old_buf;
	unsigned char temp_set;

	memset(buf,0,num / 8 + 1);

	readAddr = s_addr >> 3;
	old_buf = buf;
	shift = readAddr & 0x07;
	wshift = 0;

	for(i = 0; i < num;i++)
	{
		temp_set = (coil_o_memories[readAddr] >> shift) & 0x01;
		*buf |= temp_set << wshift;
		shift++;
		wshift++;

		if(shift == 8)
		{
			shift = 0;
			readAddr++;
		}
		if(wshift == 8)
		{
			wshift = 0;
			buf++;
		}
	}
	b_len = buf-old_buf;

	if(wshift != 0)
	{
		b_len++;
	}
	
	return b_len;
}

int get_InCoil(unsigned char *buf,unsigned int s_addr,unsigned int num)
{
	//bytes
	int b_len;
	int i;
	int shift;
	int wshift;

	unsigned int readAddr;
	unsigned char *old_buf;
	unsigned char temp_set;

	memset(buf,0,num / 8 + 1);

	readAddr = s_addr >> 3;
	old_buf = buf;
	shift = readAddr & 0x07;
	wshift = 0;

	for(i = 0; i < num;i++)
	{
		temp_set = (coil_i_memories[readAddr] >> shift) & 0x01;
		*buf |= temp_set << wshift;
		shift++;
		wshift++;

		if(shift == 8)
		{
			shift = 0;
			readAddr++;
		}
		if(wshift == 8)
		{
			wshift = 0;
			buf++;
		}
	}
	b_len = buf-old_buf;

	if(wshift != 0)
	{
		b_len++;
	}
	
	return b_len;
}

int get_OutRegistor(unsigned char *buf,unsigned int s_addr,unsigned int num)
{
	int b_len;
	int i;

	unsigned int readAddr;
	unsigned char *old_buf;
	unsigned char temp_set;

	memset(buf,0,num * 2);
	
	readAddr = s_addr;
	old_buf = buf;

	for(i = 0; i < num * 2; i++)
	{
		if( i & 0x01)
		{
			temp_set = reg_o_memories[readAddr] & 0xFF;
			*buf = temp_set;
			buf++;
			readAddr++;
		}
		else
		{
			temp_set = reg_o_memories[readAddr] >> 8;
			*buf = temp_set;
			buf++;
		}
	}
	
	b_len = buf - old_buf;
	
	return b_len;
}


int get_InRegistor(unsigned char *buf,unsigned int s_addr,unsigned int num)
{
	int b_len;
	int i;

	unsigned int readAddr;
	unsigned char *old_buf;
	unsigned char temp_set;

	memset(buf,0,num * 2);
	
	readAddr = s_addr;
	old_buf = buf;

	for(i = 0; i < num * 2; i++)
	{
		if( i & 0x01)
		{
			temp_set = reg_i_memories[readAddr] & 0xFF;
			*buf = temp_set;
			buf++;
			readAddr++;
		}
		else
		{
			temp_set = reg_i_memories[readAddr] >> 8;
			*buf = temp_set;
			buf++;
		}
	}
	
	b_len = buf - old_buf;
	
	return b_len;
}

void set_OutCoil(unsigned char data,unsigned int s_addr)
{
	unsigned int writeAddr = s_addr >> 3;
	int shift = s_addr & 0x07;
	if( data == 0x00)
	{
		coil_o_memories[writeAddr] &= ~(0x01 << shift);
	}
	else
	{
		coil_o_memories[writeAddr] |= (0x01 << shift);
	}
}

void set_OutRegistor(unsigned short data,unsigned int s_addr)
{
	reg_o_memories[s_addr] = data;
}

void set_OutCoils(unsigned char *data,unsigned int s_addr,unsigned int len_addr)
{
	unsigned int i;
	unsigned char temp_data;
	int shift = 0;

	for(i = 0; i < len_addr; i++)
	{
		temp_data = (*data >> shift) & 0x01;
		shift++;
		if(shift == 8)
		{
			data++;
			shift = 0;
		}
		set_OutCoil(temp_data,s_addr);
		s_addr++;
	}
}

void set_OutRegistors(unsigned char *data,unsigned int s_addr,unsigned int len_addr)
{
	unsigned int i;
	unsigned short temp_data;

	for(i = 0; i < len_addr; i++)
	{
		temp_data = (data[0] << 8) + data[1];
		data += 2;
		set_OutRegistor(temp_data,s_addr);
		s_addr++;
	}
}



int init_modbus(void)
{
	int i;
	struct sockaddr_in addr;
	socklen_t sin_size = sizeof(struct sockaddr_in);
	struct sockaddr_in from_addr;

	if((sd = socket(AF_INET,SOCK_STREAM,0)) < 0 )
	{
		perror("socket");
		return -1;
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(507);
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sd,(struct sockaddr *)&addr,sizeof(addr)) < 0)
	{
		perror("bind");
		return -1;
	}

	if(listen(sd,204) < 0)
	{
		perror("listen");
		return -1;
	}

	if((acc_sd = accept(sd,(struct sockaddr *)&from_addr,&sin_size)) < 0)
	{
		perror("accept");
		return -1;
	}
}	

void exit_modbus(void)
{
	close(acc_sd);
	close(sd);
}

int make_bufsend(unsigned char *senf,unsigned char *recv)
{
	int len = 0;
	unsigned short crc = 0;
	int i;
	int w_len = 0;

	unsigned char command = recv[1];
	unsigned int startAddr = 0;
	unsigned int numRegistor = 0;
	
	unsigned char *wc_Datas;
	unsigned short wc_Data;
	unsigned char *wl_Datas;
	unsigned char wl_Data;

	memset(senf,0,sizeof(senf));


	senf[0] = 0x01;
	senf[1] = command;
	len += 2;
	
	switch(command)
	{
		case 0x01:
			startAddr = (recv[2] << 8) + (recv[3]);
			numRegistor = (recv[4] << 8) + (recv[5]);

			w_len = get_OutCoil(senf+3,startAddr,numRegistor);
			len += w_len;
			senf[2] = w_len;
			len++;
			break;
		case 0x02:
			startAddr = (recv[2] << 8) + (recv[3]);
			numRegistor = (recv[4] << 8) + (recv[5]);
			
			w_len = get_InCoil(senf+3,startAddr,numRegistor);
			len += w_len;
			senf[2] = w_len;
			len++;

			break;
		case 0x03:
			startAddr = (recv[2] << 8)+(recv[3]);
			numRegistor = (recv[4]  << 8)+(recv[5]);

			w_len = get_OutRegistor(senf+3,startAddr,numRegistor);

			len += w_len;
			senf[2] = w_len;
			len++;
			break;
		case 0x04:
			startAddr = (recv[2] << 8) + (recv[3]);
			numRegistor = (recv[4] << 8) + (recv[5]);
			
			w_len = get_InRegistor(senf+3,startAddr,numRegistor);
			len += w_len;
			senf[2] = w_len;
			len++;
			break;
		case 0x05:
			wl_Data = (recv[4] == 0xFF ? 0x01 : 0x00);
			set_OutCoil(recv[4],(recv[2] << 8) + recv[3]);
			senf[2] = recv[2];
			senf[3] = recv[3];
			senf[4] = recv[4];
			senf[5] = recv[5];
			len += 4;
			break;
		case 0x06:
			wc_Data = (recv[4] << 8) + recv[5];
			set_OutRegistor(wc_Data,(recv[2] << 8) + recv[3]);
			senf[2] = recv[2];
			senf[3] = recv[3];
			senf[4] = recv[4];
			senf[5] = recv[5];
			len+= 4;
			break;
		case 0x0F:
			wl_Datas = &recv[7];
			set_OutCoils(wl_Datas,(recv[2] << 8) + recv[3],(recv[4] << 8) + recv[5]);
			senf[2] = recv[2];
			senf[3] = recv[3];
			senf[4] = recv[4];
			senf[5] = recv[5];
			len += 4;
			break;
		case 0x10:
			wc_Datas = &recv[7];
			set_OutRegistors(wc_Datas,(recv[2] << 8) + recv[3],(recv[4] << 8) + recv[5]);
			senf[2] = recv[2];
			senf[3] = recv[3];
			senf[4] = recv[4];
			senf[5] = recv[5];
			len += 4;
			break;
		default:
			return -1;
	}
	crc = calc_crc(senf,len);
	senf[len] = crc & 0xFF;
	senf[len+1] = (crc >> 8) & 0xFF;
	len += 2;

	printf("\r\n");
	for(i = 0; i < len;i++)
	{
		printf("%02x ",senf[i]);
	}
	return len;

}
void eventhandler(int fd,short nanikore,void *opinion)
{
	
	int i;
	
	unsigned char recv_buf[2048];
	unsigned char send_buf[64];
	int send_len = 0;
	int r_len = 0;

	if((r_len = recv(acc_sd,recv_buf,sizeof(recv_buf),0)) >= 0)
	{
		//display
		for(i = 0;i < r_len;i++)
		{
			printf("%02x",recv_buf[i]);
		}
		//if there is a data on the receive data.
		if(r_len > 2)
		{
			send_len = make_bufsend(send_buf,recv_buf);
			send(acc_sd,send_buf,sizeof(char) * send_len,0);
		}
		printf("\n");
	}
}
int main(int argc,char** argv)
{
	
	init_memmap();
	init_modbus();

	pEv = event_init();
	event_set(&ev,sd,EV_READ | EV_PERSIST,eventhandler,&ev);
	event_base_set(pEv,&ev);
	event_add(&ev,NULL);
	
	event_base_dispatch(pEv);
	event_base_free(pEv);

	exit_modbus();
	return 0;
}
