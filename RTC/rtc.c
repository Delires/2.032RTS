#include "rtc.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "beep.h"

_calendar_obj calendar; //¶¨ÒåÈÕÆÚ½á¹¹Ìå

int t=100;
static void RTC_NVIC_Config(void)     //static¾²Ì¬±äÁ¿·ÅÔÚº¯ÊıÃûÇ° Ìá¸ß³ÌĞòÄ£¿é»¯ÌØĞÔ 
{ //ÅäÖÃRTCµÄ×ÜÖĞ¶Ï ÅäÖÃÓÅÏÈ¼¶µÈ      //¸Ãº¯ÊıÖ»ÄÜ±»±¾ÎÄ¼şÖĞµ÷ÓÃ
  NVIC_InitTypeDef NVIC_InitStruct; //½á¹¹Ìå
	NVIC_InitStruct.NVIC_IRQChannel =  RTC_IRQn;//ÔÚstm32f10x.hÕÒÖĞ¶ÏÀàĞÍ
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; //Ö÷ÓÅÏÈ¼¶
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; //¸±ÓÅÏÈ¼¶
	NVIC_Init(&NVIC_InitStruct);
}


//RTC³õÊ¼»¯º¯Êı
u8 RTC_Init(void)
{
	u8 temp=0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE); //Ê¹ÄÜµçÔ´PWRºÍºó±¸Ê±ÖÓ½Ó¿ÚBKP ÕâÑù²ÅÄÜ½øĞĞÒÔºóÉèÖÃ
	PWR_BackupAccessCmd(ENABLE);//Ê¹ÄÜºó±¸Ê±ÖÓ·ÃÎÊ
	
