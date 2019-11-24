#include "config.h"
#include "delay.h"
#include "uart.h"
#include "led.h"
#include "GSM.h"
#include "Ultrasonic.h"
#include "ds18b20.h"
#include "BH1750.h"
#include "XY-V17B.h"

extern unsigned char flag_temper;
extern unsigned char  aaa[];
extern unsigned char  times_r[25];
unsigned char RX_Buffer[68] = "00000000000000000000000000000000000000000000";
unsigned char RX_Count = 0;

/*************	本地常量声明	**************/

sbit K1  = P2^0;
sbit K2  = P2^1;



uchar LED_MODE_C = 0;
uchar LED_MODE_G = 0;

/*******************************************************************************
* 函数名 : main 
* 描述   : 主函数
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 串口波特率是9600，GPRS模块默认波特率是115200，需要自己通过串口助手修改
				   为9600方可使用。 
*******************************************************************************/
void main(void)
{
//	unsigned int  	i;
	unsigned char 	tempdat[8];
	unsigned int 	tempda1 = 0;
	unsigned int 	tempdisda1=0;
	unsigned int 	tempMultiple=0; 
	unsigned int 	tick=0;
	unsigned int 	led_tick=0;
	unsigned char 	year	=0,
					month	=0,
					day		=0,
					hour	=0,
					minute	=0;
//	unsigned char LED_ON=0;
	unsigned char LED_ON_OFF=0;
	AUXR = AUXR|0x40;  // T1, 1T Mode
	Uart1Init();    	//初始化两个串口9600
//	GSM_INIT();    		//GSM模块初始化
	UART1_SendString("1\r\n");
	while(1)
	{
		tick++;
		led_tick++;
		if(tick % 999 == 0 )
		{
			tempdisda1 = Conut();
			if(tempdisda1<15)
			{
				LED_MODE_C = 1;   //常量
			}
			else if(tempdisda1>=15&& tempdisda1<30)
			{
				LED_MODE_C= 2;	//闪烁
			}
			else
				LED_MODE_C = 0;
		}
		if(tick % 3999 == 0 )	//温度检测
		{
			tempda1 = Get_temp();
		}
		if(tick % 6999 == 0 )		//光强检测
		{
			Multiple_read_BH1750();
			tempMultiple = GetMultipleDat();
			if( tempMultiple <20)
			{
				LED_MODE_G = 1;
			}
			else
			{
				LED_MODE_G = 0;
			}
		}
		if(tick % 4699 == 0 )
		{
			SendCMD_TIME();
		}
		if(tick % 5699==0)
		{
			if(GET_time()==1 )
			{
				//UART1_SendString("ok\r\n");
			}
			else
				;
				//UART1_SendString("error\r\n");
		}
		if( tick <= 3000)			//显示距离
		{
			tempdat[0] = tempdisda1%10;
			tempdat[1] = tempdisda1%100/10;
			tempdat[2] = tempdisda1%1000/100;
			tempdat[3] = 0;
			tempdat[4] = 0;
			tempdat[5] = 0;
			tempdat[6] = 0;
			tempdat[7] = 16;
			display( tempdat );	
		}
		else if(tick > 3000 && tick <= 6000)	//显示温度
		{
			tempdat[0] = tempda1%10/1;
			tempdat[1] = tempda1%100/10;
			tempdat[2] = tempda1%1000/100;
			tempdat[3] = 0;
			tempdat[4] = 0;
			tempdat[5] = 0;
			tempdat[6] = 16;
			tempdat[7] = 0;
			display( tempdat );
		}
		else if(tick > 6000 && tick <= 9000)	//显示光强
		{
			tempdat[0] = tempMultiple%10/1;
			tempdat[1] = tempMultiple%100/10;
			tempdat[2] = tempMultiple%1000/100;
			tempdat[3] = tempMultiple%10000/1000;
			tempdat[4] = 0;
			tempdat[5] = 16;
			tempdat[6] = 0;
			tempdat[7] = 0;
			display( tempdat );
		}
		else if(tick > 9000 && tick <= 12000)//显示日期
		{
			tempdat[0] = times_r[8]-0x30;
			tempdat[1] = times_r[7]-0x30;
			tempdat[2] = 16;
			tempdat[3] = times_r[5]-0x30;
			tempdat[4] = times_r[4]-0x30;
			tempdat[5] = 16;
			tempdat[6] = times_r[2]-0x30;
			tempdat[7] = times_r[1]-0x30;
			display( tempdat );
		}
		else if(tick > 12000 && tick <= 15000)//显示时间
		{
			hour = times_r[14]+times_r[13]*10;
			minute = times_r[11]+times_r[10]*10;
			tempdat[0] = times_r[14]-0x30;
			tempdat[1] = times_r[13]-0x30;
			tempdat[2] = 16;
			tempdat[3] = times_r[11]-0x30;
			tempdat[4] = times_r[10]-0x30;
			tempdat[5] = 16;
			tempdat[6] = 16;
			tempdat[7] = 16;
			display( tempdat );
		}
		if(tick>15000)
		{
			tick = 0;
		}
		
		if(LED_MODE_G == 1 || LED_MODE_C==1 )   ///常亮
		{
			RUNING_LED=0;
			if(LED_MODE_G !=1)
				BEEP = 1;
		}
		else if(LED_MODE_C == 2 && LED_MODE_G == 0)
		{
			if(led_tick>900)
			{
				RUNING_LED=0;
				BEEP = 1;
			}
			else
			{
				RUNING_LED=1;
				BEEP = 0;
			}
		}
		else		//关闭
		{
			RUNING_LED = 1;
			BEEP       = 0;
		}
		if(led_tick>2000)
		{
			led_tick=0;
		}
		if(K1 == 0)	//发送短信
		{
			delay5ms();
			if(K1 == 0)
			{
				while(!K1); 
				Send_SeekhelpMSG();
			}
		}
		if(K2 == 0)		//语音播报
		{
			delay5ms();
			if(K2 == 0)
			{
				while( !K2 ); 
//				SOURCEvoice(tempda1);
//				SOURCEvoiceTime(hour,minute);
				
				
				SOURCEvoice(15);
			//	SOURCEvoiceTime(16,25);
				
				//Second_AT_Command( "Second_AT_Command","OK", 3);
			}
		}
		delay5us();
	}
}


/*******************************************************************************
* 函数名 : Uart2 
* 描述   : 串口2中断服务入口函数
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void RECEIVE_DATA(void) interrupt 4 	  		
{ 
	unsigned char temp = 0;
	ES=0;
	if (RI)
    {
		temp = SBUF;
		RI = 0;
	}
	ES=1; 
}

