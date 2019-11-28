#include "rtc.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "beep.h"

_calendar_obj calendar; //�������ڽṹ��

int t=100;
static void RTC_NVIC_Config(void)     //static��̬�������ں�����ǰ ��߳���ģ�黯���� 
{ //����RTC�����ж� �������ȼ���      //�ú���ֻ�ܱ����ļ��е���
  NVIC_InitTypeDef NVIC_InitStruct; //�ṹ��
	NVIC_InitStruct.NVIC_IRQChannel =  RTC_IRQn;//��stm32f10x.h���ж�����
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; //�����ȼ�
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; //�����ȼ�
	NVIC_Init(&NVIC_InitStruct);
}


//RTC��ʼ������
u8 RTC_Init(void)
{
	u8 temp=0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE); //ʹ�ܵ�ԴPWR�ͺ�ʱ�ӽӿ�BKP �������ܽ����Ժ�����
	PWR_BackupAccessCmd(ENABLE);//ʹ�ܺ�ʱ�ӷ���
	
//IF�ж� ����Ƿ��һ������ʱ��
	if(BKP_ReadBackupRegister(BKP_DR1) != 0x0001) //�Ĵ����ź�ֵ�Լ����� �������жϼĴ����Ƿ�д���(����β�����üĴ���ֵ �Ӷ��ж��Ƿ�����)
	{
		BKP_DeInit();	//��λ�������� 
    RCC_LSEConfig(RCC_LSE_ON);//���ⲿ����ʱ��(�򿪺��������)
		while( (RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET) && (temp<250) )  //����ⲿ����ʱ��LSE�Ƿ�ʹ��
		{
			temp++;  //20msx250=2m ���ʱ��
			delay_ms(20);
		}
		if(temp>250) return 1; //��ʼ��ʧ�� 2m����ʧ��RESET��temp�Ѽ�251
 
	  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //����RTCʱ��ΪLSE	
		RCC_RTCCLKCmd(ENABLE); //ʹ��RTCʱ��
		
//����дRTC�Ĵ���
   RTC_WaitForLastTask(); //�ȴ����һ��RTC���ý���
   RTC_WaitForSynchro();  //�ȴ�RTC�Ĵ���ͬ��
	 RTC_ITConfig(RTC_IT_SEC, ENABLE);//����RTC���ж�
	 RTC_ITConfig(RTC_IT_ALR, ENABLE);		//ʹ��RTC�����ж�		
	 RTC_WaitForLastTask();
		
//����Ԥ��Ƶϵ����װ��ֵ
		RTC_EnterConfigMode();//��������ģʽ
    RTC_SetPrescaler(32767);//����Ԥ��Ƶ 3.2768KHZ/(32768-1)=1HZ 1S����һ��
		RTC_WaitForLastTask();  //ÿ����һ�ζ�Ҫ�ȴ����һ��������
	  RTC_Set(2019,11,28,17,15,00);//����ʱ��
		RTC_Alarm_Set(2022,7,1,13,00,00); //��������
		RTC_ExitConfigMode();  //�˳�����ģʽ
		
		BKP_WriteBackupRegister(BKP_DR1, 0x0001);//��ָ���ĺ󱸼Ĵ�����д��ֵ
			   //���óɹ��� ���if�ж� ���Ĵ���дֵ �����Ͳ���ڶ��������� 
	
	}
	else //RTC�Ĵ������ù�
	{
	 RTC_WaitForSynchro();	//�ȴ�RTC�Ĵ���ͬ��  
	 RTC_ITConfig(RTC_IT_SEC, ENABLE);//����RTC���ж�
	 RTC_WaitForLastTask();//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	}
	
	RTC_NVIC_Config(); //�жϷ�������
	RTC_Get();  //ÿ�θ�λ��Ͳ����������� ֻ��Ҫ����Getʱ�����ʱ��	
	
	return 0; //���óɹ�����0
}


