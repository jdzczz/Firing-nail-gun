

#include<pic.h>              	//包含单片机内部资源预定义
#include<pic16fxxx_adc.h>

#define uchar unsigned char
#define uint  unsigned int

#define _XTAL_FREQ 4000000

//__CONFIG(0x03EC);
__CONFIG(FOSC_INTOSCIO&WDTE_ON&PWRTE_OFF&MCLRE_OFF&CP_OFF&CPD_OFF&BOREN_ON&IESO_OFF&FCMEN_OFF);
//芯片配置字，看门狗开，上电延时开，掉电检测关，低压编程关，

#define AD1				RA0   	//AD1采集接口
#define AD2				RA1   	//AD2采集接口
#define AD3				RA2   	//AD3采集接口
#define IGN_BUTTON		RA4   	//点火按键
#define POSITION_BUTTON	RA5   	//位置按键
#define LED1			RC0   	//LED1接口
#define LED2			RC1   	//LED2接口
#define LED3			RC4   	//LED3接口
#define SHOCK			RC2   	//震荡开关控制口
#define IGN				RC3   	//打火开关控制口
#define PUMP			RC5   	//泵开关控制口

uchar count=0;						//
uint count1=0;						//延时计数
uchar count2=0;						//
uchar count3=0;						//
uchar flag_500mS=0;					//
uchar flag_1S=0;					//

uchar flag_Position=0;				//位置按键按下标志
uchar Unusual=0;					//异常标志
uchar Flash_S=0;					//状态红灯秒闪标志

uint adc_buffer[8];					//ADC采集缓冲区
uint last_adc=0;					//上次ADC采集数据
uint adc=0;							//ADC采集数据

void GPIO_Configuration(void);		//IO初始化
void WatchDog_Configuration(void);	//看门狗初始化
void TIM0_Configuration(void);		//TIM0初始化
void Ignition(void);				//点火驱动
uint Filter_ADC(uint *p,uchar n);	//中值平均滤波

//*********************************************************************
// 函数原型：void GPIO_Configuration(void)
// 功    能：端口配置
// 入口参数：无
// 返 回 值：无
//*********************************************************************
void GPIO_Configuration(void)
{	
	OSCCON = 0X61;		//内部时钟4M
	CMCON0 = 0X07;		//关闭比较器，CIN配置为数字IO
	TRISA = 0xfC;  		//portA2-5 输入 0-1输出
	PORTA = 0xfC;		//LED默认点亮
	ANSEL = 0x00;		//关闭所有口的ADC功能
	TRISC = 0;  		//portC 输出
	PORTC = 0x00;		//LED默认点亮，其它置低
}

//*********************************************************************
// 函数原型：void WatchDog_Configuration(void)
// 功    能：端口配置
// 入口参数：无
// 返 回 值：无
//*********************************************************************
void WatchDog_Configuration(void)
{	
	WDTCON = 0x16;		//
}

//*********************************************************************
// 函数原型：void TIM0_Configuration(void)
// 功    能：定时器0配置
// 入口参数：无
// 返 回 值：无
//*********************************************************************
void TIM0_Configuration(void)
{
	OPTION_REG=0x85;	//禁止上拉，FOSC/4，64分频 64微秒
	INTCON=0x20;		//允许TIM0溢出中断
	TMR0=99;			//10mS
}

void interrupt timer0()
{
	T0IF=0;
	TMR0=99;			//10mS
	count1++;
	count2++;
	count3++;

	if(count1>740)		//7.4秒后关闭泵
	{
		count1=0;
		PUMP=0;
	}
	if(count2>50)
	{
		count2=0;
		flag_500mS=1;	//0.5秒时间到
		
	}
	if(count3>100)
	{
		count3=0;
		flag_1S=1;		//1秒时间到
		if(Flash_S)
		{
			RA1 =~RA1;	//LED PA2驱动信号取反
		}else
			{
				if(Unusual)
				{
					PORTA &=~(1<<1);		//点亮红灯
					PORTA |=(1<<0);			//关闭绿灯
				}else
					{
						PORTA |=(1<<1);		//关闭红灯
						PORTA &=~(1<<0);	//点亮绿灯
					}
			}
	}
}

//*********************************************************************
// 函数原型：void Ignition(void)
// 功    能：点火驱动
// 入口参数：无
// 返 回 值：无
//*********************************************************************
void Ignition(void)
{
	PORTC |=(1<<2);
	__delay_ms(30);
	PORTC |=(1<<2)|(1<<3);
	__delay_ms(15);
	PORTC &=~(1<<3);
	PORTC |=(1<<2);
	__delay_ms(15);
	PORTC &=~(1<<2);
	__delay_ms(15);

	PORTC |=(1<<2);
	__delay_ms(15);
	PORTC |=(1<<2)|(1<<3);
	__delay_ms(15);
	PORTC &=~(1<<3);
	PORTC |=(1<<2);
	__delay_ms(15);
	PORTC &=~(1<<2);
	__delay_ms(15);

	PORTC |=(1<<2);
	__delay_ms(15);
	PORTC |=(1<<2)|(1<<3);
	__delay_ms(15);
	PORTC &=~(1<<3);
	PORTC |=(1<<2);
	__delay_ms(15);
	PORTC &=~(1<<2);
	__delay_ms(15);
	
	PORTC |=(1<<2);
	__delay_ms(15);
	PORTC |=(1<<2)|(1<<3);
	__delay_ms(15);
	PORTC &=~(1<<3);
	PORTC |=(1<<2);
	__delay_ms(15);
	PORTC &=~(1<<2);
	__delay_ms(15);
}

