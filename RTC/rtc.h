#ifndef __rtc_h__
#define __rtc_h__
#include "sys.h"


typedef struct  //宏定义结构体 存储 年月日星期 时分秒
{
	vu16 w_year;    //volatile unsigned char year
	vu8 w_month;    //年是4位数要4字节 要u16 千万别u8写顺手了
	vu8 w_date;
	vu8 week;

	vu8 hour;
	vu8 min;
	vu8 sec;
}_calendar_obj;  //结构体命名
extern _calendar_obj calendar;  //声明定义全局变量 存放日历 但c函数中要重新定义

extern u8 const month_table[12]; //月份数组 const变量永远不变

u8 RTC_Init(void); //RTC初始化函数 成功返回0 失败其他
u8 Is_Leap_Year(u16 year); //判断闰年平年函数 返回1是闰年 0不是
u8 RTC_Alarm_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec);
   //报警函数 输入年月日时分秒
u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec); 

u8 RTC_Get(void); //更新时间函数

u8 RTC_Get_Week(u16 year, u8 month, u8 day);//获得星期几函数

   //设置当前时间函数

#endif