//RTC���жϣ���Ϊ���������ж�
//дRTC���жϺ���  ����start.up���жϺ�����
void RTC_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET) //�������ж�
	{
		RTC_Get(); //����ʱ��
	}
	
	if(RTC_GetITStatus(RTC_IT_ALR) != RESET) //���������ж�
	{
		RTC_ClearITPendingBit(RTC_IT_ALR); //����ж�
		RTC_Get(); //����ʱ��
		printf("Alarm Time:%d-%d-%d %d:%d:%d\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//�������ʱ��	
	  printf("��ϲ�о�����ҵ,�㻹��ԭ��������");
		LED1=0;

	}
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW); //��������ж�
	RTC_WaitForLastTask(); //�ȴ�RTC�Ĵ����������
}





//****************************************************************************************************//
//**************�����㷨���� ��cnt��������ֵ��������ʱ����   ��1970�꿪ʼ*****************************//
//****************************************************************************************************//


//�ж�һ���Ƿ�Ϊ���� �ܱ�4��100�����ұ�400��Ϊ����
//�·�        1  2  3  4  5  6  7  8  9  10 11 12
//��������   31 29 31 30 31 30 31 31 30 31 30 31
//���������� 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//����ֵ������.1,��.0,����
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



//����ʱ�Ӻ���
//�����ʱ��������ʱ���� ����Ϊ ���� �ŵ�cnt�Ĵ�����
//19700101Ϊ��׼
//����0�ɹ�
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�
//const �����޷��ı�   volatile�Ż����ٶ������ֵʱ����ÿ�ζ����¶�ȡ������ʹ�ñ����ڼĴ�����ı���
u8 const mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;//����ѭ���Լ�
	u32 seccount=0;
//���������	
	if(syear<1970||syear>2099)return 1;//�����Ϸ�ʱ��
	for(t=1970; t<syear; t++)
	{
		if(Is_Leap_Year(t))seccount+=31622400; //����IS_Leap��������1
		else seccount+=31536000; //��ƽ������
	}
//����������µ�����	
	smon-=1;//����12��xx�� ��ǰ����·���11���� ���Լ�1
	for(t=0; t<smon; t++)//�����Ǵ�0��ʼ������<
	{
		seccount+=mon_table[t]*86400; //ÿ��������*ÿ������
		if( Is_Leap_Year(syear)&&t==1 )seccount+=86400;
		       //t=1��Ϊ�����0��ʼ��t=1�Ƕ��� ���������·�>=2�Ͷ��һ��
	}
//���º������
	seccount+=(u32)(sday-1)*86400; //��̫��ת��Ϊu32��
//������Сʱ��
	seccount+=(u32)(hour)*3600;
//��Сʱ��ķ�����
	seccount+=(u32)(min)*60;          //ע��һ������ ��������Ҫ-1 ��Ϊ���컹û��ȥ
//������                            //             ʱ���벻��   ��Ϊ��ȥ��
	seccount+=sec;   
	
//����ʹ��RTC�󱸼Ĵ��� �������ŵ�cnt��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE); //ʹ�ܵ�ԴPWR�ͺ�ʱ�ӽӿ�BKP �������ܽ����Ժ�����
	PWR_BackupAccessCmd(ENABLE);//ʹ�ܺ�ʱ�ӷ���
	
	RTC_SetCounter(seccount);
	//RTC->CNTH = seccount>>16;
	//RTC->CNTL = seccount&0xFFFF;
	RTC_WaitForLastTask(); //�ȴ�RTC�Ĵ����������
	
	return 0; //�ɹ�
}





//��ʼ������  1970�꿪ʼ  
//�����õ������ջ�Ϊ��д��RTC_CLR�Ĵ���  ��RTC_CNT�Ĵ�����CLR�Ĵ���ֵһ���Ϳ�ʼ��
u8 RTC_Alarm_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;//����ѭ���Լ�
	u32 seccount=0; //��������  
	if(syear<1970||syear>2099)return 1;
	for(t=1970; t<syear; t++)
	{
		if(Is_Leap_Year(t))seccount+=31622400;
		else seccount+=31536000;
	}