//IFÅĞ¶Ï ¼ì²éÊÇ·ñµÚÒ»´ÎÅäÖÃÊ±ÖÓ
	if(BKP_ReadBackupRegister(BKP_DR1) != 0x0001) //¼Ä´æÆ÷ºÅºÍÖµ×Ô¼ºÉèÖÃ ¾ÍÓÃÓÚÅĞ¶Ï¼Ä´æÆ÷ÊÇ·ñĞ´¹ıË(³ÌĞòÎ²»áÉèÖÃ¼Ä´æÆ÷Öµ ´Ó¶øÅĞ¶ÏÊÇ·ñÅäÖÃ)
	{
		BKP_DeInit();	//¸´Î»±¸·İÇøÓò 
    RCC_LSEConfig(RCC_LSE_ON);//´ò¿ªÍâ²¿µÍËÙÊ±ÖÓ(´ò¿ªºó²ÅÄÜÅäÖÃ)
		while( (RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET) && (temp<250) )  //¼ì²éÍâ²¿µÍËÙÊ±ÖÓLSEÊÇ·ñÊ¹ÄÜ
		{
			temp++;  //20msx250=2m ¼ì²éÊ±¼ä
			delay_ms(20);
		}
		if(temp>250) return 1; //³õÊ¼»¯Ê§°Ü 2mºó»¹ÊÇÊ§°ÜRESETµ«tempÒÑ¼Ó251
 
	  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //ÅäÖÃRTCÊ±ÖÓÎªLSE	
		RCC_RTCCLKCmd(ENABLE); //Ê¹ÄÜRTCÊ±ÖÓ
		
//ÅäÖÃĞ´RTC¼Ä´æÆ÷
   RTC_WaitForLastTask(); //µÈ´ı×î½üÒ»´ÎRTCÅäÖÃ½áÊø
   RTC_WaitForSynchro();  //µÈ´ıRTC¼Ä´æÆ÷Í¬²½
	 RTC_ITConfig(RTC_IT_SEC, ENABLE);//¿ªÆôRTCÃëÖĞ¶Ï
	 RTC_ITConfig(RTC_IT_ALR, ENABLE);		//Ê¹ÄÜRTCÄÖÖÓÖĞ¶Ï		
	 RTC_WaitForLastTask();
		
//ÅäÖÃÔ¤·ÖÆµÏµÊıºÍ×°ÔØÖµ
		RTC_EnterConfigMode();//½øÈëÅäÖÃÄ£Ê½
    RTC_SetPrescaler(32767);//ÅäÖÃÔ¤·ÖÆµ 3.2768KHZ/(32768-1)=1HZ 1S¼ÆÊıÒ»ÏÂ
		RTC_WaitForLastTask();  //Ã¿ÅäÖÃÒ»´Î¶¼ÒªµÈ´ı×î½üÒ»´ÎÅäÖÃÍê
	  RTC_Set(2019,11,28,17,15,00);//ÉèÖÃÊ±¼ä
		RTC_Alarm_Set(2022,7,1,13,00,00); //ÉèÖÃÄÖÖÓ
		RTC_ExitConfigMode();  //ÍË³öÅäÖÃÄ£Ê½
		
		BKP_WriteBackupRegister(BKP_DR1, 0x0001);//ÏòÖ¸¶¨µÄºó±¸¼Ä´æÆ÷ÖĞĞ´ÈëÖµ
			   //ÅäÖÃ³É¹¦ºó ½áºÏifÅĞ¶Ï ¸ø¼Ä´æÆ÷Ğ´Öµ ÕâÑù¾Í²»»áµÚ¶ş´ÎÅäÖÃÁË 
	
	}
	else //RTC¼Ä´æÆ÷ÅäÖÃ¹ı
	{
	 RTC_WaitForSynchro();	//µÈ´ıRTC¼Ä´æÆ÷Í¬²½  
	 RTC_ITConfig(RTC_IT_SEC, ENABLE);//¿ªÆôRTCÃëÖĞ¶Ï
	 RTC_WaitForLastTask();//µÈ´ı×î½üÒ»´Î¶ÔRTC¼Ä´æÆ÷µÄĞ´²Ù×÷Íê³É
	}
	
	RTC_NVIC_Config(); //ÖĞ¶Ï·Ö×éÉèÖÃ
	RTC_Get();  //Ã¿´Î¸´Î»ºó¾Í²»ÓÃÖØĞÂÅäÖÃ Ö»ĞèÒªÖØĞÂGetÊ±¼ä¸üĞÂÊ±¼ä	
	
	return 0; //ÅäÖÃ³É¹¦·µ»Ø0
}


//RTCÃëÖĞ¶Ï£¬ÒòÎªÉèÖÃÁËÃëÖĞ¶Ï
//Ğ´RTCÃëÖĞ¶Ïº¯Êı  ÏÖÔÚstart.upÕÒÖĞ¶Ïº¯ÊıÃû
void RTC_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET) //¶Áµ½ÃëÖĞ¶Ï
	{
		RTC_Get(); //¸üĞÂÊ±¼ä
	}
	
	if(RTC_GetITStatus(RTC_IT_ALR) != RESET) //¶Áµ½ÄÖÖÓÖĞ¶Ï
	{
		RTC_ClearITPendingBit(RTC_IT_ALR); //Çå³şÖĞ¶Ï
		RTC_Get(); //¸üĞÂÊ±¼ä
		printf("Alarm Time:%d-%d-%d %d:%d:%d\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//Êä³öÄÖÁåÊ±¼ä	
	  printf("¹§Ï²ÑĞ¾¿Éú±ÏÒµ,Äã»¹ÊÇÔ­À´µÄÄãÂğ");
		LED1=0;

	}
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW); //Çå³ş¸÷ÖÖÖĞ¶Ï
	RTC_WaitForLastTask(); //µÈ´ıRTC¼Ä´æÆ÷²Ù×÷Íê³É
}