/*******************************************************************************
中值平均滤波
*******************************************************************************/ 
uint Filter_ADC(uint *p,uchar n)
{
	char count,i,j;
	uint temp;
	unsigned long int sum=0;
	for(j=0;j<n;j++)
	 { 
		for(i=0;i<n-j;i++)
	    {
		 if(p[i]>p[i+1])
	 	 {
			temp=p[i];
			p[i]=p[i+1]; 
			p[i+1]=temp;
	 	 }
		}
		CLRWDT();
	}
	for(count=1;count<(n-1);count++)
	sum+=p[count];
	return (uint)(sum/(n-2));
}

/****************************************************************************
* 名    称：main() 
* 功    能：主函数
* 入口参数：
* 出口参数：
* 说    明：
****************************************************************************/
void main()                 
{
	uchar i;
	count1=0;
	count2=0;
	count3=0;
	Flash_S=0;

	WatchDog_Configuration();
	GPIO_Configuration();
	AD_IO_Configuration();
    TIM0_Configuration();
	AD_Configuration();
	CLRWDT();

	PORTA |=(1<<1);			//关闭状态红灯
	PORTA &=~(1<<0);		//开启状态绿灯
	
	GIE=1;					//开全局中断

	while(1)
	{
		CLRWDT();

		if(PUMP!=1)
		{
			if(flag_500mS)
			{
				flag_500mS=0;
				//adc=GET_AD();
				for(i=0;i<8;i++)
				{
					adc_buffer[i++]=GET_AD();
					__delay_ms(1);
				}
				adc=Filter_ADC(adc_buffer,8);
				CLRWDT();

				if(adc>709)								//8V //3.3V 709
				{
					PORTC &=~((1<<0)|(1<<1)|(1<<4));	//引脚为输出低电平
					Flash_S=0;
				}else
					if(adc>680)							//7.6V //3.3V 680
					{
						PORTC &=~((1<<0)|(1<<4));		//引脚为输出低电平
						PORTC |=(1<<1);					//引脚为输出高电平
						Flash_S=0;
					}else
						if(adc>652)						//7.3V //3.3V 652
						{
							PORTC &=~(1<<4);			//引脚为输出低电平
							PORTC |=(1<<0)|(1<<1);		//引脚为输出高电平
							Flash_S=0;
						}else
							if(adc>625)					//7.05V //3.3V 625
							{
								PORTC |=(1<<0)|(1<<1)|(1<<4);	//关闭所以电量指示灯
								Flash_S=0;
							}else
							{
								PORTA |=(1<<0);					//关闭状态绿灯
								PORTC |=(1<<0)|(1<<1)|(1<<4);	//关闭电量指示灯
								Flash_S=1;						//开启红灯秒闪
							}
				}
		}
		
		if(!Flash_S)
		{
			if((POSITION_BUTTON == 0)||(IGN_BUTTON == 0))	//判断按键是否有按下
			{
				if(last_adc==0)
				{
					for(i=0;i<8;i++)
					{
						adc_buffer[i++]=GET_AD();
						__delay_ms(1);
					}
					last_adc=Filter_ADC(adc_buffer,8);
					count2=0;						//计时清零
				}
				if(flag_500mS)
				{
					for(i=0;i<8;i++)
					{
						adc_buffer[i++]=GET_AD();
						__delay_ms(1);
					}
					adc=Filter_ADC(adc_buffer,8);

					if((last_adc-adc)>50)			//压差达到0.5V认为出现异常
					{
						Unusual=1;					//异常标志
					}else Unusual=0;				//异常恢复
				}

				PUMP=1;								//打开电机
				count1=0;							//初始化计数器，以便延时关闭电机
				__delay_ms(20);
				if((POSITION_BUTTON == 0)&&(IGN_BUTTON != 0))			//位置按键PA5按下
				{
					if(flag_Position==0)flag_Position=1;
				}else if((POSITION_BUTTON == 0)&&(IGN_BUTTON == 0))
					{
						if(flag_Position)
						{
							if(count==0)Ignition();		//点火
							count=2;
							flag_Position=0;
						}
					}
				CLRWDT();
			}
			else
			{
				count=0;
				last_adc=0;
				flag_Position=0;
				Unusual=0;				//异常恢复
			}
		}
	}
}

