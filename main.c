

#include<pic.h>              	//������Ƭ���ڲ���ԴԤ����
#include<pic16fxxx_adc.h>

#define uchar unsigned char
#define uint  unsigned int

#define _XTAL_FREQ 4000000

//__CONFIG(0x03EC);
__CONFIG(FOSC_INTOSCIO&WDTE_ON&PWRTE_OFF&MCLRE_OFF&CP_OFF&CPD_OFF&BOREN_ON&IESO_OFF&FCMEN_OFF);
//оƬ�����֣����Ź������ϵ���ʱ����������أ���ѹ��̹أ�

#define AD1				RA0   	//AD1�ɼ��ӿ�
#define AD2				RA1   	//AD2�ɼ��ӿ�
#define AD3				RA2   	//AD3�ɼ��ӿ�
#define IGN_BUTTON		RA4   	//��𰴼�
#define POSITION_BUTTON	RA5   	//λ�ð���
#define LED1			RC0   	//LED1�ӿ�
#define LED2			RC1   	//LED2�ӿ�
#define LED3			RC4   	//LED3�ӿ�
#define SHOCK			RC2   	//�𵴿��ؿ��ƿ�
#define IGN				RC3   	//��𿪹ؿ��ƿ�
#define PUMP			RC5   	//�ÿ��ؿ��ƿ�

uchar count=0;						//
uint count1=0;						//��ʱ����
uchar count2=0;						//
uchar count3=0;						//
uchar flag_500mS=0;					//
uchar flag_1S=0;					//

uchar flag_Position=0;				//λ�ð������±�־
uchar Unusual=0;					//�쳣��־
uchar Flash_S=0;					//״̬���������־

uint adc_buffer[8];					//ADC�ɼ�������
uint last_adc=0;					//�ϴ�ADC�ɼ�����
uint adc=0;							//ADC�ɼ�����

void GPIO_Configuration(void);		//IO��ʼ��
void WatchDog_Configuration(void);	//���Ź���ʼ��
void TIM0_Configuration(void);		//TIM0��ʼ��
void Ignition(void);				//�������
uint Filter_ADC(uint *p,uchar n);	//��ֵƽ���˲�

//*********************************************************************
// ����ԭ�ͣ�void GPIO_Configuration(void)
// ��    �ܣ��˿�����
// ��ڲ�������
// �� �� ֵ����
//*********************************************************************
void GPIO_Configuration(void)
{	
	OSCCON = 0X61;		//�ڲ�ʱ��4M
	CMCON0 = 0X07;		//�رձȽ�����CIN����Ϊ����IO
	TRISA = 0xfC;  		//portA2-5 ���� 0-1���
	PORTA = 0xfC;		//LEDĬ�ϵ���
	ANSEL = 0x00;		//�ر����пڵ�ADC����
	TRISC = 0;  		//portC ���
	PORTC = 0x00;		//LEDĬ�ϵ����������õ�
}

//*********************************************************************
// ����ԭ�ͣ�void WatchDog_Configuration(void)
// ��    �ܣ��˿�����
// ��ڲ�������
// �� �� ֵ����
//*********************************************************************
void WatchDog_Configuration(void)
{	
	WDTCON = 0x16;		//
}

//*********************************************************************
// ����ԭ�ͣ�void TIM0_Configuration(void)
// ��    �ܣ���ʱ��0����
// ��ڲ�������
// �� �� ֵ����
//*********************************************************************
void TIM0_Configuration(void)
{
	OPTION_REG=0x85;	//��ֹ������FOSC/4��64��Ƶ 64΢��
	INTCON=0x20;		//����TIM0����ж�
	TMR0=99;			//10mS
}

void interrupt timer0()
{
	T0IF=0;
	TMR0=99;			//10mS
	count1++;
	count2++;
	count3++;

	if(count1>740)		//7.4���رձ�
	{
		count1=0;
		PUMP=0;
	}
	if(count2>50)
	{
		count2=0;
		flag_500mS=1;	//0.5��ʱ�䵽
		
	}
	if(count3>100)
	{
		count3=0;
		flag_1S=1;		//1��ʱ�䵽
		if(Flash_S)
		{
			RA1 =~RA1;	//LED PA2�����ź�ȡ��
		}else
			{
				if(Unusual)
				{
					PORTA &=~(1<<1);		//�������
					PORTA |=(1<<0);			//�ر��̵�
				}else
					{
						PORTA |=(1<<1);		//�رպ��
						PORTA &=~(1<<0);	//�����̵�
					}
			}
	}
}

//*********************************************************************
// ����ԭ�ͣ�void Ignition(void)
// ��    �ܣ��������
// ��ڲ�������
// �� �� ֵ����
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
��ֵƽ���˲�
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
* ��    �ƣ�main() 
* ��    �ܣ�������
* ��ڲ�����
* ���ڲ�����
* ˵    ����
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

	PORTA |=(1<<1);			//�ر�״̬���
	PORTA &=~(1<<0);		//����״̬�̵�
	
	GIE=1;					//��ȫ���ж�

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
					PORTC &=~((1<<0)|(1<<1)|(1<<4));	//����Ϊ����͵�ƽ
					Flash_S=0;
				}else
					if(adc>680)							//7.6V //3.3V 680
					{
						PORTC &=~((1<<0)|(1<<4));		//����Ϊ����͵�ƽ
						PORTC |=(1<<1);					//����Ϊ����ߵ�ƽ
						Flash_S=0;
					}else
						if(adc>652)						//7.3V //3.3V 652
						{
							PORTC &=~(1<<4);			//����Ϊ����͵�ƽ
							PORTC |=(1<<0)|(1<<1);		//����Ϊ����ߵ�ƽ
							Flash_S=0;
						}else
							if(adc>625)					//7.05V //3.3V 625
							{
								PORTC |=(1<<0)|(1<<1)|(1<<4);	//�ر����Ե���ָʾ��
								Flash_S=0;
							}else
							{
								PORTA |=(1<<0);					//�ر�״̬�̵�
								PORTC |=(1<<0)|(1<<1)|(1<<4);	//�رյ���ָʾ��
								Flash_S=1;						//�����������
							}
				}
		}
		
		if(!Flash_S)
		{
			if((POSITION_BUTTON == 0)||(IGN_BUTTON == 0))	//�жϰ����Ƿ��а���
			{
				if(last_adc==0)
				{
					for(i=0;i<8;i++)
					{
						adc_buffer[i++]=GET_AD();
						__delay_ms(1);
					}
					last_adc=Filter_ADC(adc_buffer,8);
					count2=0;						//��ʱ����
				}
				if(flag_500mS)
				{
					for(i=0;i<8;i++)
					{
						adc_buffer[i++]=GET_AD();
						__delay_ms(1);
					}
					adc=Filter_ADC(adc_buffer,8);

					if((last_adc-adc)>50)			//ѹ��ﵽ0.5V��Ϊ�����쳣
					{
						Unusual=1;					//�쳣��־
					}else Unusual=0;				//�쳣�ָ�
				}

				PUMP=1;								//�򿪵��
				count1=0;							//��ʼ�����������Ա���ʱ�رյ��
				__delay_ms(20);
				if((POSITION_BUTTON == 0)&&(IGN_BUTTON != 0))			//λ�ð���PA5����
				{
					if(flag_Position==0)flag_Position=1;
				}else if((POSITION_BUTTON == 0)&&(IGN_BUTTON == 0))
					{
						if(flag_Position)
						{
							if(count==0)Ignition();		//���
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
				Unusual=0;				//�쳣�ָ�
			}
		}
	}
}