//****************************************************************************************************//
//**************ÒÔÏÂËã·¨²¿·Ö ÓÃcnt¼ÆÊıÆ÷µÄÖµËãÄêÔÂÈÕÊ±·ÖÃë   ´Ó1970Äê¿ªÊ¼*****************************//
//****************************************************************************************************//


//ÅĞ¶ÏÒ»ÄêÊÇ·ñÎªÈòÄê ÄÜ±»4ºÍ100Õû³ıÇÒ±»400³ıÎªÈòÄê
//ÔÂ·İ        1  2  3  4  5  6  7  8  9  10 11 12
//ÈòÄêÌìÊı   31 29 31 30 31 30 31 31 30 31 30 31
//·ÇÈòÄêÌìÊı 31 28 31 30 31 30 31 31 30 31 30 31
//ÊäÈë:Äê·İ
//·µ»ØÖµ£ºÈòÄê.1,ÊÇ.0,²»ÊÇ
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



//ÉèÖÃÊ±ÖÓº¯Êı
//ÊäÈëµÄÊ±ÖÓÄêÔÂÈÕÊ±·ÖÃë »»ËãÎª ÃëÖÓ ·Åµ½cnt¼Ä´æÆ÷ÖĞ
//19700101Îª»ù×¼
//·µ»Ø0³É¹¦
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //ÔÂĞŞÕıÊı¾İ±í
//const ±äÁ¿ÎŞ·¨¸Ä±ä   volatileÓÅ»¯Æ÷ÔÙ¶Áµ½Õâ¸öÖµÊ±±ØĞëÃ¿´Î¶¼ÖØĞÂ¶ÁÈ¡¶ø²»ÊÇÊ¹ÓÃ±£´æÔÚ¼Ä´æÆ÷ÀïµÄ±¸·İ
u8 const mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;//ÓÃ×÷Ñ­»·×Ô¼Ó
	u32 seccount=0;
//¼ÓÄêµÄÃëÊı	
	if(syear<1970||syear>2099)return 1;//³¬¹ıºÏ·¨Ê±¼ä
	for(t=1970; t<syear; t++)
	{
		if(Is_Leap_Year(t))seccount+=31622400; //ÈòÄêIS_Leapº¯Êı·µ»Ø1
		else seccount+=31536000; //¼ÓÆ½ÄêÃëÊı
	}
//¼ÓÄêÊıºóµÄÔÂµÄÃëÊı	
	smon-=1;//±ÈÈç12ÔÂxxºÅ ÔòÇ°ÃæµÄÔÂ·İÊÇ11¸öÔÂ ËùÒÔ¼õ1
	for(t=0; t<smon; t++)//Êı×éÊÇ´Ó0¿ªÊ¼£¬ËùÒÔ<
	{
		seccount+=mon_table[t]*86400; //Ã¿¸öÔÂÌìÊı*Ã¿ÌìÃëÊı
		if( Is_Leap_Year(syear)&&t==1 )seccount+=86400;
		       //t=1ÒòÎªÊı×é´Ó0¿ªÊ¼£¬t=1ÊÇ¶şÔÂ ÔòÈòÄêÇÒÔÂ·İ>=2¾Í¶à¼ÓÒ»Ìì
	}
//¼ÓÔÂºóµÄÌìÊı
	seccount+=(u32)(sday-1)*86400; //ÅÂÌ«´ò×ª»»Îªu32ĞÍ
//¼ÓÌìºóµÄĞ¡Ê±Êı
	seccount+=(u32)(hour)*3600;
//¼ÓĞ¡Ê±ºóµÄ·ÖÖÓÊı
	seccount+=(u32)(min)*60;          //×¢ÒâÒ»¸öÎÊÌâ ÄêÔÂÈÕËãÒª-1 ÒòÎªÕâÌì»¹Ã»¹ıÈ¥
//¼ÓÃëÊı                            //             Ê±·ÖÃë²»ÓÃ   ÒòÎª¹ıÈ¥ÁË
	seccount+=sec;   
	
