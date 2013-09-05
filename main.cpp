/*
 * main.cpp
 *
 *  Created on: 05/09/2013
 *      Author: João Sousa
 */




#include <msp430.h>
#include <stdint.h>
#include "msp430g2231-calibration.h"

#define BLUE_LED		BIT0
#define GREEN_LED		BIT1
#define RED_LED			BIT3
#define FAN				BIT6

#define SOFT_PWM_CCR0	100
#define T_MIN_FAN_ADC	370
#define T_MAX_FAN_ADC	470


void init_io(void);
void init_timerA(void);
void config_pwm(void);
void init_adc(void);
void start_adc_sampling(void);
void set_fan_speed(int16_t vel);
long map(long x, long in_min, long in_max, long out_min, long out_max);
void set_rgb_value(int16_t red, int16_t green, int16_t blue);

unsigned long int ms = 0;
unsigned long int s = 0, m = 0;
char new_val_adc = 0;
unsigned long int soft_pwm_counter = 0;
uint16_t tick = 0;
uint32_t last_adc = 0;
unsigned int red_led = 0;
unsigned int green_led = 0;
unsigned int blue_led = 0;

int main()
{
	WDTCTL = WDTPW + WDTHOLD;

	init_timerA();

	config_pwm();

	init_io();

	init_adc();

	// interrupts enabled
	__bis_SR_register(GIE);

	set_fan_speed(100);

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
	CCR1 = 0;
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

void set_fan_speed(int16_t vel)
{
	if(vel >= 1 && vel <= 100)
	{
		CCR1 = map(vel, 10, 100, 15, 80);
	}
	else if(vel < 1)
	{
		CCR1 = 0;
	}
	else
	{
		CCR1 = 80;
	}
}

void set_rgb_value(int16_t red, int16_t green, int16_t blue)
{
	if(red < 0)		red = 0;
	if(green < 0)	green = 0;
	if(blue < 0)	blue = 0;
	if(red > 100)	red = 100;
	if(green > 100)	green = 100;
	if(blue > 100)	blue = 100;

	red_led = red;
	green_led = green;
	blue_led = blue;
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
//===========================================================================
// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	ms++;
	tick++;
	soft_pwm_counter++;

	int16_t fan_now;

	if(ms >= 200000)
	{
		ms = 0;
		set_fan_speed(fan_now);
		set_rgb_value(fan_now, fan_now,50);
	}

	if(tick > 2000)
	{
		tick = 0;

		start_adc_sampling();
		last_adc = (last_adc * 90) /100 + (ADC10MEM * 10 / 100);

		fan_now = map(last_adc, T_MIN_FAN_ADC, T_MAX_FAN_ADC, 0, 100);

	}

	if(soft_pwm_counter >= SOFT_PWM_CCR0)
	{
		soft_pwm_counter = 0;
	}
	// red led
	if(soft_pwm_counter >= red_led)
	{
		P1OUT |= RED_LED;
	}
	else
	{
		P1OUT &= ~RED_LED;
	}
	// green led
	if(soft_pwm_counter >= green_led)
	{
		P1OUT |= GREEN_LED;
	}
	else
	{
		P1OUT &= ~GREEN_LED;
	}
	// blue led
	if(soft_pwm_counter >= blue_led)
	{
		P1OUT |= BLUE_LED;
	}
	else
	{
		P1OUT &= ~BLUE_LED;
	}
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{

}
