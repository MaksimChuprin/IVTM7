#include  "define.h"
#include  "modbus.h"

static U8               spiBuffer[MESSAGELEN + 2];
static U16              varTimeR;

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
  static U8     transmitNum;  
  
  // ѕроверка на вшивость
  if( !(ConfWordCopy & RADIO_ON) || (Errors & (INFOMEM_ERROR + RT_ERROR)) ) 
  {
    MS_Clock      = 0;
    TXRX_READYINT_DIS;
    SX1276_Sleep(); 
    Flags.TransmitOn = Flags.TransmitDone = Flags.ReceiveDone = Flags.timeout = 0;
    return;
  }
    
  U32   current_RadioChannel     =  UnitBand * CHANEL_SIZE + FREQ_CARRY; 
  U8    current_BW_Setting       = (RadioSpeed - 1) + BW_Setting;
  U8    current_POWEROUT_Setting =  RadioPower; 
  U16   crc  = 0;
  U32   time = 0;
  
  // обработка событий: передача выполнена, прием выполнен, таймаут приема
  if( Flags.timeout )
  {
    Flags.timeout = 0;
    SX1276_Standby ( );
    
    // передано
    if( Flags.TransmitDone )
    {
      Flags.TransmitDone = 0;
      
      SX1276_Write1       ( REG_LR_IRQFLAGS, 0xff );    // стереть флаги прерываний
      SX1276_SetChannel   ( current_RadioChannel );
      // bandwidth,  datarate, coderate,  preambleLen,  timeout, fixLen, payload, crcOn, freqHopOn, hopPeriod, iqInverted, rxcontinue 
      SX1276_SetRxConfig  ( current_BW_Setting, DATARATE_Setting, CODERATE_Setting, PREAMBLELEN_Setting, 0, false, 0, true, false, 0, true, true ); 
      SX1276_Rx           ( );      
      TXRX_READYINT_EN;    
      MS_Clock = SEC( SX1276_TimeOnAir(MESSAGEMAXLEN) + .1 );   // задать врем€ ожидани€ 
      return;
    }
    
    // прин€то
    if( Flags.ReceiveDone )
    {
      Flags.ReceiveDone = 0;      
      
      U8 irqFlags   = SX1276_Read1( REG_LR_IRQFLAGS );
      U8 messageLen = SX1276_Read1( REG_LR_RXNBBYTES );
      SX1276_Write1 ( REG_LR_IRQFLAGS, 0xff );          // стереть флаги прерываний
      if( (irqFlags & RFLR_IRQFLAGS_PAYLOADCRCERROR ) || !(irqFlags & RFLR_IRQFLAGS_VALIDHEADER) || (messageLen > MESSAGELEN) )  { SX1276_Sleep(); return; }
                                                    
      SX1276_ReadFifo( spiBuffer, messageLen );
      SX1276_Sleep();
      if( spiBuffer[0] != UnitAddress ) return;
      
      switch( spiBuffer[1] )
      {                
        case ASKTIME_LORA:  if( messageLen == 8) 
                            {
                              crc = GetModbusRTUCRC(spiBuffer, 6);
                              if( crc != *((pU16)&spiBuffer[6]) )  break;
                            }
                            else if( messageLen != 6 ) break;
          
                            time = spiBuffer[2] + ((U32)spiBuffer[3] << 8) + ((U32)spiBuffer[4] << 16) + ((U32)spiBuffer[5] << 24);
                            if( time >= ValidTime )
                            {
                              _DINT(); Time = time; _EINT();                              
                            }
                            STACKMINUS;
                            break;
                            
        case ASK_LORA:      if( messageLen == 2) STACKMINUS;
                            break;
                            
        case NOASK_LORA:    if( messageLen == 2) transmitNum = 0;
                            break;                            
                            
        case NOASKTIME_LORA:    
                            if( messageLen == 8) 
                            {
                              crc = GetModbusRTUCRC(spiBuffer, 6);
                              if( crc != *((pU16)&spiBuffer[6]) )  break;
                            }
                            else if( messageLen != 6 ) break;
                            
                            time = spiBuffer[2] + ((U32)spiBuffer[3] << 8) + ((U32)spiBuffer[4] << 16) + ((U32)spiBuffer[5] << 24);
                            if( time >= ValidTime )
                            {
                              _DINT(); Time = time;  _EINT();                              
                            }                            
                            transmitNum = 0;
                            break; 
      }
      return;
    }
    
    // таймаут
    TXRX_READYINT_DIS;
    SX1276_Sleep(); 
    return;
  }
            
  // сформировать пакет дл€ передачи - по WritePeriod
  if( Time2Write == 1 )
  {
    // подготовка данных
    varTimeR       = SX1276_Random() * WritePeriod / 2 + 1; // разброс выхода в эфир
    SX1276_Sleep();
    TXRX_READYINT_DIS; 
    MS_Clock      = 0;  
    Flags.TransmitDone = Flags.ReceiveDone = Flags.timeout = 0;
    
    U16     errors  = 0;
    S16     t       = round(Tempr * 10);
    
    if(Errors & TS_ERROR)       errors |= (1<<0);    
    if(Errors & HS_ERROR)       errors |= (1<<1);    
    if(Errors & PRESS_ERROR)    errors |= (1<<2); 
    if(ConfigWord & NO_HUM)     errors |= (1<<6);  
    if(ConfigWord & PRES)       errors |= (1<<7);    
                                errors |= (1<<4); // ѕризнак нового ћ4 Practik
         
    // stack++
    if( Time < ValidTime )      { saveCounter = 1; }
    else                        { STACKPLUS;       } 
    
    saveBuffer[0][0]  = LORA_TYPE;
    saveBuffer[0][1]  = UnitAddress;              
    _DINT();    saveBuffer[0][2]  = Time; saveBuffer[0][3]  = Time >> 8; saveBuffer[0][4]  = Time >> 16; saveBuffer[0][5]  = Time >> 24; _EINT();
    saveBuffer[0][6]  = ConfigWord >> 0;   saveBuffer[0][7]  = ConfigWord >> 8;
    saveBuffer[0][8]  = errors >> 0;       saveBuffer[0][9]  = errors >> 8;
    saveBuffer[0][10] = PowerShow >> 0;    /* saveBuffer[0][11] = PowerShow  >> 8; */ saveBuffer[0][11] = UnitBand;
    saveBuffer[0][12] = (Int16U)t & 0xff;  saveBuffer[0][13] = (Int16U)t >> 8;          
    saveBuffer[0][14] = round(Humidy);     
    saveBuffer[0][15] = round(Pressure - 700);
    
    transmitNum       = PacketNum & 0xf;
    if( PacketNum & STACK_DIS ) saveCounter = 1;
  }
  
  // разброс на выход в эфир
  if( varTimeR )                { varTimeR--; return; } 
  
  // трансивер зан€т
  if( TXRX_INPROGRESS )         return; 
    
  // проверка наличи€ данных на передачу
  if( !saveCounter )            return;  
  
  // проверка количества попыток
  if( !transmitNum )            return;  
          
  // проверить канал     
  // bandwidth,  datarate,  coderate, preambleLen, timeout, fixLen, payload, crcOn, freqHopOn, hopPeriod, iqInverted, rxcont
  SX1276_Init();
  SX1276_SetRxConfig( current_BW_Setting, DATARATE_Setting, CODERATE_Setting, PREAMBLELEN_Setting, 0, false, 0, true, false, 0, true, true );    
  if( !SX1276_IsChannelFree( current_RadioChannel, LOW_TRESH_FREE ) )
  {
    SX1276_Sleep ( ); // эфир зан€т +1 sec
    return;
  }
            
  // старт передачи
  SX1276_Write1      ( REG_LR_IRQFLAGS, 0xff );   // стереть флаги прерываний        
  SX1276_SetChannel  ( current_RadioChannel );
                      // power, bandwidth, datarate, coderate, preambleLen, fixLen, crcOn, freqHopOn, hopPeriod, iqInverted 
  SX1276_SetTxConfig ( current_POWEROUT_Setting, current_BW_Setting, DATARATE_Setting, CODERATE_Setting, PREAMBLELEN_Setting, false, true, false, 0, true );  
  memcpy(spiBuffer, &saveBuffer[0][0], MESSAGELEN);
  crc = GetModbusRTUCRC(spiBuffer, MESSAGELEN);
  spiBuffer[16] = crc; spiBuffer[17] = crc >> 8;
  SX1276_Send        ( spiBuffer, MESSAGELEN + 2 );
    
  TXRX_READYINT_EN;
  Flags.TransmitOn = 1;
  transmitNum--;
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