//ÖØĞÂÊ¹ÄÜRTCºó±¸¼Ä´æÆ÷ °ÑÃëÊı·Åµ½cntÖĞ
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE); //Ê¹ÄÜµçÔ´PWRºÍºó±¸Ê±ÖÓ½Ó¿ÚBKP ÕâÑù²ÅÄÜ½øĞĞÒÔºóÉèÖÃ
	PWR_BackupAccessCmd(ENABLE);//Ê¹ÄÜºó±¸Ê±ÖÓ·ÃÎÊ
	
	RTC_SetCounter(seccount);
	//RTC->CNTH = seccount>>16;
	//RTC->CNTL = seccount&0xFFFF;
	RTC_WaitForLastTask(); //µÈ´ıRTC¼Ä´æÆ÷²Ù×÷Íê³É
	
	return 0; //³É¹¦
}





//³õÊ¼»¯ÄÖÖÓ  1970Äê¿ªÊ¼  
//°ÑÉèÖÃµÄÄêÔÂÈÕ»¯ÎªÃëĞ´ÈëRTC_CLR¼Ä´æÆ÷  µ±RTC_CNT¼Ä´æÆ÷ºÍCLR¼Ä´æÆ÷ÖµÒ»Ñù¾Í¿ªÊ¼ÄÖ
u8 RTC_Alarm_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;//ÓÃ×÷Ñ­»·×Ô¼Ó
	u32 seccount=0; //¼Ç×÷ÃëÊı  
	if(syear<1970||syear>2099)return 1;
	for(t=1970; t<syear; t++)
	{
		if(Is_Leap_Year(t))seccount+=31622400;
		else seccount+=31536000;
	}
//ËãÔÂ·İµÄÃë
	smon-=1;
	for(t=0; t<smon; t++)
	{
		seccount+=(u32)mon_table[t]*86400;
		if(Is_Leap_Year(syear) && t==1) seccount+=86400;
	}
//ËãÈÕ
	seccount+=(u32)(sday-1)*86400;
//ËãĞ¡Ê±
	seccount+=(u32)hour*3600;
//Ëã·Ö
	seccount+=(u32)min*60;
//ËãÃë
	seccount+=sec;

//ÖØĞÂÊ¹ÄÜRTCºó±¸¼Ä´æÆ÷ °ÑÃëÊı·Åµ½cntÖĞ
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE); //Ê¹ÄÜµçÔ´PWRºÍºó±¸Ê±ÖÓ½Ó¿ÚBKP ÕâÑù²ÅÄÜ½øĞĞÒÔºóÉèÖÃ
	PWR_BackupAccessCmd(ENABLE);//Ê¹ÄÜºó±¸Ê±ÖÓ·ÃÎÊ	

//ËãºÃµÄÖµĞ´ÈëRTC_CLR¼Ä´æÆ÷
	RTC_SetAlarm(seccount);
	//RTC->ALRH = seccount>>16;
	//RTC->ALRL = seccount&0x0000ffff;
	RTC_WaitForLastTask(); //µÈ´ıRTC¼Ä´æÆ÷²Ù×÷Íê³É
	return 0;
	
}







