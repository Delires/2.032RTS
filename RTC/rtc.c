#include "rtc.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "beep.h"

_calendar_obj calendar; //定义日期结构体

int t=100;
static void RTC_NVIC_Config(void)     //static静态变量放在函数名前 提高程序模块化特性 
{ //配置RTC的总中断 配置优先级等      //该函数只能被本文件中调用
  NVIC_InitTypeDef NVIC_InitStruct; //结构体
	NVIC_InitStruct.NVIC_IRQChannel =  RTC_IRQn;//在stm32f10x.h找中断类型
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; //主优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; //副优先级
	NVIC_Init(&NVIC_InitStruct);
}


//RTC初始化函数
u8 RTC_Init(void)
{
	u8 temp=0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE); //使能电源PWR和后备时钟接口BKP 这样才能进行以后设置
	PWR_BackupAccessCmd(ENABLE);//使能后备时钟访问
	
//IF判断 检查是否第一次配置时钟
	if(BKP_ReadBackupRegister(BKP_DR1) != 0x0001) //寄存器号和值自己设置 就用于判断寄存器是否写过�(程序尾会设置寄存器值 从而判断是否配置)
	{
		BKP_DeInit();	//复位备份区域 
    RCC_LSEConfig(RCC_LSE_ON);//打开外部低速时钟(打开后才能配置)
		while( (RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET) && (temp<250) )  //检查外部低速时钟LSE是否使能
		{
			temp++;  //20msx250=2m 检查时间
			delay_ms(20);
		}
		if(temp>250) return 1; //初始化失败 2m后还是失败RESET但temp已加251
 
	  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //配置RTC时钟为LSE	
		RCC_RTCCLKCmd(ENABLE); //使能RTC时钟
		
//配置写RTC寄存器
   RTC_WaitForLastTask(); //等待最近一次RTC配置结束
   RTC_WaitForSynchro();  //等待RTC寄存器同步
	 RTC_ITConfig(RTC_IT_SEC, ENABLE);//开启RTC秒中断
	 RTC_ITConfig(RTC_IT_ALR, ENABLE);		//使能RTC闹钟中断		
	 RTC_WaitForLastTask();
		
//配置预分频系数和装载值
		RTC_EnterConfigMode();//进入配置模式
    RTC_SetPrescaler(32767);//配置预分频 3.2768KHZ/(32768-1)=1HZ 1S计数一下
		RTC_WaitForLastTask();  //每配置一次都要等待最近一次配置完
	  RTC_Set(2019,11,28,17,15,00);//设置时间
		RTC_Alarm_Set(2022,7,1,13,00,00); //设置闹钟
		RTC_ExitConfigMode();  //退出配置模式
		
		BKP_WriteBackupRegister(BKP_DR1, 0x0001);//向指定的后备寄存器中写入值
			   //配置成功后 结合if判断 给寄存器写值 这样就不会第二次配置了 
	
	}
	else //RTC寄存器配置过
	{
	 RTC_WaitForSynchro();	//等待RTC寄存器同步  
	 RTC_ITConfig(RTC_IT_SEC, ENABLE);//开启RTC秒中断
	 RTC_WaitForLastTask();//等待最近一次对RTC寄存器的写操作完成
	}
	
	RTC_NVIC_Config(); //中断分组设置
	RTC_Get();  //每次复位后就不用重新配置 只需要重新Get时间更新时间	
	
	return 0; //配置成功返回0
}


//RTC秒中断，因为设置了秒中断
//写RTC秒中断函数  现在start.up找中断函数名
void RTC_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET) //读到秒中断
	{
		RTC_Get(); //更新时间
	}
	
	if(RTC_GetITStatus(RTC_IT_ALR) != RESET) //读到闹钟中断
	{
		RTC_ClearITPendingBit(RTC_IT_ALR); //清楚中断
		RTC_Get(); //更新时间
		printf("Alarm Time:%d-%d-%d %d:%d:%d\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//输出闹铃时间	
	  printf("恭喜研究生毕业,你还是原来的你吗");
		LED1=0;

	}
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW); //清楚各种中断
	RTC_WaitForLastTask(); //等待RTC寄存器操作完成
}





//****************************************************************************************************//
//**************以下算法部分 用cnt计数器的值算年月日时分秒   从1970年开始*****************************//
//****************************************************************************************************//


//判断一年是否为闰年 能被4和100整除且被400除为闰年
//月份        1  2  3  4  5  6  7  8  9  10 11 12
//闰年天数   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年天数 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//返回值：闰年.1,是.0,不是
u8 Is_Leap_Year(u16 year)
{
	if(year%4 ==0)
	{
		if(year%100 == 0)
		{
			if(year%400 == 0)
			{
				return 1;
			}
			else return 0;
		}
		else return 0;
	}
	else return 0;
}



//设置时钟函数
//输入的时钟年月日时分秒 换算为 秒钟 放到cnt寄存器中
//19700101为基准
//返回0成功
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表
//const 变量无法改变   volatile优化器再读到这个值时必须每次都重新读取而不是使用保存在寄存器里的备份
u8 const mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;//用作循环自加
	u32 seccount=0;
//加年的秒数	
	if(syear<1970||syear>2099)return 1;//超过合法时间
	for(t=1970; t<syear; t++)
	{
		if(Is_Leap_Year(t))seccount+=31622400; //闰年IS_Leap函数返回1
		else seccount+=31536000; //加平年秒数
	}
