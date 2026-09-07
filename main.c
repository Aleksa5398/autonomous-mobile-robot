/*
 * File:   main.c
 * Author: markoooooo
 *
 * 
 */

#define FCY 10000000
//        Y 10 000 000
#define ECHO1 PORTBbits.RB1
#define ECHO2 PORTBbits.RB3 //neki vec

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <p30fxxxx.h>
#include <libpic30.h>
#include <stdbool.h>

#include "outcompare.h"
#include "uart_driver.h"
#include "Tajmeri.h"
#include "adc.h"
#include "tajmer2.h"

// DSPIC30F4013 Configuration Bit Settings

// 'C' source line config statements

// FOSC
#pragma config FOSFPR = XT_PLL4         // Oscillator (XT w/PLL 4x)
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_16        // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512       // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV20          // Brown Out Voltage (Reserved)
#pragma config BOREN = PBOR_ON          // PBOR Enable (Enabled)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>


//C:\Mikroe\M\aaa.exe -w -${Device} -q -f"D:\PRIM\projekat2.X\dist\default\production\projekat2.X.production.hex"

 //_FOSC(CSW_FSCM_OFF & XT_PLL4); // instrukcioni takt je isti kao i kristal
 //_FWDT(WDT_OFF);

unsigned int stoperica, stoperica2;
unsigned int senzorNapred, brojlevi, brojlevi2;
unsigned int sirovi0;
unsigned int stanje;
//stanja
//0 = full stop, ceka BT signal
//1 = kretanje napred
//2 = skretanje/rotiranje levo 
//3 = skretanje/rotiranje desno
//4 = kretanje nazad
//5 = korekcija kursa (stanje 1+)


char dists[6]="";

void PWMdesni(int pwm)
{
    PR2 = 500;//odredjuje frekvenciju po formuli
    OC1RS = pwm;//postavimo pwm
    OC1R = 1000;//inicijalni pwm pri paljenju samo
    OC1CON  = OC_IDLE_CON & OC_TIMER2_SRC & OC_PWM_FAULT_PIN_DISABLE & T2_PS_1_256;//konfiguracija pwma
                   
    T2CONbits.TON = 1;//ukljucujemo timer koji koristi
}

void PWMlevi(int pwm)
{
    PR2 = 500;//odredjuje frekvenciju po formuli
    OC2RS = pwm;//postavimo pwm
    OC2R = 1000;//inicijalni pwm pri paljenju samo
    OC2CON = OC_IDLE_CON & OC_TIMER2_SRC & OC_PWM_FAULT_PIN_DISABLE & T2_PS_1_256;//konfiguracija pwma
             
    T2CONbits.TON = 1;//ukljucujemo timer koji koristi
} 

void Delay_us (int vreme)//funkcija za kasnjenje u 10mikro
{
	stoperica2 = 0;
	while(stoperica2 < vreme);
}

void Delay_ms (int vreme)//funkcija za kasnjenje u milisekundama
{
	stoperica = 0;
	while(stoperica < vreme);
}

void __attribute__ ((__interrupt__,no_auto_psv)) _T1Interrupt(void) // svakih 1ms
{
	TMR1 =0;
    stoperica++;//brojac za funkciju Delay_ms
	IFS0bits.T1IF = 0;
}

void __attribute__ ((__interrupt__,no_auto_psv)) _T2Interrupt(void) // svakih 10us
{
	TMR2 =0;
    stoperica2++;//brojac za funkciju Delay_us
	IFS0bits.T2IF = 0;    
}

void __attribute__ ((__interrupt__,no_auto_psv)) _T3Interrupt(void) // svakih 10us
{
	TMR3 =0;
	IFS0bits.T3IF = 0;    
}

void __attribute__ ((__interrupt__,no_auto_psv)) _T4Interrupt(void) // svakih 10us
{
	TMR4 =0;
	IFS1bits.T4IF = 0;    
}

void __attribute__((__interrupt__,no_auto_psv)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;
    
    rx_buffer[buffer_last++]=U1RXREG;
    buffer_last &= MAX_BUFFER_SIZE - 1;
    
    if (buffer_size < MAX_BUFFER_SIZE)
    {
        buffer_size++;
    }
}

void __attribute__((__interrupt__,no_auto_psv)) _U2RXInterrupt(void) 
{
    IFS1bits.U2RXIF = 0;
    
    rx_buffer[buffer_last++]=U2RXREG;
    buffer_last &= MAX_BUFFER_SIZE - 1;
    
    if (buffer_size < MAX_BUFFER_SIZE)
    {
        buffer_size++;
    }
}

void __attribute__((__interrupt__,auto_psv)) _ADCInterrupt(void) 
{   
    
	sirovi0=ADCBUF0;
    
    sirovi0=30000-sirovi0*10;//skaliranje analognog senzora
										
    IFS0bits.ADIF = 0;
}

