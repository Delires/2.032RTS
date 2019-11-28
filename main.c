#include "led.h"
#include "delay.h"
#include "lcd.h"
#include "sys.h"
#include "usart.h"
#include "usmart.h"
#include "rtc.h"


int main(void)
{
	u8 t=0;
	u16 a=10086;
//各种初始化
	delay_init(); 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
 	LED_Init();			     //LED端口初始化
	LCD_Init();			 	
	usmart_dev.init(SystemCoreClock/1000000);	//初始化USMART	
	RTC_Init();

//显示信息
	POINT_COLOR=BLACK;
	LCD_ShowString(50,50,200,16,16, "Delires");
	LCD_ShowString(50,70,200,16,16, "CaoPengxiang");
	LCD_ShowString(50,90,200,16,16, "2019/11/28");

//显示时间
	POINT_COLOR=GRAY;
	LCD_ShowString(50,150,200,16,16, "    -  -  "); //显示年月日
	LCD_ShowString(50,180,200,16,16, "  :  :  ");  //显示时分秒
	
	while(1)
	{
		if(t != calendar.sec)
		{
			t= calendar.sec;
 			
			LCD_ShowNum(50,150, calendar.w_year,4,16);
			LCD_ShowNum(90,150, calendar.w_month,2,16);
			LCD_ShowNum(114,150, calendar.w_date,2,16);
			
		switch(calendar.week)
			{
				case 0:
					LCD_ShowString(50,210,200,16,16,"Sunday   ");
					break;
				case 1:
					LCD_ShowString(50,210,200,16,16,"Monday   ");
					break;
				case 2:
					LCD_ShowString(50,210,200,16,16,"Tuesday  ");
					break;
				case 3:
					LCD_ShowString(50,210,200,16,16,"Wednesday");
					break;
				case 4:
					LCD_ShowString(50,210,200,16,16,"Thursday ");
					break;
				case 5:
					LCD_ShowString(50,210,200,16,16,"Friday   ");
					break;
				case 6:
					LCD_ShowString(50,210,200,16,16,"Saturday ");
					break;  
			}
			LCD_ShowNum(50,180,calendar.hour,2,16);
			LCD_ShowNum(74,180,calendar.min,2,16);
			LCD_ShowNum(98,180,calendar.sec,2,16);
	    
      LED0 = !LED0;			
		}
		delay_ms(10);
	}
		
}