#include  "define.h"

const  U8       ArrayCommand[5][17] = { "$FFFFIRD7\r", "$FFFFRI100002FA\r", "$FFFFRR00000808\r", "$FFFFRR00100405\r", "$FFFFRR00140207\r" };
static U8       masterCommand       = CONFIG_ASK;
static U8       exchErrorCount      = 0;

void  master_process(void)
{  
  Flags.timeout = 0;
  if( ISSLEEP_MODE )
  {   
    masterCommand  =  CONFIG_ASK;
    exchErrorCount =  0;
    Errors        &= ~PR_ERROR;
    return;
  }
    
  // отвал по таймауту, повторные опросы каждые 2 секунды
  if(exchErrorCount == 5)       
  { 
    Errors        |= PR_ERROR; 
    exchErrorCount = 0; 
    masterCommand  = CONFIG_ASK;
    START_TA2_S(2);
    return; 
  }
    
  // повтор или первая посылка комманды
  U16   i;
  for(TxByte2send = i = 0; ArrayCommand[masterCommand][i]; i++, TxByte2send++) UartBuffer[i] = ArrayCommand[masterCommand][i];  
  RS7K_TX_EN;
  TX_START;
  exchErrorCount++;
}

// config word преобразователя
#define   MICRO_F           BIT6
#define   TNS_F             BIT8

// error's bits преобразователя
#define   P_PRESS_ERROR     BIT4
#define   P_TS2_ERROR       BIT5
#define   P_FREQ_ERROR      BIT6
#define   P_TS_ERROR        BIT7

/* ########################### разбор обмена ############################### */
void master_uart_process(void)
{    
  static U16    herrCount;
  U8            i;
  U16           cfw;
  
  Flags.uart_receive = 0;
  
  /* проверка контрольной суммы */
  if(check_sum(UartBuffer, RxCounter - 3) != hex2char(UartBuffer, RxCounter - 3)) 
  { 
    START_TA2_S(0.05);  // ошибка - повторить запрос чз 50 мс
    return; 
  }
  
  exchErrorCount  =  0;
  Errors         &= ~PR_ERROR;

  switch(masterCommand)
  {
    // вычитивание ID - пока не пользуется
    case      ID_ASK: masterCommand =   CONFIG_ASK; 
                      START_TA2_S(0.01); 
                      break;
      
    // вычитивание слова конфигурации преобразователя
    case  CONFIG_ASK: cfw  =  hex2char(UartBuffer, 7) + hex2char(UartBuffer, 9) * 256;
                      Errors  &= ~(TYPE_ERROR + TNS_F);
                      Errors  |=  ( !(cfw & MICRO_F) && CHECK_TYPE(K1P_TYPE)) ? TYPE_ERROR : 0;
                      Errors  |=  ( ((cfw & MICRO_F) && CHECK_TYPE(K7_TYPE))  ? TYPE_ERROR : 0);
                      if( Errors & TYPE_ERROR )  { START_TA2_S(2); break; }  // неверный тип преобразователя
                      
                      Errors  |=  (cfw & TNS_F) ? TNS_TYPE : 0;                      
                      masterCommand = ERROR_ASK;
                      START_TA2_S(0.01);
                      break;
      
    // вычитивание ошибок
    case ERROR_ASK:   cfw      =  hex2char(UartBuffer, 7) + hex2char(UartBuffer, 9) * 256;
                      Errors  &= ~(HS_ERROR + TS_ERROR + TS2_ERROR);
                      Errors  |=  (cfw & P_TS_ERROR)  ? TS_ERROR  : 0;
                      Errors  |=  (cfw & P_TS2_ERROR) ? TS2_ERROR : 0;
                      if( cfw & P_FREQ_ERROR )  herrCount++;
                      else                      herrCount = 0;
                      Errors  |=  (herrCount >= 3) ? HS_ERROR : 0;
                      masterCommand =   MEASURE_ASK;
                      START_TA2_S(0.01);
                      break;
                          
    // вычитивание температуры и влажности
    case MEASURE_ASK: for(i = 0; i < 4; i++) *((pU8)((U16)&Tempr + i))           = hex2char(UartBuffer, 7 + 2 * i);
                      for(i = 0; i < 4; i++) *((pU8)((U16)&Humidy_original + i)) = hex2char(UartBuffer, 15 + 2 * i);

                      if( IS_DEV_TNS )
                      {                                                
                        masterCommand = MEASURE_ASK2;
                        START_TA2_S(0.01);
                      }
                      else 
                      {                        
                        masterCommand = ERROR_ASK;                                                                      
                        START_TA2_S(1);
                      }
                      break;
                          
    // вычитивание второй температуры (в шаре)
    case MEASURE_ASK2:                          
                      for(i = 0; i < 4; i++)  *((pU8)((U16)&TemprBlack + i)) = hex2char(UartBuffer, 7 + 2 * i);
                                            
                      masterCommand =   ERROR_ASK;
                      START_TA2_S(1);
                      break;
                          
    default:          masterCommand =   CONFIG_ASK;
                      START_TA2_S(0.01);
  }  
}
/* ########################### разбор обмена ############################### */