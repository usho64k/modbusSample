#include <stdio.h>
#include "comSchedule.h"

typedef struct t_motion_sche
{
	char id_func;
	int reference;
}T_SCHEDULE;

T_SCHEDULE commandScheduler[64];
T_SCHEDULE *pSche_W;
T_SCHEDULE *pSche_R;

void push_Schedule(int function,int reference)
{
	T_SCHEDULE t;
	t.id_func = function;
	t.reference = reference;
	*pSche_W = t;
	if(pSche_W < &comSchedule[63])
	{
		pSche_W++;
	}
	else
	{
		pSche_W = comSchedule;
	}
}

int pull_Schedule(int function)
{
	T_SHEDULE t;
	t.id_func = 0;
	t.reference = 0;
	while(pSche_W != pSche_R)
	{
		int f = pSche_R->id_func;
		int ref = pSche_R->reference;
		
		*pSche_R = t;
		if(pSche_R < &comSchedule[63])
		{
			pSche_R++;
		}
		else
		{
			pSche_R = comSchedule;
		}
		
		if(function == f)
		{
			return ref;
		}
		else
		{
			//再送要求
		}
	}
	return -1;
}