//µÃµ½µ±Ç°Öµ  °Ñµ±Ç°cntÀïµÄÃëÖµ×ª»»ÎªÄêÔÂÈÕÊ±·ÖÃë ´æÈë½á¹¹ÌåcalendarÖĞ
//³É¹¦ ·µ»Ø0
//´Ó1970¿ªÊ¼,ÖğÄêÔö¼Ó¡£Ã¿Ôö¼ÓÒ»Äê¾Í¼õÈ¥cntÒ»ÄêµÄÖµ ²»¹»Ò»ÄêÔÙ¼õÈ¥ÔÂ ÈÕ ·ÖÃëÖ±µ½¼õÎª0
u8 RTC_Get(void) //¸üĞÂÊ±¼äº¯Êı
{
	static u16 daycnt=0; //¾²Ì¬±äÁ¿±£´æÈÕÆÚ ²»»áËæº¯ÊıÔËËã¸Ä±ä
	u32 timecount=0;  //´æ·Åcnt¼Ä´æÆ÷µÄÖµ
	u32 temp=0; //ÖĞ¼ä±äÁ¿ ´æ·ÅÌìÊı
	u32 temp1=0;//ÖĞ¼ä±äÁ¿ ´æ·ÅÄêÔÂÈÕ
	
	timecount=RTC_GetCounter(); //temp=RTC->CNTL;	temp=((u32)RTC->CNTH<<16) | temp;
	                    //µÃµ½CNT¼Ä´æÆ÷µÄÖµ
	temp = timecount/86400; //µÃµ½ÃëÊı¶ÔÓ¦ÌìÊı
	if(daycnt != temp) //³¬¹ıÒ»Ìì
	{
		daycnt =temp;
		temp1 = 1970; //´Ó1970¿ªÊ¼,ÖğÄêÔö¼Ó¡£Ã¿Ôö¼ÓÒ»Äê¾Í¼õÈ¥cntµÄÖµ Ö±µ½¼õÎª0
		while(temp>=365)//²»Ê£×îºóÒ»Äê¾ÍÒ»Ö±¼õ
		{
			if(Is_Leap_Year(temp1))//ÊÇÈòÄê
			{
				if(temp>=366)temp-=366; //temp¼õÈ¥ÈòÄêµÄÌìÊı
				else {temp1++; break;} //ÊÇÈòÄêµ«ÓàÊıµÄÌìÊı²»¹»366Ìì¾Í¼Ó1ÄêÌø³öÑ­»·
			}
			else temp-=365; //Æ½Äê-365Ìì
			temp1++;  //Æ½ÄêÈòÄê temp1´æ·ÅÄê·İ¶¼¼Ó1
		}
		calendar.w_year=temp1;//µÃµ½Äê
		
//ËãÔÂ·İ
		temp1=0;
		while(temp>=28)//³¬¹ıÒ»¸öÔÂ tempÒÑ¾­¼õÈ¥ÁËËùÓĞÄêµÄÃë
		{
			if(Is_Leap_Year(calendar.w_year)&&temp==1)//×îºóÒ»ÄêÊÇÈòÄêÇÒ´óÓÚÒ»¸öÔÂ
			{
				if(temp>=29)temp-=29; //temp´óÓÚ29Ìì
				else break; //²»´óÓÚÌø³ö
			}
			else //Æ½Äê
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1]; //Ê£ÏÂÌìÊı´óÓÚÔÂÖµ ¾Í¼õÈ¥ÕâÔÂÌìÊı
				else break; //²»´óÓÚÌø³ö
			}
			temp1++;
		}
		calendar.w_month=temp1+1 ;//µÃµ½ÔÂ·İ
		calendar.w_date = temp+1; //µÃµ½ÌìÊı
	}
	
	temp = timecount%86400; //µÃµ½³ıÄêÔÂÈÕÍâ  Ê£ÏÂµÄÃë
	calendar.hour = temp/3600;
	calendar.min = (temp%3600)/60;
	calendar.sec = (temp%3600)%60;
	calendar.week = RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//»ñÈ¡ĞÇÆÚ   
	
	return 0;
}




//»ñÈ¡ĞÇÆÚº¯Êı
u8 RTC_Get_Week(u16 year, u8 month, u8 day)  //Í¨¹ıÄêÔÂÈÕËãĞÇÆÚ
{	
	u16 temp2;
	u8 yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// Èç¹ûÎª21ÊÀ¼Í,Äê·İÊı¼Ó100  
	if (yearH>19)yearL+=100;
	// Ëù¹ıÈòÄêÊıÖ»Ëã1900ÄêÖ®ºóµÄ  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}			  