#include <p30fxxxx.h>
#include "tajmeri.h"

#define TMR2_period 100 //  Fosc = 10MHz, 10us
#define TMR1_period_us 10000 // 1ms 
#define TMR11 100000 // 10* tmr1

//#define TMR2_period 8 /*  Fosc = 10MHz,
					         // 1/Fosc = 0.1us !!!, 0.1us * 1000 = 0.1ms  */

void Init_T1(void)//tajmer za digitalni senzor 1
{
	TMR1 = 0;
	PR1 = TMR1_period_us;
	
	T1CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T2 interrupt pririty (0-7)
//	SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T1IF = 0; // clear interrupt flag
	IEC0bits.T1IE = 1; // enable interrupt

	T1CONbits.TON = 1; // T1 on
}

void Init_T2(void)//tajmer za delay ms
{
	TMR2 = 0;
	PR2 = TMR2_period;
	
	T2CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T2 interrupt pririty (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T2IF = 0; // clear interrupt flag
	IEC0bits.T2IE = 1; // enable interrupt

	T2CONbits.TON = 1; // T2 on
}

void Init_T3(void)//tajmer za digitalni senzor 2
{
	TMR3 = 0;
    PR3 = 10*TMR1_period_us;
	// unsigned int aprr = TMR11;
	//PR3 = aprr;
	
	T3CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T2 interrupt pririty (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T3IF = 0; // clear interrupt flag
	IEC0bits.T3IE = 1; // enable interrupt

	T3CONbits.TON = 0; // T3 off, upali pri merenju echo pina 
}

void Init_T4(void)//tajmer za digitalni senzor 2
{
	TMR4 = 0;
    PR4 = 10*TMR1_period_us;
	// unsigned int aprr = TMR11;
	//PR3 = aprr;
	
	T4CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T2 interrupt pririty (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS1bits.T4IF = 0; // clear interrupt flag
	IEC1bits.T4IE = 1; // enable interrupt

	T4CONbits.TON = 0; // T3 off, upali pri merenju echo pina 
}