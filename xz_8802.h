#ifndef __XZ8802_H__
#define __XZ8802_H__

#include "c8051f120.h"
#define uchar unsigned char 
#define uint  unsigned int  
//#define unsigned char byte
//#define unsigned int  word


#define RMM_COMM 0x30    //������������
#define WMM_COMM 0x38    //д����������
#define VER_COMM 0x33    //У������
#define RSM_COMM 0x31    //������洢��
#define WSM_COMM 0x39    //д����洢��
#define RPM_COMM 0x34    //�������洢��
#define WPM_COMM 0x3c    //д�����洢��

//******************************************
//             ����XZ8802�ӿ�

//sbit     IC_RST     = P1^1;   /*XZ8802��λ������ */
//sbit     IC_CLK     = P1^0;   /*XZ8802ʱ�������� */
//sbit     IC_IO      = P1^2;   /*XZ8802������˫�����ţ�������open-drain��©��*/

#define  IC_IO_1 	P7 |=  0x04
#define  IC_IO_0 	P7 &= ~0x04
#define  IC_CLK_1 	P7 |=  0x02
#define  IC_CLK_0 	P7 &= ~0x02
#define  IC_RST_1  	P7 |=  0x01
#define  IC_RST_0  	P7 &= ~0x01


extern void          _nop_     (void);
extern bit           _testbit_ (bit);
extern unsigned char _cror_    (unsigned char, unsigned char);
extern unsigned int  _iror_    (unsigned int,  unsigned char);
extern unsigned long _lror_    (unsigned long, unsigned char);
extern unsigned char _crol_    (unsigned char, unsigned char);
extern unsigned int  _irol_    (unsigned int,  unsigned char);
extern unsigned long _lrol_    (unsigned long, unsigned char);
extern unsigned char _chkfloat_(float);
extern void          _push_    (unsigned char _sfr);
extern void          _pop_     (unsigned char _sfr);

//sbit     IC_PGM     = P3^2;   /*XZ8802��������� */
/******************************************/

//************************************************************************
//XZ8802�ײ������ⲿ��������

void ReadMainMem(uchar addr,uchar idata *pt,uchar count); //��XZ8802����
void WriteMainMem(uchar addr,uchar idata *pt);        //дXZ8802����
void ReadProtectMem(uchar idata *pt);                 //�������洢��
void ProtectByte(uchar CardAdd,uchar idata *pt);      //����һ�ֽ�,ע�����
//�����ֽ����Ѿ�д�����,��ַֻ���ڱ����洢����
bit  Verify(uchar idata *pt);                         //У������,�ɹ�����1

void SendComm(uchar a,uchar b,uchar c);                      //��������
void ReadMode(uchar idata *pt,uchar count);
void ProcessMode(void);
void ResetCard(void);
void BreakOperate(void);
void StartComm(void);
void StopComm(void);
uchar ReadByte(void);
void WriteByte(uchar ch);
void Delay10us(void);
void Delay5us(void);
//extern unsigned char Verify8802(void);
//extern void PowerOn(uchar idata *pt);                        //XZ8802�ϵ�,��ʱԼ80us,XZ8802��λ,ͬʱ����4����ʾ�ֽ�
//extern void PowerOff(void);                                  //XZ8802�µ�,��ʱԼ80us
//******************************************************************

#endif