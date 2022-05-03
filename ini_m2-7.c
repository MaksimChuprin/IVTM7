#include  "define.h"

void  choose_uart_speed(U16 speed)
{
  switch(speed)
  {
    case    S115200: UCA0BRW=   18;  UCA0MCTL= (2 << 1); TA2CCR0 = SMCLK_HZ * 11 * 5 / 115200;  break;   // 115200
    case    S57600:  UCA0BRW=   36;  UCA0MCTL= (3 << 1); TA2CCR0 = SMCLK_HZ * 11 * 5 / 57600 ;  break;
    case    S38400:  UCA0BRW=   56;  UCA0MCTL= (5 << 1); TA2CCR0 = SMCLK_HZ * 11 * 5 / 38400 ;  break;
    case    S19200:  UCA0BRW=  109;  UCA0MCTL= (2 << 1); TA2CCR0 = SMCLK_HZ * 11 * 5 / 19200 ;  break;
    case    S9600:   UCA0BRW=  218;  UCA0MCTL= (4 << 1); TA2CCR0 = SMCLK_HZ * 11 * 5 / 9600  ;  break;
    case    S4800:   UCA0BRW=  436;  UCA0MCTL= (7 << 1); TA2CCR0 = SMCLK_HZ * 11 * 5 / 4800  ;  break;
    
    default:         UCA0BRW=  218;  UCA0MCTL= (4 << 1); TA2CCR0 = SMCLK_HZ * 11 * 5 / 9600;           // 9600 
  } 
}