void delay (uint16_t delay_time)
{
    uint16_t i, j;
    
    for (i = 0; i < delay_time; i++)
        for (j = 0; j < 1000; j++);
}

int LeviNapred(){
    LATBbits.LATB0=1;   // ukljucimo trig signal
    Delay_us(5);        // duzina signala 50us (ne moze manje)
    LATBbits.LATB0=0;   // ugasimo trig signal
    //Delay_us(1);
    while (ECHO1 == 0);//cekaj echo
    TMR3=0;
    IFS0bits.T3IF = 0;
    T3CONbits.TON = 1;
    while (ECHO1 == 1); //dok je echo 1
    T3CONbits.TON = 0;
    unsigned int time_measured = TMR3;
    TMR3=0;
    unsigned int dist = (time_measured)*0.343/0.2;
    return dist;
}

int LeviNazad3(){            // tmr3
    LATBbits.LATB2=1;   // ukljucimo trig signal
    Delay_us(5);        // duzina signala 50us (ne moze manje)
    LATBbits.LATB2=0;   // ugasimo trig signal
    //Delay_us(1);
    while (ECHO2 == 0);//cekaj echo
    TMR3=0;
    IFS0bits.T3IF = 0;
    T3CONbits.TON = 1;
    while (ECHO2 == 1); //dok je echo 1
    T3CONbits.TON = 0;
    unsigned int time_measured3 = TMR3;
    TMR3=0;
    unsigned int dist2 = (time_measured3)*0.343/0.2;
    return dist2;
}

void ispisStanje (){uart2WriteString("stanje: ");   WriteUART2dec2string(stanje);    uart2WriteString(" \r\n");}
void ispisStanje1 (){uartWriteString("stanje: ");   WriteUART1dec2string(stanje);    uartWriteString(" \r\n");}

void proveraSenzora()
    {   
        senzorNapred = sirovi0;
        brojlevi = LeviNapred();//provera senzora levi napred
        brojlevi2 = LeviNazad3();//provera senzora levi nazad ODKOMENTARISI KAD GA STAVIS
    }

void ispisSenzora()
    {   
        uart2WriteString("\r\n sen sirovi: ");
        WriteUART2dec2string(sirovi0);
        uart2WriteString("\r\n sen digital: ");
        WriteUART2dec2string(brojlevi);
        uart2WriteString("\r\n sen digit 2: ");
        WriteUART2dec2string(brojlevi2);
        uart2WriteString("\r\n ");
    }

void stopp () {LATFbits.LATF0=0; LATFbits.LATF1=0;LATBbits.LATB9=0; LATBbits.LATB10=0;}

void leftf () {LATFbits.LATF0=0; LATFbits.LATF1=1;}
void leftb () {LATFbits.LATF1=0; LATFbits.LATF0=1;}

void rightf () {LATBbits.LATB9=0; LATBbits.LATB10=1;}
void rightb () {LATBbits.LATB10=0; LATBbits.LATB9=1;;}

