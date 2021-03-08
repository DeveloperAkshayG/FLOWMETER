//flow meter using timer1 V2 with prescalar and 10ms delay and lcd function removed.
#define F_CPU 8000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include<string.h>
#include<math.h>
#include "lcd.c"

#define LED PE6

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
	TCCR1B |= (1<<CS10) | (1<<CS12);   //1024 prescalar
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
	LCD_String_xy(1,0,"    V5.0    ");
	EIFR=  0b00000001; //delete it after for int1 to work properly
	EIMSK = 0b00000101; //INT0 AND INT2 ACTIVATED
	EICRA=0b00000011;   //DELETE AFTER
	sei();
}



ISR (TIMER1_OVF_vect)  //TIMER OVERFLOW ISR
{
	overflow++;
	TCNT1=65380; 
}

void timer1_init()
{
	TCNT1=65380;     //from calculation 
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
	
	//float ID=4;             //diameter=8mm so radius of capillary in mm is d/2 which is 4mm
//	float ID=1.75;             //diameter=3.5mm so radius of capillary in mm is d/2 which is 1.75mm
	float ID=1.82;             //diameter=3.64mm so radius of capillary in mm is d/2 which is 1.82mm(adjusted)

//	float ID=3.5;             //diameter=7mm so radius of capillary in mm is d/2 which is 3.5mm(change to 4.0mm)
	//int height=50;	   //height of capillary in mm

	int height=47;	   //height of capillary in mm(change to 50)

	double pie=3.14159265358; //pie value used for volume calculation
	
	LCD_String_xy(0,0,"NETEL FLOWMETER");
	LCD_String_xy(1,0,"    V5.0    ");

	
	volume=((pie*(ID)*(ID)*height)/1000)*60;   //volume in mm3 which is converted into ml by dividing it with 1000 on 21 jan which is 150.09
	
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
//			ticks=(((float)overflow) * 0.009856) + (((float)tcntres) * 0.000128);  //for 1024 prescalar using timer1

			ticks=(((float)overflow) * 0.02) + (((float)tcntres) * 0.000128);  //for 1024 prescalar using timer1
			ticks=roundf(ticks * 100) / 100; //(if value is 25.62523 after this it will be 25.65000)
			flow=((volume)/ticks);           
			flow=roundf(flow * 100) / 100;   //(if value is 25.62523 after this it will be 25.65000)
			sprintf(flowres,"t=%.2f fl=%.1lf",ticks,flow);
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
	