//���·ݵ���
	smon-=1;
	for(t=0; t<smon; t++)
	{
		seccount+=(u32)mon_table[t]*86400;
		if(Is_Leap_Year(syear) && t==1) seccount+=86400;
	}
//����
	seccount+=(u32)(sday-1)*86400;
//��Сʱ
	seccount+=(u32)hour*3600;
//���
	seccount+=(u32)min*60;
//����
	seccount+=sec;

//����ʹ��RTC�󱸼Ĵ��� �������ŵ�cnt��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE); //ʹ�ܵ�ԴPWR�ͺ�ʱ�ӽӿ�BKP �������ܽ����Ժ�����
	PWR_BackupAccessCmd(ENABLE);//ʹ�ܺ�ʱ�ӷ���	

//��õ�ֵд��RTC_CLR�Ĵ���
	RTC_SetAlarm(seccount);
	//RTC->ALRH = seccount>>16;
	//RTC->ALRL = seccount&0x0000ffff;
	RTC_WaitForLastTask(); //�ȴ�RTC�Ĵ����������
	return 0;
	
}







//�õ���ǰֵ  �ѵ�ǰcnt�����ֵת��Ϊ������ʱ���� ����ṹ��calendar��
//�ɹ� ����0
//��1970��ʼ,�������ӡ�ÿ����һ��ͼ�ȥcntһ���ֵ ����һ���ټ�ȥ�� �� ����ֱ����Ϊ0
u8 RTC_Get(void) //����ʱ�亯��
{
	static u16 daycnt=0; //��̬������������ �����溯������ı�
	u32 timecount=0;  //���cnt�Ĵ�����ֵ
	u32 temp=0; //�м���� �������
	u32 temp1=0;//�м���� ���������
	
	timecount=RTC_GetCounter(); //temp=RTC->CNTL;	temp=((u32)RTC->CNTH<<16) | temp;
	                    //�õ�CNT�Ĵ�����ֵ
	temp = timecount/86400; //�õ�������Ӧ����
	if(daycnt != temp) //����һ��
	{
		daycnt =temp;
		temp1 = 1970; //��1970��ʼ,�������ӡ�ÿ����һ��ͼ�ȥcnt��ֵ ֱ����Ϊ0
		while(temp>=365)//��ʣ���һ���һֱ��
		{
			if(Is_Leap_Year(temp1))//������
			{
				if(temp>=366)temp-=366; //temp��ȥ���������
				else {temp1++; break;} //�����굫��������������366��ͼ�1������ѭ��
			}
			else temp-=365; //ƽ��-365��
			temp1++;  //ƽ������ temp1�����ݶ���1
		}
		calendar.w_year=temp1;//�õ���
		
//���·�
		temp1=0;
		while(temp>=28)//����һ���� temp�Ѿ���ȥ�����������
		{
			if(Is_Leap_Year(calendar.w_year)&&temp==1)//���һ���������Ҵ���һ����
			{
				if(temp>=29)temp-=29; //temp����29��
				else break; //����������
			}
			else //ƽ��
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1]; //ʣ������������ֵ �ͼ�ȥ��������
				else break; //����������
			}
			temp1++;
		}
		calendar.w_month=temp1+1 ;//�õ��·�
		calendar.w_date = temp+1; //�õ�����
	}
	
	temp = timecount%86400; //�õ�����������  ʣ�µ���
	calendar.hour = temp/3600;
	calendar.min = (temp%3600)/60;
	calendar.sec = (temp%3600)%60;
	calendar.week = RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//��ȡ����   
	
	return 0;
}




//��ȡ���ں���
u8 RTC_Get_Week(u16 year, u8 month, u8 day)  //ͨ��������������
{	
	u16 temp2;
	u8 yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// ���Ϊ21����,�������100  
	if (yearH>19)yearL+=100;
	// ����������ֻ��1900��֮���  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}			  