int main(int argc, char** argv)
{         
    //inicijalizacije
    Init_T1();
    Init_T2();
    Init_T3();
    //Init_T4();
    //uartInit();
    uart2Init();
    
    ADCinit();
    ADCON1bits.ADON=1;
    
    ADPCFGbits.PCFG8=0; // =0 sharp analogni
    TRISBbits.TRISB8=1; // =1 sharp RB8 kao ulazni/citamo signal
    
    //ADPCFGbits.PCFG5=0; // =0 sharp analogni
    //TRISBbits.TRISB5=1; // =1 sharp RB5 kao ulazni/citamo signal
    
    
    //setovanje ulaza/izlaza i analognih pinova
    TRISBbits.TRISB0=0; //trig1 na izlaz
    TRISBbits.TRISB1=1; //echo1 na ulaz
    ADPCFGbits.PCFG0=1; //trig1 je digitalni
    ADPCFGbits.PCFG1=1; //echo1 je digitalni
    
    TRISBbits.TRISB2=0; //trig2 na izlaz
    TRISBbits.TRISB3=1; //echo2 na ulaz
    ADPCFGbits.PCFG2=1; //trig2 je digitalni
    ADPCFGbits.PCFG3=1; //echo2 je digitalni
 
    TRISDbits.TRISD0=0;  //PWM1
    TRISDbits.TRISD1=0;  //PWM2
    TRISFbits.TRISF0=0;  //IN1
    TRISFbits.TRISF1=0;  //IN2
    TRISBbits.TRISB9=0;  //IN3
    TRISBbits.TRISB10=0; //IN4
    
   
    Delay_ms(1000);//cekamo da tenk dodje sebi :P
    uartWriteString("START  1 aleksa`1 \r\n");
    uart2WriteString("START 2 \r\n");
    
    // za proveru komm
    uint8_t len = 0;
    uint8_t str[MAX_BUFFER_SIZE];
    
    //pri pokretanju tenka prvo krece napred
    leftf();
    rightf();
    PWMlevi(0);
    PWMdesni(0);
    stanje = 0;
    sirovi0 = 0;
    
    int dst= 14000;
    
    int skreni= 30000;
    int skreni2= 22500;
    
    int i;
    int desno = 0;
    int desno1 = 0;
    int levo = 0;
    int levo1 = 0;
    int poravnanjeLevo = 1000;
    int poravnanjeDesno = 1200;
    int zidmin = 12500;
    int zidmax = 16000;
    
    
    while(1)
    {   
        proveraSenzora();
        //ispisSenzora();
        
        // blutut kontrola
        len = uartReadString(str);
        for(i=0; i<len; i++)
        {
            if(str[i]=='S')
            { if((str[i+1]=='T') && (str[i+2]=='A') && (str[i+3]=='R') && (str[i+4]=='T')){
                   //uartWriteString("primljen start\r\n");
                   uart2WriteString("primljen start2\r\n");
                   stanje=10;
                }
                else if((str[i+1]=='T') && (str[i+2]=='O') && (str[i+3]=='P')){
                   //uartWriteString("primljen stop\r\n");
                   uart2WriteString("primljen stop2\r\n");
                   stanje=0;
                }
            }   
        }

        //Delay_ms(1);
        
        // stanja
        if(stanje == 1)//kretanje napred
        {
            leftf();
            rightf();
            PWMlevi(400);
            PWMdesni(400);
            
            //prelasci stanja
            if(senzorNapred < dst)//ako je zid napred preblizu                    
            {   
                uart2WriteString("STOP NAPRED -idi desno---st3\r\n");
                //ispisSenzora();
                stanje = 3;   //skreni desno
                PWMlevi(0);   //stani do sledeceg stanja
                PWMdesni(0);
                desno=0;
                desno1=0;
            }
            
            else if((brojlevi > skreni))//ako ne vidi zadnji levi senozr zid
            {
                if(brojlevi2 < skreni2 ) Delay_ms(1000);
                uart2WriteString("STOP LEVO skreni levo -----st0\r\n");
                //ispisSenzora();
                stanje = 2;       //skreni levo
                PWMlevi(0);       //stani do sledeceg stanja
                PWMdesni(0);
                levo = 0;
                levo1 = 0;
            }
            
            else if ((abs(brojlevi - brojlevi2) > 500) || (brojlevi < zidmin) || ((brojlevi > zidmax)&&(brojlevi< skreni)) ) {//ako zidovi nisu poravnati
                stanje = 5;//korekcija
                uart2WriteString("KOREKCIJA KURSA\r\n");
                //ispisSenzora();
            }
            
            Delay_ms(5);//mali delay da ne zaglupi, !!PROMENI PO POTREBI!!
        }
        
        else if(stanje == 2){//skretanje levo
            leftb();            // rotiraj u levo
            rightf();           //
            PWMlevi(400);       //
            PWMdesni(400);      //
            Delay_ms(1500);     // rotiraj vreme 
            PWMlevi(0);         // 
            PWMdesni(0);        //
            Delay_ms(100);      //stop vreme
            leftf();            //
            rightf();           //
            PWMlevi(400);       // 
            PWMdesni(400);      //
            Delay_ms(1200);     // idi pravo vreme
            stanje=1;           // idi u stanje 1
            /*
            leftb();
            rightf();
            PWMlevi(430);
            PWMdesni(370);
            if(levo1==0) Delay_ms(1500);
            levo=1;
            proveraSenzora();
            uart2WriteString("\r\n sen digital: ");
            WriteUART2dec2string(brojlevi);
            uart2WriteString("\r\n sen digit 2: ");
            WriteUART2dec2string(brojlevi2);
            uart2WriteString("\r\n ");
            /*if(brojlevi < 15000){//ako je prisao zidu levo napred
                levo++;
                if(levo == 1){
                    stanje = 1;//kreni napred
                    PWMlevi(0);//stani do sledeceg stanja
                    PWMdesni(0);
                    uart2WriteString("UHVATIO ZID krecem pravo\r\n");
                    ispisSenzora();
                    Delay_ms(100);
                    leftf();
                    rightf();
                    PWMlevi(400);
                    PWMdesni(400);
                    Delay_ms(2000);//kada se okrenuo dovoljno idi pravo pola sekunde da ne udje odmah u rezim skretanje levo
                }
            }*/
            /*
            if(abs(brojlevi - brojlevi2) < 5000){//ako su levi napred i levi nazad priblizno isti
                levo++;
                if(levo == 1){
                    stanje = 10;//kreni napred
                    PWMlevi(0);//stani do sledeceg stanja
                    PWMdesni(0);
                    uart2WriteString("NAZAD NEMA NISTA krecem pravo\r\n");
                    ispisSenzora();
                    Delay_ms(100);
                    leftf();
                    rightf();
                    PWMlevi(400);
                    PWMdesni(400);
                    Delay_ms(1000);//kada se okrenuo dovoljno idi pravo pola sekunde da ne udje odmah u rezim skretanje levo
                }
            }*/
            
            Delay_ms(5);//mali delay da ne zaglupi, !!PROMENI PO POTREBI!!
        }
        
        else if(stanje == 3){//skretanje desno
            leftf();
            rightb();
            PWMlevi(425);
            PWMdesni(425);
            if(desno1==0) Delay_ms(400);
            desno1=1;
            
            if(abs(brojlevi - brojlevi2) < poravnanjeDesno){//ako su levi napred i levi nazad priblizno isti
                desno++;
                if(desno==3){
                    stanje = 1;//kreni napred
                    desno=0;
                    desno1=0;
                    PWMlevi(0);//stani do sledeceg stanja
                    PWMdesni(0);
                    uart2WriteString("kecem pravo 2\r\n");
                    ispisSenzora();
                }
            }
            
            Delay_ms(5);//mali delay da ne zaglupi, !!PROMENI PO POTREBI!!
        }
        
        else if(stanje == 5){// korekcija
            
            //prelasci stanja
            if(senzorNapred < dst)//ako je zid napred preblizu                    
            {   
                uart2WriteString("STOP NAPREDk -idi desno---st3\r\n");
                //ispisSenzora();
                stanje = 3;   //skreni desno
                PWMlevi(0);   //stani do sledeceg stanja
                PWMdesni(0);
                desno=0;
                desno1=0;
            }
            
            else if((brojlevi > skreni))//ako ne vidi zadnji levi senozr zid
            {
                if(brojlevi2 < skreni2) Delay_ms(1000);
                uart2WriteString("STOP LEVOk skreni levo -----st0\r\n");
                //ispisSenzora();
                stanje = 2;       //skreni levo
                PWMlevi(0);       //stani do sledeceg stanja
                PWMdesni(0);
                levo = 0;
                levo1 = 0;
            }
            
            else if ((abs(brojlevi - brojlevi2) < 500) && (brojlevi > zidmin) && (brojlevi < zidmax)){//ako zidovi nisu poravnati
                stanje = 1;//korekcija
                ispisSenzora();
            }
            
            //korekcija
            if(/*(brojlevi>brojlevi2) || */(brojlevi>zidmax)){
                //korekcija rotacijom oko desnog tocka
                leftf();
                rightf();
                PWMlevi(430);//stavi na 100 ili nesto sitno ako zaglupi
                PWMdesni(370);
                //za korekciju rotacijom oko ose staviti rigthb() i PWMdesni isti kao PWMlevi (500)
            }
            else if((brojlevi<zidmin)){//ako je levo nazad blize nego levo napred
                //korekcija rotacijom oko levog tocka
                leftf();
                rightf();
                PWMlevi(370);
                PWMdesni(440);//stavi na 100 ili nesto sitno ako zaglupi
                
                //za korekciju rotacijom oko ose staviti leftb() i PWMdesni isti kao PWMlevi (500)
            }
            else if((brojlevi>brojlevi2)){//ako je levo nazad blize nego levo napred
                //korekcija rotacijom oko levog tocka
                leftf();
                rightf();
                PWMdesni(370);
                PWMlevi(430);//stavi na 100 ili nesto sitno ako zaglupi
                
                //za korekciju rotacijom oko ose staviti leftb() i PWMdesni isti kao PWMlevi (500)
            }
            else{           // idi napred
                leftf();
                rightf();
                PWMlevi(400);//stavi na 100 ili nesto sitno ako zaglupi
                PWMdesni(400);
            }
            
            Delay_ms(5);//mali delay da ne zaglupi, !!PROMENI PO POTREBI!!   
        }
        
        else if(stanje == 10){// nazad
            leftb();
            rightf();
            PWMlevi(0);
            PWMdesni(0);
            //izlazak iz stanja 0 ide preko BT kontrole na pocetku while petlje (isto vazi i za ulazak u stanje 0)
        }
        
        else if(stanje == 0){// full stop, ceka BT signal
            stopp();
            leftf();
            rightf();
            PWMlevi(0);
            PWMdesni(0);
            if(sirovi0 > dst) stanje=1;
            //izlazak iz stanja 0 ide preko BT kontrole na pocetku while petlje (isto vazi i za ulazak u stanje 0)
        }
               
    } //za while
    return (EXIT_SUCCESS);
}