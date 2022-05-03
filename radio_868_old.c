#include  "define.h"

const RadioRegisters_t RadioRegsInit[7] = 
{   
    // 868 MHz
    { REG_LR_FRFMSB           , 0xd9 },
    { REG_LR_FRFMID           , 0x20 },
    { REG_LR_FRFLSB           , 0x04 },
    // PA_BOOST pin out 2 dBm
    { REG_LR_PACONFIG         , RFLR_PACONFIG_PASELECT_PABOOST },
    // LNA Gain-max  
    { REG_LR_LNA              , RFLR_LNA_GAIN_G1 },
    // OCP - 90 mA
    { REG_LR_OCP              , RFLR_OCP_ON + RFLR_OCP_TRIM_090_MA }, 
    // PAYLOADMAXLENGTH - 64
    { REG_LR_PAYLOADMAXLENGTH , 0x40 }, 
};

void  radio868_process(void)
{   
  static U8     repeatNum;
  
  // Проверка на вшивость
  if( !(ConfWordCopy & RADIO_ON) || (Errors & (INFOMEM_ERROR + RT_ERROR)) || DP1276_TX_INPROGRESS )  return;
        
  // первый выход в эфир за 1 сек до записи статистики, зате повторы раз в секунду
  if( Time2Write == 1 )
  {
    // подготовка данных
    VarTimeR      = ADC12MEM0 & 0x0007; // + 0...7 секунд разброса на следующий выход в эфир       
    U8     errors = 0, c = 0;
  
    if(Errors & TS_ERROR)       errors |= (1<<0);    
    if(Errors & HS_ERROR)       errors |= (1<<1);    
    if(Errors & PRESS_ERROR)    errors |= (1<<2);          
    if(ConfigWord & NO_HUM)     errors |= (1<<6);  
    if(ConfigWord & PRES)       errors |= (1<<7);    
                                errors |= (1<<4); // Признак нового М4
    
    S16           i  =  round(Tempr * 10);    
    UartBuffer[c++]  = UnitAddress;              
    UartBuffer[c++]  = (Int16U)i & 0xff;   
    UartBuffer[c++]  = (Int16U)i >> 8;     
    UartBuffer[c++]  = round(Humidy);                  
    UartBuffer[c++]  = PowerShow;          
    UartBuffer[c++]  = errors; 
    UartBuffer[c++]  = round(Pressure - 700);
    repeatNum        = PacketNum;
  }
  else if( repeatNum == 0 ) return;
  
  // проверить канал
  U8    RadioChannelTx           =  UnitBand;  
  U8    current_BW_Setting       = (RadioSpeed - 1) + BW_Setting;
  U8    current_POWEROUT_Setting =  RadioPower;
     
  // bandwidth,  datarate,  coderate, preambleLen, timeout, fixLen, payload, crcOn, freqHopOn, hopPeriod, iqInverted, rxcont
  SX1276_Init();
  SX1276_SetRxConfig( current_BW_Setting, DATARATE_Setting, CODERATE_Setting, PREAMBLELEN_Setting, 100, true, MESSAGE_LEN, true, false, 0, true, true );    
  if( !SX1276_IsChannelFree( (U32)RadioChannelTx * CHANEL_SIZE + FREQ_CARRY, LOW_TRESH_FREE ) )
  {
    SX1276_Sleep ( ); // эфир занят
    return;
  }
            
  // старт передачи        
  SX1276_SetChannel  ( (U32)RadioChannelTx * CHANEL_SIZE + FREQ_CARRY );
  SX1276_SetTxConfig ( current_POWEROUT_Setting, current_BW_Setting, DATARATE_Setting, CODERATE_Setting, PREAMBLELEN_Setting, true, true, false, 0, true );  // power, bandwidth, datarate, coderate, preambleLen, fixLen, crcOn, freqHopOn, hopPeriod, iqInverted 
  SX1276_Send        ( UartBuffer, MESSAGE_LEN );
  SX1276_Write1      ( REG_LR_IRQFLAGS, 0xff );  
  TXRX_READYINT_EN;
  repeatNum--;
}

/* -------------- */
#pragma optimize=none
U8  shift_byte_spi1(U8 byte)
{
  SHIFT_BYTE_SX1276( byte );   
  return byte; 
}

/* -------------- */
void Radio_Reset (void)
{
  RES_LOW;
  DELAY_USB_MS( 1 );
  RES_HI;
  DELAY_USB_MS( 5 );
}

/* -------------- */
void spi_nss (U8 NSS)
{
  if( NSS ) NSS_HI;
  else      NSS_LOW;
}

/* -------------- */
#pragma optimize=none
void wait_ms(U32 time)
{
  for(U32 i = 0; i < time; i++) DELAY_USB_MS( 1 );
}

/* -------------- */
U8 spi_write(U8 byte)
{
  return shift_byte_spi1( byte );
}

