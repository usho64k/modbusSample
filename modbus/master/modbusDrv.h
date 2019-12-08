#ifndef __MODBUSDRV_H__
#define __MODBUSDRV_H__
int send_03read(char *msg,int reference,int length);
int send_04read(char *msg,int reference,int length);
int send_05write(char *msg,int reference);
int send_16write(char *msg,int reference,int length);
int send_23write(char *msg);

#endif
