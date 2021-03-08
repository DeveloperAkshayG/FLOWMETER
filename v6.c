//flow meter using timer1 V2 with prescalar and 20ms delay.
#define F_CPU 8000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include<string.h>
#include<math.h>
#include "lcd.c"

#define LED PE6

float ticks=0.00;
volatile unsigned long overflow=0;    //original
unsigned int tcntres=0;
double volume=0.00;       //for volume calulation
double flow=0.00;		   //for flow calculation flow=vol*60/sec
char flowres[20]={0};
char ticksres[20]={0};
int flag0=0;
int flag1=0;


ISR(INT0_vect)
{
	flag0=1;
//	TCCR1B |= (1<<CS10) | (1<<CS12);   //1024 prescalar
	TCCR1B |= (1<<CS10);   //no prescalar
	EIFR=  0b00000010;    //INT1 flag cleared before enabling it
	EIMSK= 0b00000110;      //setting interrupt1 on and interrupt0 off uncomment after
	EICRA=0X0C;              //setting interrupt1 for rising edge uncomment after

}


ISR(INT1_vect)
{
//	tcntres=65535-TCNT1; 
	tcntres=TCNT1;
	TCCR1B=0X00;
	flag1=1;
	EIMSK &= ~(1<<INT1);
	EIMSK = 0b00000100;
//	flag1=1;
}

ISR(INT2_vect)     //RESET ISR
{  
	TCCR1B=0X00;
	overflow=0;
	TCNT1=0;
	tcntres=0;
	LCD_Clear();
	flow=0.00;
	ticks=0.00;
	flag0=0;
	flag1=0;
	memset(flowres,0,20);
	memset(ticksres,0,20);
	LCD_String_xy(0,0,"NETEL FLOWMETER");
	LCD_String_xy(1,0,"    V6.0    ");
	EIFR=  0b00000001; //delete it after for int1 to work properly
	EIMSK = 0b00000101; //INT0 AND INT2 ACTIVATED
	EICRA=0b00000011;   //DELETE AFTER
	sei();
}



ISR (TIMER1_OVF_vect)  //TIMER OVERFLOW ISR
{
	overflow++;
//	TCNT1=0; 
}

void timer1_init()
{
//	TCNT1=65380;     //from calculation 
	TCNT1=0;     //from calculation 
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
	
	float ID=1.82;             //diameter=8.0mm so radius of capillary in mm is d/2 which is 4.0mm(change to 4.0mm)
//	float ID=4.01;             //diameter=8.0mm so radius of capillary in mm is d/2 which is 4.0mm(change to 4.0mm)

	//	int height=50;	   //height of capillary in mm(change to 50)
//	float height=50.0;	   //height of capillary in mm(change to 50)

	int height=47;	   //height of capillary in mm(change to 50)

	double pie=3.14159265358; //pie value used for volume calculation
	
	LCD_String_xy(0,0,"NETEL FLOWMETER");
	LCD_String_xy(1,0,"    V6.0    ");

	
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
//			ticks=(((float)overflow) * 8.4) + (((float)tcntres) * 0.0001);  //for 1024 prescalar using timer1(tcnt value 0)

			//ticks=(((float)overflow) * 0.0082) + (((float)tcntres) * 0.125);  //using timer1(tcnt value 0) prescalar removed
			ticks=((overflow) * 0.008192) + ((tcntres) * 0.000000125);  //using timer1(tcnt value 0) prescalar removed
			ticks=roundf(ticks * 100) / 100;
	
	
			sprintf(ticksres,"time=%f    ",ticks);
			flow=((volume)/ticks);           //on 21 jan
			flow=roundf(flow * 100) / 100;
		//	sprintf(flowres,"t=%.2f fl=%.1lf",ticks,flow);
			sprintf(flowres,"fl=%lf",flow);
		//	LCD_String_xy(0,0,"FLOW COUNT STOP ");
			LCD_String_xy(0,0,ticksres       );
			LCD_String_xy(1,0,flowres);
			memset(flowres,0,20);
			memset(ticksres,0,20);
			ticks=0.00;
			tcntres=0;
			overflow=0;
			flow=0.00;
			flag1=0;
		}
	}
}
	