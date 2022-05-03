#include  "define.h"

#pragma optimize=none
U16  getConfigW(void) { 
  //return M6_TYPE|SLEEP_WR;
  return  ConfigWord; 
}  

void  ini_common(void)
{
  __disable_interrupt(); 
  
  SFRIE1    =  SFRIFG1= SYSCTL= 0;       // выключение ACCVIE(разрешение flash),SFRIFG1 - основные ошибки, SYSCTL - запрет BSL
  SFRRPCR   =  BIT2 + BIT3;              // ресетный резистор вкл
  RCCTL0    =  RCKEY ;                   // ключить всё ОЗУ
  
  // WDT
  WDTCTL    =  WDTPW + WDTSSEL__VLO + WDTCNTCL + WDTIS__32K;    // ~ 2.4c
  
  // PMM
  PMMCTL0   =  PMMPW;                    // Vcore = 1,4 В
  SVSMHCTL  =  SVSHE + SVSHRVL0;         // вкл только супервизор на напр. около 2 В
  SVSMLCTL  =  SVSLE;                    // вкл только супервизор на напр. 1,4 В
  PMMRIE    =  SVSHPE + SVSLPE;          // вкл ресет от супервизоров H и L
  
    // BAKSYS
  while(BAKCTL & LOCKBAK) BAKCTL &= ~(LOCKBAK);                 // Unlock XT1 pins for operation                   
  BAKCTL   |= BAKDIS;  
  
  // set UCS
  UCSCTL6   =  XT2OFF + XT2DRIVE0 + XCAP0 + XCAP1;
  
  // ждем генератор XT1
  while(UCSCTL7 & XT1LFOFFG) UCSCTL7 &= ~XT1LFOFFG;                // Если генератор не але, то 1 устанавливается опять
  
  UCSCTL1   =  DCORSEL_2;                                          // 0.5...3.5 MHz
  UCSCTL2   =  63;                                                 // 32,768*(63+1) ~ 2MHz на выходе FLL
  UCSCTL3   =  FLLREFDIV_0 + SELREF__XT1CLK;                       // 32 kHz на вход FLL

  UCSCTL5   =  DIVA__32;                                           // ACLK = XT1/32 = 1024 Hz
  UCSCTL7   =  UCSCTL8 =  0;                                       // запретить тактирование по требованию

  // ждем генератор DCO
  while(UCSCTL7 & DCOFFG) UCSCTL7 &= ~DCOFFG; 
  SFRIFG1   &= ~OFIFG; 
  
  UCSCTL4    =  SELM__DCOCLKDIV + SELS__DCOCLKDIV + SELA__XT1CLK;         // MCLK - FLL(2MHz), SMCLK - FLL(2MHz), ACLK - XT1/32(1024Hz)
  
  // TA0, ACLK - таймер на ~50 мс
  TA0CTL     =  TASSEL_1 + MC__UP + TACLR + TAIE;       
  TA0CCR0    =  49;                                                       // первое через 0,05с
  
  // TB inclk = ACLK (1024Hz) - звук
  TBCTL      =  TBCLGRP_0 + CNTL_0 + TBSSEL_1 + ID_0 + MC_1 + TBCLR;      // 
  TBCCTL0    =  TBCCTL2= TBCCTL3= TBCCTL4= TBCCTL5= 0;
  TBCCTL1    =  OUTMOD_0 + OUT;
  TBCCR0     =  500;  
  
  // Ports init  
  // ремаппим порт 4
  PMAPKEYID  =  PMAPKEY;                  // вводим пароль
  PMAPCTL    =  PMAPRECFG;                // 
  
  // USCI_A0 UART
  PMAPKEYID  =  PMAPKEY;
  P2MAP4     =  PM_UCA0TXD;
  PMAPKEYID  =  PMAPKEY;
  P2MAP5     =  PM_UCA0RXD;  
  // Timer TB0: TB0.1 compare output Out1
  PMAPKEYID  =  PMAPKEY;
  P2MAP7     =  PM_TB0CCR1B;
    
  P2IE       =  P2DS = 0;
  P2SEL      =  BIT7;
  P2DIR      = ~BIT5; 
  P2OUT      =  BIT5;
  P2REN      =  BIT2 + BIT5;
  
    
  P5REN      =  P5OUT = P5DS = 0; 
  P5SEL      =  BIT0 + BIT1;  
  P5DIR      =  255; 
  
  P6REN      =  P6DS = P6OUT = 0; 
  P6SEL      =  BIT0 + BIT1 + BIT2;
  P6DIR      =  0xff; 
      
  P7SEL      =  BIT4;
  P7OUT      =  P7REN =  BIT5+BIT6+BIT7;
  P7DS       =  0;
  P7DIR      =  0x0f;    
  
  PJOUT      =  PJREN= 0;
  PJDS       =  BIT0 + BIT1 + BIT2;
  PJDIR      =  255;  
  
  // LCD ports LCD  - ACLK (1024 Hz)
  P1OUT      =  P1SEL= P1IE = P1DS = P1REN= 0;
  P1DIR      =  255;    
  P3OUT      =  P3SEL =  P3REN = P3DS = 0;
  P3DIR      =  255;   
  P4OUT      =  P4SEL =  P4REN = P4DS = 0;
  P4DIR      =  255; 
  P8SEL      =  P8OUT = P8DS = P8REN = 0;
  P8DIR      =  255;
  P9SEL      =  P9OUT = P9DS = P9REN = 0;
  P9DIR      =  255;  

  LCDBCTL0   = 0;  
  LCDBVCTL   = LCDBCTL1  = 0;  
  LCDBMEMCTL = LCDCLRM + LCDCLRBM;  
  LCDBBLKCTL = 0;                       // blinkin 2 Hz
  LCDBPCTL0  = 0xffff;                  // S0...S15
  LCDBPCTL1  = 0xffff;                  // S16...S31
  LCDBPCTL2  = 0x01ff;                  // S32...S47
  LCDBPCTL3  = 0;                       // S48...S50  
  LCDBCTL0   = (10 << 11);              // f(LCD) = 100
  LCDBCTL0  |= (5 << 0); 
    
  // USCA0 - uart
  UCA0CTL1   =  UCSSEL__SMCLK + UCSWRST; // SMCLK (2MHz)
   
  // USCB0 - spi
  UCB0CTL1   =  UCSSEL__SMCLK + UCSWRST;                            // SMCLK (2MHz)
      
  // COMP_B
  CBCTL0     =  CBCTL2= CBCTL3= CBINT= 0;
  
  // USB  
  USBKEYPID  =  USBKEY; 
  USBCNF     =  0;                                                  // выкл USB
  USBPHYCTL  =  PUSEL;                                              // вкл DP и DM 
  USBPWRCTL  =  USBDETEN;  
  
  // ADC12 + REF
  REFCTL0    = 0;
  ADC12CTL0  = ADC12SHT0_4 + ADC12SHT1_4 + ADC12MSC;                                    // множественное преобразование
  ADC12CTL1  = ADC12CSTARTADD_0 + ADC12SHP + ADC12CONSEQ_1 + ADC12DIV_2 + ADC12SSEL_0;  // М0, софт запуск, одна последовательность, ADC12OSC / 3 ~ 1.8 MHz
  ADC12CTL2  = ADC12RES_2 + ADC12REFBURST;                                              // разрешение 12-бит, буффер опоры по требованию 
  
  sprintf(ID,"UNKNOWN %s", Version);
}
