//flow meter using timer1 without prescalar and 5ms delay 
#define F_CPU 8000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include<string.h>
#define LED PE6
#define LCD_Command_Dir DDRE
#define LCD_Command_Port PORTE
#define LCD_Data_Dir DDRC
#define LCD_Data_Port PORTC
#define RS PE4
#define EN PE2
#define RW PE3

float ticks=0.00;
volatile unsigned long overflow;    //original
unsigned int tcntres=0;
double volume=0.00;       //for volume calulation
double flow=0.00;		   //for flow calculation flow=vol*60/sec
char tcntresult[20]={0};
char overresult[20]={0};
char ticksres[20]={0};
char flowres[20]={0};

ISR(INT0_vect)
{
	PORTE=0b00000000;     //led off
	TCCR1B |= (1<<CS10);   //no prescalar and normal mode
	EIFR=  0b00000010;    //INT1 flag cleared before enabling it
	EIMSK= 0b00000110;      //setting interrupt1 on and interrupt0 off uncomment after
	EICRA=0X0C;              //setting interrupt1 for rising edge uncomment after
	sei();
//	while(1)
//	{
//		
//	}	  	
}


ISR(INT1_vect)
{
	tcntres=65535-TCNT1;
	PORTE=0b01000000; 
	TCCR1B=0X00;
//	ticks=(((float)overflow) * 0.009984) + (((float)tcntres) * 0.000128);  //for 1024 prescalar with calculation changes
	
	ticks=(((float)overflow) * 0.005) + (((float)tcntres) * 0.000000125);  //for 1024 prescalar using timer1

	//sprintf(ticksres,"t=%.2f",ticks);
	//LCD_String_xy(0,0,ticksres);
	
	//flow=((volume*60)/ticks);
	flow=((volume)/ticks);           //on 21 jan
	sprintf(flowres,"t=%.3f fl=%lf",ticks,flow);  //changed from .2f to .3f on 21 jan
	LCD_String_xy(0,0,flowres);
	//sprintf(tcntresult,"tcnt=%u ov=%lu  ",tcntres,overflow);
	sprintf(tcntresult,"ov=%lu vo=%lf  ",overflow,volume);
	//LCD_String_xy(1,0,tcntresult);
	LCD_String_xy(1,0,tcntresult);
	memset(flowres,0,20);
	memset(tcntresult,0,20);
	ticks=0.00;
	tcntres=0;
	overflow=0;
	flow=0.00;
	EIMSK &= ~(1<<INT1);
	EIMSK = 0b00000100;
}

ISR(INT2_vect)
{
//	EIMSK = 0b00000000;  //delete it after for int1 to work properly
	LCD_Clear();
//	PORTE=~PORTE;		// Toggle PORTE  
	TCCR1B=0X00;
	overflow=0;
	tcntres=0;
	flow=0.00;
	ticks=0.00;
	memset(flowres,0,20);
	memset(tcntresult,0,20);
//	LCD_Clear();
	LCD_String_xy(0,1,"WELCOME TO");
	LCD_String_xy(1,1,"NETEL FLOWMETER");
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

void LCD_Command(unsigned char cmnd)
{
	LCD_Data_Port= cmnd;
	LCD_Command_Port &= ~(1<<RS);	/* RS=0 command reg. */
	_delay_ms(1);
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 Write operation */
	_delay_ms(1);
	LCD_Command_Port |= (1<<EN);	/* Enable pulse */
	_delay_ms(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(1);
}

void LCD_Char (unsigned char char_data)	/* LCD data write function */
{
	LCD_Data_Port= char_data;
	LCD_Command_Port |= (1<<RS);	/* RS=1 Data reg. */
	_delay_ms(1);
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 write operation */
	_delay_ms(1);
	LCD_Command_Port |= (1<<EN);	/* Enable Pulse */
	_delay_ms(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(1);
}

void LCD_Init(void)
{
	LCD_Command_Dir = 0xFF;
	LCD_Data_Dir = 0xFF;		
	_delay_ms(20);			
	
	LCD_Command (0x38);		//initialize in 8-bit mode
	_delay_ms(1);
	LCD_Command (0x0C);		//display on cursor off
	_delay_ms(1);
	LCD_Command (0x06);		//autoincrement cursor
	_delay_ms(1);
	LCD_Command (0x01);		//clear screen
	_delay_ms(1);
	LCD_Command (0x80);     //cursor at starting point
}

void LCD_Clear(void)
{
	LCD_Command (0x01);		/* clear display */
	_delay_ms(1);
	LCD_Command (0x80);		/* cursor at home position */
	_delay_ms(1);
}

void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);		/* Call LCD string function */
}

int main ( )
{
	DDRE=0XFF; //MAKE PORTE OUTPUT PORT
	DDRD=0X00; //MAKE PORT D AS INPUT
	PORTE=0b01000000;   //LED GLOW FOR INDICATION
	PORTD=0XFC;  //except pd1 and pd0 all pins are high
	
    LCD_Init();  //intialize lcd
	
	//float ID=1.7;             //diameter=3.3mm so radius of capillary in mm is d/2 which is 1.65mm
	float ID=4;             //diameter=3.3mm so radius of capillary in mm is d/2 which is 1.65mm
	//int height=47;	   //height of capillary in mm
	int height=50;	   //height of capillary in mm
	double pie=3.14159265358; //pie value used for volume calculation
	
	LCD_String_xy(0,0,"WELCOME TO");
	LCD_String_xy(1,0,"NETEL FLOWMETER");
	_delay_ms(1000);
//	volume=(pie*(ID)*(ID)*height)/1000;   //volume in mm3 which is converted into ml by dividing it with 1000
	
	volume=((pie*(ID)*(ID)*height)/1000)*60;   //volume in mm3 which is converted into ml by dividing it with 1000 on 21 jan
	
//	EIMSK |= (1<<INT0); //set int0 interrupt
	EIMSK = 0b00000101; //set int0 and int2 interrupt 
//	EICRA=0b00100011;  //INT0 CONFIGURED FOR rising EDGE AND INT2 FOR FALLING EDGE
	EIMSK &= ~(1<<INT1);
	EICRA=0b00000011;  //INT0 CONFIGURED FOR rising EDGE AND INT2 FOR LOW LEVEL 
 

	
	sei();
	timer1_init();
	LCD_Clear();
	while(1)
	{
		//DO NOTHING
		_delay_ms(1000);
		PORTE=~PORTE;
		
	}
}
	