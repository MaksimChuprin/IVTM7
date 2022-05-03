#include  "define.h"

void  choose_uart_speed (U16 speed)     { }
U8    sd_write_block    (U32 sector)    { return ERROR; }
U8    sd_read_block     (U32 sector, pU8 pBuffer) { return ERROR; }
U8    sd_initialization (void)          { return ERROR; }
U8    sd_erase_block    (U32 starting_sector, U32 total_sectors) { return ERROR; }
U8    spi_xmit_byte     (U8 byte)       { return ERROR; }
void  SX1276_Sleep      (void)          { }
void  dp1203_sleep      (void)          { }
void  sd_spi_clear      (void)          { }

void ini_spec(void)
{  
  P2DIR     &=  ~BIT5;
  P2OUT     |=   BIT5;  
  P2REN     |=   BIT5;
  
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
      
  // Инициализация переменных из сегмента RAM  
  if( strcmp(HotReset, HotResetPattern) )
  {
    memset( (void *)&Tempr, 0, RAMSIZE);
    memcpy( (void *)HotReset, (void *)HotResetPattern, 8);  
    Tempr    = UC_Temp = 25;
    Humidy   = 50;
    Pressure = 745;
  }
  else
  {
    Errors            = 0;
    Res               = 1;
    HumidyParam       = PressureParam = 0;
  }   
    
  Time2Write      =  30;
  AdcGain         =  ADC_GAIN / 32768.;
  Ref15           =  Uref     / 4095.;
  STATSize        =  FLASHENDADR - 0x200 - StartStatistic;
  
  sprintf(ID,"%s %s VID=%04x PID=%04x EAL=%04x ", ID_M1, Version, USBD_VID, USBD_PID, ((ConfWordCopy & PRES) ? (EAL + 1) : EAL) ); 
}
