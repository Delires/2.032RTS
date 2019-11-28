#ifndef __rtc_h__
#define __rtc_h__
#include "sys.h"


typedef struct  //�궨��ṹ�� �洢 ���������� ʱ����
{
	vu16 w_year;    //volatile unsigned char year
	vu8 w_month;    //����4λ��Ҫ4�ֽ� Ҫu16 ǧ���u8д˳����
	vu8 w_date;
	vu8 week;

	vu8 hour;
	vu8 min;
	vu8 sec;
}_calendar_obj;  //�ṹ������
extern _calendar_obj calendar;  //��������ȫ�ֱ��� ������� ��c������Ҫ���¶���

extern u8 const month_table[12]; //�·����� const������Զ����

u8 RTC_Init(void); //RTC��ʼ������ �ɹ�����0 ʧ������
u8 Is_Leap_Year(u16 year); //�ж�����ƽ�꺯�� ����1������ 0����
u8 RTC_Alarm_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec);
   //�������� ����������ʱ����
u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec); 

u8 RTC_Get(void); //����ʱ�亯��

u8 RTC_Get_Week(u16 year, u8 month, u8 day);//������ڼ�����

   //���õ�ǰʱ�亯��

#endif