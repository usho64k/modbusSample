#ifndef __MODBUSAPP_H__
#define __MODBUSAPP_H__

void airjokyo(int air,int bulb,int sore,int kore);
void solenoid(unsigned char sole,unsigned char noid);

void set_mbus05(int reference,unsigned char data);
void set_mbus16(int reference,int data);
int get_mbus04(int reference);
void set_mbus23(int reference,unsigned char data);
int get_mbus23(int reference);

#endif
