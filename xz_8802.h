#ifndef __XZ8802_H__
#define __XZ8802_H__

#include "c8051f120.h"
#define uchar unsigned char 
#define uint  unsigned int  
//#define unsigned char byte
//#define unsigned int  word


#define RMM_COMM 0x30    //读主存命令字
#define WMM_COMM 0x38    //写主存命令字
#define VER_COMM 0x33    //校验密码
#define RSM_COMM 0x31    //读密码存储区
#define WSM_COMM 0x39    //写密码存储区
#define RPM_COMM 0x34    //读保护存储区
#define WPM_COMM 0x3c    //写保护存储区

//******************************************
//             定义XZ8802接口

//sbit     IC_RST     = P1^1;   /*XZ8802复位线引脚 */
//sbit     IC_CLK     = P1^0;   /*XZ8802时钟线引脚 */
//sbit     IC_IO      = P1^2;   /*XZ8802数据线双向引脚（最好设成open-drain开漏）*/

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

//sbit     IC_PGM     = P3^2;   /*XZ8802编程线引脚 */
/******************************************/

//************************************************************************
//XZ8802底层程序的外部函数声明

void ReadMainMem(uchar addr,uchar idata *pt,uchar count); //读XZ8802主存
void WriteMainMem(uchar addr,uchar idata *pt);        //写XZ8802主存
void ReadProtectMem(uchar idata *pt);                 //读保护存储器
void ProtectByte(uchar CardAdd,uchar idata *pt);      //保护一字节,注意待保
//护的字节是已经写入过的,地址只能在保护存储区内
bit  Verify(uchar idata *pt);                         //校验密码,成功返回1

void SendComm(uchar a,uchar b,uchar c);                      //发送命令
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
//extern void PowerOn(uchar idata *pt);                        //XZ8802上电,延时约80us,XZ8802复位,同时读入4个标示字节
//extern void PowerOff(void);                                  //XZ8802下电,延时约80us
//******************************************************************

#endif