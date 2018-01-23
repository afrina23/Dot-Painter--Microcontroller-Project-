#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "my_header.h"	/* Include LCD header file */
#include "matrix.h"

#define  Trigger_pin	PD7 /* Trigger pin */
#define NUMBER_OF_SAMPLES 10

int TimerOverflow = 0;
char stringY[10], stringX[10];
int distanceX, distanceY;
double arrayX[20], arrayY[20];

int myDrawingMode = DRAW_MODE;

ISR(TIMER1_OVF_vect)
{
	TimerOverflow++;	/* Increment Timer Overflow count */
}
double Mode(double a[],int n) {
	double maxValue = 0;
	int  maxCount = 0, i, j;

	for (i = 0; i < n; ++i) {
		int count = 0;
		double diff;
		for (j = 0; j < n; ++j) {
			diff=abs(a[j]-a[i]);
			if (diff<.6)
			++count;
		}
		
		if (count > maxCount) {
			maxCount = count;
			maxValue = a[i];
		}
	}

	return maxValue;
}

int main(void)
{
	MCUCSR = (1 << JTD);
	MCUCSR = (1 << JTD);
	
	//char string[10];
	long count;
	double distance;
	int x,y;
	
	DDRB|=0b00010000;  //for selecting between two sensors setting PB4=1
    DDRD =0x80;		/* Make trigger pin as output */
	DDRD &= (0b11110111);
	initialize_for_matrix_output();
	PORTD = 0xFF;		/* Turn on Pull-up */
	
	LCDInit(LS_BLINK);
	LCDWriteStringXY(0, 0, "Ultrasonic");
	
	sei();			/* Enable global interrupt */
	TIMSK = (1 << TOIE1);	/* Enable Timer1 overflow interrupts */
	TCCR1A = 0;		/* Set all bit to zero Normal operation */

	while(1)
	{
		
		//select the 1st sonar sensor
		PORTB &= 0b11101111;//setting PB4=0;
		
		/* Give 10us trigger pulse on trig. pin to HC-SR04 */
	for(x=0;x<NUMBER_OF_SAMPLES;x++){	
		PORTD |= (1 << Trigger_pin);
		_delay_us(10);
		PORTD &= (~(1 << Trigger_pin));
		
		TCNT1 = 0;	/* Clear Timer counter */
		TCCR1B = 0x41;	/* Capture on rising edge, No prescaler*/
		TIFR = 1<<ICF1;	/* Clear ICP flag (Input Capture flag) */
		TIFR = 1<<TOV1;	/* Clear Timer Overflow flag */

		/*Calculate width of Echo by Input Capture (ICP) */
		
		while ((TIFR & (1 << ICF1)) == 0);/* Wait for rising edge */
		TCNT1 = 0;	/* Clear Timer counter */
		TCCR1B = 0x01;	/* Capture on falling edge, No prescaler */
		TIFR = 1<<ICF1;	/* Clear ICP flag (Input Capture flag) */
		TIFR = 1<<TOV1;	/* Clear Timer Overflow flag */
		TimerOverflow = 0;/* Clear Timer overflow count */

		while ((TIFR & (1 << ICF1)) == 0);/* Wait for falling edge */
		count = ICR1 + (65535 * TimerOverflow);	/* Take count */
		/* 8MHz Timer freq, sound speed =343 m/s */
	//	distance = (double)count / 58.3;
		arrayX[x] =round( (double)count / 58.3);
		
	}
		distance = Mode(arrayX,NUMBER_OF_SAMPLES);
		distanceX=round(distance);
		dtostrf(distanceX, 2, 2, stringX);/* distance to string */
		strcat(stringX, " cm   ");	/* Concat unit i.e.cm */
		LCDClear();
		LCDWriteStringXY(0, 0, "X = ");
		LCDWriteStringXY(5, 0, stringX);	/* Print distance */
		_delay_ms(200);
			
			///selecting 2nd sensor
			PORTB |= 0b00010000;//PB4=1;
			
			/* Give 10us trigger pulse on trig. pin to HC-SR04 */
		for(y=0;y<NUMBER_OF_SAMPLES;y++){	
			PORTD |= (1 << Trigger_pin);
			_delay_us(10);
			PORTD &= (~(1 << Trigger_pin));
			
			TCNT1 = 0;	/* Clear Timer counter */
			TCCR1B = 0x41;	/* Capture on rising edge, No prescaler*/
			TIFR = 1<<ICF1;	/* Clear ICP flag (Input Capture flag) */
			TIFR = 1<<TOV1;	/* Clear Timer Overflow flag */

			/*Calculate width of Echo by Input Capture (ICP) */
			
			while ((TIFR & (1 << ICF1)) == 0);/* Wait for rising edge */
			TCNT1 = 0;	/* Clear Timer counter */
			TCCR1B = 0x01;	/* Capture on falling edge, No prescaler */
			TIFR = 1<<ICF1;	/* Clear ICP flag (Input Capture flag) */
			TIFR = 1<<TOV1;	/* Clear Timer Overflow flag */
			TimerOverflow = 0;/* Clear Timer overflow count */

			while ((TIFR & (1 << ICF1)) == 0);/* Wait for falling edge */
			count = ICR1 + (65535 * TimerOverflow);	/* Take count */
			/* 8MHz Timer freq, sound speed =343 m/s */
			//distance = (double)count / 58.3;
			arrayY[y] =round( (double)count / 58.3);
	
		}
			distance = Mode(arrayY,NUMBER_OF_SAMPLES);		distanceY=round(distance);
			dtostrf(distanceY, 2, 2, stringY);/* distance to string */
			strcat(stringY, " cm   ");	/* Concat unit i.e.cm */
			LCDWriteStringXY(0, 1, "Y = ");
			LCDWriteStringXY(5, 1, stringY);	/* Print distance */
			_delay_ms(200);
		
		
		set_mode_of_painting(myDrawingMode);
		set_cursor_from_sensor_data(distanceX, distanceY);
		draw_board();	
		
		
		if (PIND & (1<<PD3)){
			LCDClear();
			LCDWriteStringXY(0,0, "Erase!!");
			reset_board();
			
			DDRD |= (0b100001000);
			PORTD&=(0b11110111);
			DDRD &= (0b11110111);
			_delay_ms(100);
		}
		
		
	}
}