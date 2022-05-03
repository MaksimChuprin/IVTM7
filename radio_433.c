#include  "define.h"

static U8               varTimeR;

void  radio433_process(void)
{ 
  static U8     repeatNum;
  
  if( !(ConfWordCopy & RADIO_ON) || (Errors & (INFOMEM_ERROR + RT_ERROR) ) || DP1203_TX_INPROGRESS ) return;
  
  // первый выход в эфир за 1 сек до записи статистики, зате повторы раз в секунду
  if( Time2Write == 1 )
  {
    varTimeR      = ADC12MEM0 & 0x0007; // + 0...7 секунд разброса на следующий выход в эфир
    S16         i = round(Tempr * 10); 
    UartBuffer[0] = 0xff; 
    UartBuffer[1] = '!'; 
    UartBuffer[2] = 9; 
    UartBuffer[3] = UnitAddress;
    UartBuffer[4] = (U16)i & 0xff;
    UartBuffer[5] = (U16)i >> 8;
    UartBuffer[6] = round(Humidy);
    UartBuffer[7] = PowerShow;
    
    UartBuffer[8] = Errors & 0x0e; // маска ошибок
    UartBuffer[8]|= (1<<4);        // Признак нового М4
    if(Errors & PRESS_ERROR)    UartBuffer[8] |= (1<<0);
    if(ConfigWord & NO_HUM)     UartBuffer[8] |= (1<<6);  
    if(ConfigWord & PRES)       UartBuffer[8] |= (1<<7);
    
    UartBuffer[9] = round(Pressure - 700);
    UartBuffer[10]= check_sum_CRC8( &UartBuffer[1], UartBuffer[2]);
    
    UartBuffer[11]= UartBuffer[12] = 0xff;
    repeatNum     = PacketNum;
  }
  else if( repeatNum == 0 ) return;  
  
  if( varTimeR ) { varTimeR--; return; } // разброс на выход в эфир
    
  R433_txnum    = 13;
  dp1203_stby(UnitBand);
  dp1203_tx();
  DP1203_SPI_TX_START;      
  repeatNum--;
}