void ini_spec(void)
{      
  switch( ConfWordCopy & DEVTYPE_MASK )
  {                
    case M2_TYPE: 
                // for LPS22HB
                PMAPKEYID =  PMAPKEY;
                P2MAP1    =  PM_UCB0SIMO;
                PMAPKEYID =  PMAPKEY;
                P2MAP2    =  PM_UCB0SOMI;
                PMAPKEYID =  PMAPKEY;
                P2MAP3    =  PM_UCB0CLK;
                P2SEL    |=  (BIT1+BIT2+BIT3);
                
                UCB0CTL1  =  UCSSEL__SMCLK + UCSWRST;           // SMCLK (2MHz)
                UCB0CTL0  =  UCSYNC + UCMST + UCMSB + UCCKPH;
                UCB0STAT  =  0;
                UCB0BRW   =  SMCLK_HZ / 250000L - 1;            // 250 kHz
                UCB0CTL1  =  UCSSEL__SMCLK;
                
                sprintf(ID,"%s %s", ID_M2, Version);
                break;                
                
    case M3_TYPE:
                // for LPS22HB
                PMAPKEYID =  PMAPKEY;
                P2MAP1    =  PM_UCB0SIMO;
                PMAPKEYID =  PMAPKEY;
                P2MAP2    =  PM_UCB0SOMI;
                PMAPKEYID =  PMAPKEY;
                P2MAP3    =  PM_UCB0CLK;
                P2SEL    |=  (BIT1+BIT2+BIT3);
                
                UCB0CTL1  =  UCSSEL__SMCLK + UCSWRST;           // SMCLK (2MHz)
                UCB0CTL0  =  UCSYNC + UCMST + UCCKPH + UCMSB;
                UCB0STAT  =  0;
                UCB0BRW   =  SMCLK_HZ / 250000L - 1;            // 250 kHz
                UCB0CTL1  =  UCSSEL__SMCLK;
                
                // for UART RS485
                UCA0CTL1 =  UCSSEL__SMCLK + UCSWRST; 
                UCA0CTL0 =  0; 
                UCA0STAT =  0; 
                choose_uart_speed(ConfWordCopy & SPEED_MASK); 
                UCA0CTL1 =  UCSSEL__SMCLK;
                P2SEL   |=   (BIT4 + BIT5);
                RXI_ENABLE; 
                sprintf(ID,"%s %s", ID_M3, Version);   
                break; 
                  
    case M4L_TYPE:       
                PMAPKEYID =  PMAPKEY;
                P2MAP1    =  PM_UCB0SIMO;
                PMAPKEYID =  PMAPKEY;
                P2MAP2    =  PM_UCB0SOMI;
                PMAPKEYID =  PMAPKEY;
                P2MAP3    =  PM_UCB0CLK;
                UCB0CTL1  =  UCSSEL__SMCLK + UCSWRST; 
                UCB0CTL0  =  UCSYNC + UCMST + UCCKPH + UCMSB;   // master
                UCB0BRW   =  SMCLK_HZ / 250000L - 1;            // 250 kHz
                UCB0CTL1  =  UCSSEL__SMCLK;                
                P2SEL    |=  (BIT1+BIT2+BIT3);
                P2OUT    |=  BIT0;
                P2DIR    &= ~BIT6;
                P2REN    |=  BIT6;                              // прерывание по приему/передачи   
                P2IES     =  0;
                P6DIR    &=  BIT4;                              // reset up
                sprintf(ID,"%s %s", ID_M4L, Version);
                break;
                
    case M4_TYPE:                 
                PMAPKEYID =  PMAPKEY;
                P2MAP1    =  PM_UCB0SIMO;
                PMAPKEYID =  PMAPKEY;
                P2MAP2    =  PM_UCB0SOMI;
                PMAPKEYID =  PMAPKEY;
                P2MAP3    =  PM_UCB0CLK;
                UCB0CTL1  =  UCSSEL__SMCLK + UCSWRST; 
                UCB0CTL0  =  UCCKPH + UCSYNC + UCMSB;           // slave
                UCB0BRW   =  SMCLK_HZ / 250000L - 1;            // 250 kHz
                UCB0CTL1  =  UCSSEL__SMCLK;
                P2SEL    |=  (BIT1+BIT2+BIT3);
                P2OUT    |=  BIT0;                
                sprintf(ID,"%s %s", ID_M4, Version);
                break;
    
    case M6_TYPE:              
                PMAPKEYID =  PMAPKEY;
                P2MAP1    =  PM_UCB0SIMO;
                PMAPKEYID =  PMAPKEY;
                P2MAP2    =  PM_UCB0SOMI;
                PMAPKEYID =  PMAPKEY;
                P2MAP3    =  PM_UCB0CLK;
                P2SEL    |=  (BIT1+BIT2+BIT3);
                P2OUT    |=  BIT0;
                
                UCB0CTL1  =  UCSSEL__SMCLK + UCSWRST;           // SMCLK (2MHz)
                UCB0CTL0  =  UCSYNC + UCMST + UCMSB + UCCKPH;
                UCB0STAT  =  0;
                UCB0BRW   =  SMCLK_HZ / 250000L - 1;            // 250 kHz
                UCB0CTL1  =  UCSSEL__SMCLK; 
                
                sprintf(ID,"%s %s", ID_M6, Version);
                break;
                
    case M7_TYPE:
                // for LPS22HB
                PMAPKEYID =  PMAPKEY;
                P2MAP1    =  PM_UCB0SIMO;
                PMAPKEYID =  PMAPKEY;
                P2MAP2    =  PM_UCB0SOMI;
                PMAPKEYID =  PMAPKEY;
                P2MAP3    =  PM_UCB0CLK;
                P2SEL    |=  (BIT1+BIT2+BIT3);
                
                UCB0CTL1  =  UCSSEL__SMCLK + UCSWRST;           // SMCLK (2MHz)
                UCB0CTL0  =  UCSYNC + UCMST + UCCKPH + UCMSB;
                UCB0STAT  =  0;
                UCB0BRW   =  SMCLK_HZ / 250000L - 1;            // 250 kHz
                UCB0CTL1  =  UCSSEL__SMCLK;
                
                // for BT-module
                UCA0CTL1  =  UCSSEL__SMCLK + UCSWRST;           // SMCLK (2MHz)
                UCA0CTL0  =  0;
                UCA0STAT  =  0;
                choose_uart_speed(S115200);
                UCA0CTL1  =   UCSSEL__SMCLK;                                
                sprintf(ID,"%s %s", ID_M7, Version);
                break;
  }
  
  PMAPKEYID  =  PMAPKEY;
  PMAPCTL    =  0;
  PMAPKEYID  =  0;   

  ADC12MCTL0 = ADC12SREF_2 + ADC12INCH_0;   // температура
  ADC12MCTL1 = ADC12SREF_2 + ADC12INCH_0;   // температура
  ADC12MCTL2 = ADC12SREF_2 + ADC12INCH_0;   // температура
  ADC12MCTL3 = ADC12SREF_2 + ADC12INCH_0;   // температура
  ADC12MCTL4 = ADC12SREF_2 + ADC12INCH_0;   // температура
  ADC12MCTL5 = ADC12SREF_2 + ADC12INCH_0;   // температура
  ADC12MCTL6 = ADC12SREF_2 + ADC12INCH_0;   // температура
  ADC12MCTL7 = ADC12SREF_2 + ADC12INCH_0;   // температура
  ADC12MCTL8 = ADC12SREF_1 + ADC12INCH_12;  // питание
  ADC12MCTL9 = ADC12SREF_1 + ADC12INCH_2;   // давление 
  ADC12MCTL10= ADC12SREF_1 + ADC12INCH_1;   // влажность
  ADC12MCTL11= ADC12SREF_1 + ADC12INCH_1;   // влажность
  ADC12MCTL12= ADC12SREF_1 + ADC12INCH_1;   // влажность
  ADC12MCTL13= ADC12SREF_1 + ADC12INCH_1;   // влажность  
  ADC12MCTL14= ADC12SREF_1 + ADC12INCH_1;   // влажность
  ADC12MCTL15= ADC12SREF_1 + ADC12INCH_1 + ADC12EOS;   // влажность  
  ADC12IFG   = 0;
  ADC12IE    = BITF; 
  
  // if LPS22 pressure sensor  
  SD_ON;
  DELAY_USB_MS(5);
  
  if( LPS22_Read(LPS22_WHO_AM_I) == LPS22_I_AM_ALIVE )
  {    
    LPS22_Write(LPS22_CTRL_REG2, LPS22_SWRESET);                        // soft reset
    DELAY_USB_MS(25);
    LPS22_Write(LPS22_CTRL_REG2, LPS22_BOOT);                           // soft reset
    DELAY_USB_MS(25);
    LPS22_Write(LPS22_RPDS_L, 0);                                       // clear offset-L
    LPS22_Write(LPS22_RPDS_H, 0);                                       // clear offset-H
    LPS22_Write(LPS22_CTRL_REG2, LPS22_ONE_SHORT + LPS22_I2C_DIS);      // I2C disable, start one-short
    DELAY_USB_MS(25);
        
    Flags.LPS22HB = 1;
  } 
  
  SD_OFF;
  
  // убираем sprintf из прерывания USB
  S16 k = strlen( ID );
  sprintf( &ID[k], " VID=%04x PID=%04x EAL=%04x ", USBD_VID, USBD_PID, ((ConfWordCopy & PRES) ? (EAL + 1) : EAL));  
}

void ini_ram(void)
{
  // Инициализация переменных из сегмента RAM  
  
  if( strcmp(HotReset, HotResetPattern) )
  {
    memset( (void *)&Tempr, 0, RAMSIZE);
    memcpy( (void *)HotReset, (void *)HotResetPattern, 8);  
    Tempr    = UC_Temp = 25;
    Humidy   =  50;
    Pressure = 745;    
  }
  else
  {
    Errors            = 0;
    Res               = 1;
    HumidyParam       = PressureParam = 0;
  }
    
  AdcGain         =  ADC_GAIN / 32768.;
  Ref15           =  Uref     / 4095.;  
  Time2Write      =  30;
  STATSize        =  FLASHENDADR - StartStatistic - ((StartStatistic >= 0x10000) ? 0 : 0x200);  
}