#ifndef __POLAR_H__
#define __POLAR_H__


extern UINT16 xdata PolarFlag;
extern UINT16 xdata PloarNormal;
extern UINT8 xdata PloarAngleStage;

void GotoPolarAngle(float angle);		//极化转到angle


#endif