//加年数后的月的秒数	
	smon-=1;//比如12月xx号 则前面的月份是11个月 所以减1
	for(t=0; t<smon; t++)//数组是从0开始，所以<
	{
		seccount+=mon_table[t]*86400; //每个月天数*每天秒数
		if( Is_Leap_Year(syear)&&t==1 )seccount+=86400;
		       //t=1因为数组从0开始，t=1是二月 则闰年且月份>=2就多加一天
	}
//加月后的天数
	seccount+=(u32)(sday-1)*86400; //怕太打转换为u32型
//加天后的小时数
	seccount+=(u32)(hour)*3600;
//加小时后的分钟数
	seccount+=(u32)(min)*60;          //注意一个问题 年月日算要-1 因为这天还没过去
//加秒数                            //             时分秒不用   因为过去了
	seccount+=sec;   
	
//重新使能RTC后备寄存器 把秒数放到cnt中
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE); //使能电源PWR和后备时钟接口BKP 这样才能进行以后设置
	PWR_BackupAccessCmd(ENABLE);//使能后备时钟访问
	
	RTC_SetCounter(seccount);
	//RTC->CNTH = seccount>>16;
	//RTC->CNTL = seccount&0xFFFF;
	RTC_WaitForLastTask(); //等待RTC寄存器操作完成
	
	return 0; //成功
}





//初始化闹钟  1970年开始  
//把设置的年月日化为秒写入RTC_CLR寄存器  当RTC_CNT寄存器和CLR寄存器值一样就开始闹
u8 RTC_Alarm_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;//用作循环自加
	u32 seccount=0; //记作秒数  
	if(syear<1970||syear>2099)return 1;
	for(t=1970; t<syear; t++)
	{
		if(Is_Leap_Year(t))seccount+=31622400;
		else seccount+=31536000;
	}
//算月份的秒
	smon-=1;
	for(t=0; t<smon; t++)
	{
		seccount+=(u32)mon_table[t]*86400;
		if(Is_Leap_Year(syear) && t==1) seccount+=86400;
	}
//算日
	seccount+=(u32)(sday-1)*86400;
//算小时
	seccount+=(u32)hour*3600;
//算分
	seccount+=(u32)min*60;
//算秒
	seccount+=sec;

//重新使能RTC后备寄存器 把秒数放到cnt中
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE); //使能电源PWR和后备时钟接口BKP 这样才能进行以后设置
	PWR_BackupAccessCmd(ENABLE);//使能后备时钟访问	

//算好的值写入RTC_CLR寄存器
	RTC_SetAlarm(seccount);
	//RTC->ALRH = seccount>>16;
	//RTC->ALRL = seccount&0x0000ffff;
	RTC_WaitForLastTask(); //等待RTC寄存器操作完成
	return 0;
	
}







//得到当前值  把当前cnt里的秒值转换为年月日时分秒 存入结构体calendar中
//成功 返回0
//从1970开始,逐年增加。每增加一年就减去cnt一年的值 不够一年再减去月 日 分秒直到减为0
u8 RTC_Get(void) //更新时间函数
{
	static u16 daycnt=0; //静态变量保存日期 不会随函数运算改变
	u32 timecount=0;  //存放cnt寄存器的值
	u32 temp=0; //中间变量 存放天数
	u32 temp1=0;//中间变量 存放年月日
	
	timecount=RTC_GetCounter(); //temp=RTC->CNTL;	temp=((u32)RTC->CNTH<<16) | temp;
	                    //得到CNT寄存器的值
	temp = timecount/86400; //得到秒数对应天数
	if(daycnt != temp) //超过一天
	{
		daycnt =temp;
		temp1 = 1970; //从1970开始,逐年增加。每增加一年就减去cnt的值 直到减为0
		while(temp>=365)//不剩最后一年就一直减
		{
			if(Is_Leap_Year(temp1))//是闰年
			{
				if(temp>=366)temp-=366; //temp减去闰年的天数
				else {temp1++; break;} //是闰年但余数的天数不够366天就加1年跳出循环
			}
			else temp-=365; //平年-365天
			temp1++;  //平年闰年 temp1存放年份都加1
		}
		calendar.w_year=temp1;//得到年
		
//算月份
		temp1=0;
		while(temp>=28)//超过一个月 temp已经减去了所有年的秒
		{
			if(Is_Leap_Year(calendar.w_year)&&temp==1)//最后一年是闰年且大于一个月
			{
				if(temp>=29)temp-=29; //temp大于29天
				else break; //不大于跳出
			}
			else //平年
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1]; //剩下天数大于月值 就减去这月天数
				else break; //不大于跳出
			}
			temp1++;
		}
		calendar.w_month=temp1+1 ;//得到月份
		calendar.w_date = temp+1; //得到天数
	}
	
	temp = timecount%86400; //得到除年月日外  剩下的秒
	calendar.hour = temp/3600;
	calendar.min = (temp%3600)/60;
	calendar.sec = (temp%3600)%60;
	calendar.week = RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//获取星期   
	
	return 0;
}




//获取星期函数
u8 RTC_Get_Week(u16 year, u8 month, u8 day)  //通过年月日算星期
{	
	u16 temp2;
	u8 yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// 如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}			  