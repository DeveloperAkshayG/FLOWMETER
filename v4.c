//flow meter using timer1 V3 without prescalar and 5ms delay and lcd function removed and only flow value displayed at last.
#define F_CPU 8000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include<string.h>
#include "lcd.c"

float ticks=0.00;
volatile unsigned long overflow;    //original
unsigned int tcntres=0;
double volume=0.00;       //for volume calulation
double flow=0.00;		   //for flow calculation flow=vol*60/sec
char flowres[20]={0};
int flag0=0;
int flag1=0;


ISR(INT0_vect)
{
	flag0=1;
	TCCR1B |= (1<<CS10);   //no prescalar and normal mode
	EIFR=  0b00000010;    //INT1 flag cleared before enabling it
	EIMSK= 0b00000110;      //setting interrupt1 on and interrupt0 off uncomment after
	EICRA=0X0C;              //setting interrupt1 for rising edge uncomment after

}


ISR(INT1_vect)
{
	tcntres=65535-TCNT1; 
	TCCR1B=0X00;
	flag1=1;
	EIMSK &= ~(1<<INT1);
	EIMSK = 0b00000100;
//	flag1=1;
}

ISR(INT2_vect)     //RESET ISR
{
	LCD_Clear();  
	TCCR1B=0X00;
	overflow=0;
	tcntres=0;
	flow=0.00;
	ticks=0.00;
	flag0=0;
	flag1=0;
	memset(flowres,0,20);
	LCD_String_xy(0,0,"NETEL FLOWMETER");
	LCD_String_xy(1,0,"    V4.0    ");
	EIFR=  0b00000001; //delete it after for int1 to work properly
	EIMSK = 0b00000101; //INT0 AND INT2 ACTIVATED
	EICRA=0b00000011;   //DELETE AFTER
	sei();
}



ISR (TIMER1_OVF_vect)  //TIMER OVERFLOW ISR
{
	overflow++;
	TCNT1=25536; 
}

void timer1_init()
{
	TCNT1=25536;     //from calculation 
	overflow=0;
	TIMSK |= (1<<TOIE1);    //INTERRUPT FOR TIMER1 OVERFLOW
	sei();
}


int main ( )
{
	DDRE=0XFF; //MAKE PORTE OUTPUT PORT
	DDRD=0X00; //MAKE PORT D AS INPUT
	PORTE=0b01000000;   //LED GLOW FOR INDICATION
	PORTD=0XFC;  //except pd1 and pd0 all pins are high
	
    LCD_Init();  //intialize lcd
	
	float ID=4;             //diameter=8mm so radius of capillary in mm is d/2 which is 4mm
	int height=50;	   //height of capillary in mm
	double pie=3.14159265358; //pie value used for volume calculation
	
	LCD_String_xy(0,0,"NETEL FLOWMETER");
	LCD_String_xy(1,0,"    V4.0    ");

	
	volume=((pie*(ID)*(ID)*height)/1000)*60;   //volume in mm3 which is converted into ml by dividing it with 1000 on 21 jan
	
	EIMSK = 0b00000101; //set int0 and int2 interrupt 
	EIMSK &= ~(1<<INT1);
	EICRA=0b00000011;  //INT0 CONFIGURED FOR rising EDGE AND INT2 FOR LOW LEVEL 
 
	sei();
	timer1_init();
	while(1)
	{
		if(flag0==1)
		{
			LCD_Clear();
			LCD_String_xy(0,0,"FLOW COUNT START");
			flag0=0;
		}
		else if(flag1==1)
		{
			ticks=(((float)overflow) * 0.005) + (((float)tcntres) * 0.000000125);  //for no prescalar using timer1

			flow=((volume)/ticks);           
			sprintf(flowres,"FLOW:%.2lfml/min",flow);
			LCD_String_xy(0,0,"FLOW COUNT STOP ");
			LCD_String_xy(1,0,flowres);
			memset(flowres,0,20);
			ticks=0.00;
			tcntres=0;
			overflow=0;
			flow=0.00;
			flag1=0;
		}
	}
}
	