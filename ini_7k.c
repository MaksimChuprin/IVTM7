#include  "define.h"

U8    sd_write_block    (U32 sector)    { }
U8    sd_read_block     (U32 sector, pU8 pBuffer) { }
U8    sd_initialization (void)          { }
U8    sd_erase_block    (U32 starting_sector, U32 total_sectors) { }
U8    spi_xmit_byte     (U8 byte)       { }
void  SX1276_Sleep      (void)          { }
void  SX1276_Standby    (void)          { }
void  dp1203_sleep      (void)          { }
void  sd_spi_clear      (void)          { }

void  choose_uart_speed(U16 speed)
{
  switch(speed)
  {
    case    S115200: UCA0BRW=   18;  UCA0MCTL= (2 << 1); break;   // 115200
    case    S57600:  UCA0BRW=   36;  UCA0MCTL= (3 << 1); break;
    case    S38400:  UCA0BRW=   56;  UCA0MCTL= (5 << 1); break;
    case    S19200:  UCA0BRW=  109;  UCA0MCTL= (2 << 1); break;
    case    S9600:   UCA0BRW=  218;  UCA0MCTL= (4 << 1); break;
    case    S4800:   UCA0BRW=  436;  UCA0MCTL= (7 << 1); break;
    
    default:         UCA0BRW=  218;  UCA0MCTL= (4 << 1);          // 9600 
  } 
}

void ini_spec(void)
{    
  PMAPKEYID  =  PMAPKEY;
  P2MAP0     =  PM_MCLK;
  PMAPKEYID  =  PMAPKEY;
  P2MAP1     =  PM_MCLK;
  
  PMAPKEYID  =  PMAPKEY;
  PMAPCTL    =  0;
  PMAPKEYID  =  0; 
  
  UCA0CTL1   =  UCSSEL__SMCLK + UCSWRST; // SMCLK (2MHz)
  UCA0CTL0   =  0; 
  UCA0STAT   =  0;
  choose_uart_speed(S9600);                
  UCA0CTL1   =  UCSSEL__SMCLK;
  
  P2SEL     |=  (BIT4 + BIT5);  
  P2OUT     |=  (BIT2 + BIT3);
  P2OUT     &=  ~BIT6;
  P2DS      |=  (BIT0 + BIT1);
  P6DS      |=  (BIT6 + BIT7);
    
  ADC12CTL1  = ADC12CSTARTADD_7 + ADC12SHP + ADC12CONSEQ_1 + ADC12DIV_2 + ADC12SSEL_0;  // М7, софт запуск, одна последовательность, ADC12OSC / 3 ~ 1.8 MHz  
  ADC12MCTL7 = ADC12SREF_1 + ADC12INCH_12;              // питание
  ADC12MCTL8 = ADC12SREF_1 + ADC12INCH_12;              // питание
  ADC12MCTL9 = ADC12SREF_1 + ADC12INCH_2 + ADC12EOS;    // давление 
  ADC12IFG   = 0;
  ADC12IE    = BIT9;     

  switch( ConfWordCopy & DEVTYPE_MASK )
  {
    case K7_TYPE: sprintf(ID,"%s %s", ID_K7, Version);
                  break;
                
    case K1P_TYPE:
                  sprintf(ID,"%s %s", ID_KP1, Version);  
                  break;
  }  
  
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
    Errors        = 0;
    PressureParam = 0;
  }
    
  PowerShow       =  100;
  AdcGain         =  ADC_GAIN / 32768.;
  Ref15           =  Uref     / 4095.;  
  Time2Write      =  30;  
  STATSize        =  FLASHENDADR - 0x200 - StartStatistic;
  // убираем sprintf из прерывания USB
  S16 k = strlen( ID );
  sprintf( &ID[k], " VID=%04x PID=%04x EAL=%04x ", USBD_VID, USBD_PID, ((ConfWordCopy & PRES) ? (EAL + 1) : EAL));    
}