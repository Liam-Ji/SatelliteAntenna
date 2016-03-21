#ifndef __ANTENNA_H__
#define __ANTENNA_H__


#include "public.h"


#define MaxFilterLen    10					//最大滤波器长度 //2008-08-19


extern void AntennaInit(void);
extern void AntennaReadySearch(void);
extern void AntennaTimeSearch(void);
extern void AntennaTracking(void);
extern void AntennaManual(void);
extern void AntennaStore(void);


#endif