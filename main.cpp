/*
 * main.cpp
 *
 *  Created on: 05/09/2013
 *      Author: João Sousa
 */




#include <msp430.h>
#include "msp430g2231-calibration.h"

#define BLUE_LED	BIT0
#define GREEN_LED	BIT1
#define RED_LED		BIT3
#define FAN			BIT6

void init_io(void);
void init_timerA(void);
void config_pwm(void);
void init_adc(void);
void start_adc_sampling(void);


unsigned long int ms = 0;
unsigned long int s = 0, m = 0;
char new_val_adc = 0;


int main()
{
	WDTCTL = WDTPW + WDTHOLD;

	init_timerA();

	config_pwm();

	init_io();

	init_adc();

	// interrupts enabled
	__bis_SR_register(GIE);

	P1OUT |= FAN;
	while(1)
	{
		if(new_val_adc)
		{
			new_val_adc = 0;
		}
	}

	return 0;
}

void init_io(void)
{
	P1DIR |= BLUE_LED + GREEN_LED + RED_LED + FAN;
}

void init_timerA(void)
{
	BCSCTL1 = CALBC1_16MHZ;            			// Set DCO to 8MHz
	DCOCTL = CALDCO_16MHZ;
	CCR0 = 80;
	TACTL = TASSEL_2 + MC_1;
	CCTL0 = CCIE;                            // CCR0 interrupt enabled
}

void config_pwm(void)
{
	CCTL1 = OUTMOD_6;
	CCR1 = 10;
	P1SEL = BIT6;
}

void init_adc(void)
{
	ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; 	// ADC10ON, interrupt enabled
	ADC10CTL1 = INCH_7;                       		// input A7
	ADC10AE0 |= BIT7;                         		// PA.7 ADC option select
}
void start_adc_sampling(void)
{
	ADC10CTL0 |= ENC + ADC10SC;
}

//===========================================================================
// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	ms++;

	if(ms >= 200000)
	{
		ms = 0;
		s++;
		P1OUT ^= BLUE_LED + GREEN_LED + RED_LED ;
		start_adc_sampling();
	}
	if(s >= 60)
	{
		s = 0;
		m++;

	}
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
	new_val_adc = 1;
}
