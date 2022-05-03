#include  "define.h"

/* ###################################################################### */
#pragma vector=ADC12_VECTOR
__interrupt void adc_isr(void)
{   
  ALL_SENS_OFF;
  ADC12CTL0 &= ~ADC12ENC;
  ADC12CTL0 &= ~(ADC12ON + ADC12REFON);
  ADC12IFG   =  0;
  Flags.measure_ready = 1;
  __low_power_mode_off_on_exit();
}

/* ###################################################################### */
#pragma vector=TIMER0_A1_VECTOR
__interrupt void ta0_isr(void)
{          
  static U16   rtc;  
  static U8    last_key = NONE;  
  static U8    KeyClock;
  static U8    state    = WAITING;
  
  TA0CTL &= ~TAIFG;
  
  // sd таймер
  if(SD_Clock)
  {
    if(SD_Clock > (TA0CCR0 + 1))          SD_Clock -= (TA0CCR0 + 1);
    else                                  SD_Clock  = 0;
  }
  
  // master таймер
  if(MS_Clock)
  {
    if(MS_Clock > (TA0CCR0 + 1))          MS_Clock -= (TA0CCR0 + 1);
    else
    {
      TXRX_READYINT_DIS;
      MS_Clock      = 0;
      Flags.timeout = 1;
      __low_power_mode_off_on_exit();
    }
  }  
  
  // RTC 
  rtc += (TA0CCR0 + 1);
  if(rtc >= 1024)
  {
    rtc -= 1024;
    Time++;
    // таймеры с шагом 1 сек
    if(Time2Write)    Time2Write--;
    if(GenClock1)     GenClock1--;
    if(SleepClock)    SleepClock--;
    if(BlueClock)     BlueClock--; 
      
    Flags.systick    = 1;
    __low_power_mode_off_on_exit();                  // для сброса WDT и подсчета времени
  }

  switch( ~(P7IN & (BIT5 + BIT6)) & 0x60)
  {
     case  BIT5: if(last_key == SW1) KeyClock++;
                 else KeyClock  = 0; 
                 last_key       = SW1; 
                 break;
                 
     case  BIT6: if(last_key == SW2) KeyClock++;
                 else KeyClock  = 0; 
                 last_key       = SW2;
                 break;
                                                  
     default:    if( ( KeyClock < 3) || ( KeyClock > 14) ) { last_key  = NONE;  }
                 else                                      { Flags.key = 1; KeyCode = last_key; __low_power_mode_off_on_exit(); } // короткое нажатие
                 KeyClock  = 0;                 
  }
  
  if(KeyClock == 15) 
  { 
    Flags.key = 1; 
    KeyCode   = last_key + LONGKEY_F; // длинное нажатие
    __low_power_mode_off_on_exit(); 
  }
  
#if  ( defined (__7M1_PRA_MSD_USER) || defined (__7M2_PRA_MSD_USER) )
  switch(state)
  {
    case WAITING:     if( ISNOMEAS_MODE ) // 0.05 сек
                      {
                        TA0CCR0  = 49; 
                        break;
                      }
      
                      if( Flags.measure_ready || !Flags.systick || (Time & 1) ) // измерение раз в 2 сек
                      {
                        TA0CCR0  = 49; 
                        break;
                      }

                      HS_ON;
                      if(ConfWordCopy & PRES)
                      {
                        TA0CCR0  = 19;
                        state    = START_PRES;
                      }
                      else
                      {
                        TA0CCR0  = 39;
                        state    = START_RES;
                      }
                      break;

    case START_PRES:  PS_ON;
                      state    = START_RES;
                      TA0CCR0  = 19; 
                      break;
                        
    case START_RES:   TS_ON;
                      state   = START_MES;
                      TA0CCR0 = 9;
                      break;
               
    case START_MES:   ADC12CTL0 |= (ADC12REFON + ADC12ON);  // ADC12 ON
                      state      = WAITING;
                      TA0CCR0    = 49;
                      ADC12CTL0 |= (ADC12ENC + ADC12SC);    // Start Convertion                     
                      break;
  }  
  
#elif defined (__7K_PRA_MSD_USER)  
  switch(state)
  {
    case WAITING:     if( ISNOMEAS_MODE ) // 0.05 сек
                      {
                        TA0CCR0  = 49; 
                        break;
                      }
      
                      if( Flags.measure_ready || !Flags.systick || (Time & 1) ) // измерение раз в 2 сек
                      {
                        TA0CCR0  = 49; 
                        break;
                      }

    case START_PRES:  PS_ON;
                      state    = START_MES;
                      TA0CCR0  = 19; 
                      break;
                                       
    case START_MES:   ADC12CTL0 |= (ADC12REFON + ADC12ON);  // ADC12 ON
                      state      = WAITING;
                      TA0CCR0    = 49;
                      ADC12CTL0 |= (ADC12ENC + ADC12SC);    // Start Convertion                     
                      break;
  } 
#else
  #error "No Type defined!"
#endif        
}

/* ###################################################################### */
#pragma vector=TIMER2_A1_VECTOR
__interrupt void ta2_isr(void)
{   
  STOP_TA2;  
    
  switch( ConfWordCopy & DEVTYPE_MASK )
  {
    case M3_TYPE: if( IS_RXIEN ) 
                  { 
                    // принят пакет
                    RXI_DISABLE; 
                    Flags.uart_receive = 1;                     
                    __low_power_mode_off_on_exit();
                    break; 
                  }
                  else 
                  { 
                    // отправлен пакет
                    if(Flags.respeeduart) 
                    { 
                      choose_uart_speed(ConfWordCopy & SPEED_MASK); 
                      Flags.respeeduart = 0; 
                    }                    
                    RS_TX_DS; 
                    SET_U0_1SB;
                    RXI_ENABLE;                     
                    break; 
                  }
                                           
    case M7_TYPE: RXI_DISABLE; 
                  Flags.uart_receive = 1; 
                  __low_power_mode_off_on_exit(); 
                  break;
                  
    case K7_TYPE:
    case K1P_TYPE:  
                  if( IS_TIMEOUT )
                  {
                    // таймаут
                    Flags.timeout = 1;
                    RS7K_RX_DS;
                    RS7K_TX_DS;
                    RXI_DISABLE;
                    TXI_DISABLE;
                    __low_power_mode_off_on_exit();
                  }
                  else 
                  {
                    if( IS_RXIEN ) 
                    { 
                      // принят пакет                     
                      Flags.uart_receive = 1;                    
                      RXI_DISABLE;
                      RS7K_RX_DS;
                      __low_power_mode_off_on_exit();
                    }
                    else 
                    { 
                      // отправлен пакет                      
                      RS7K_TX_DS;
                      RS7K_RX_EN;
                      RXI_ENABLE;
                      START_TA2_S(0.1);
                    }
                  }
                  break;
  }  
}

/* ###################################################################### */

// USCI_A0
#pragma vector = USCI_A0_VECTOR
__interrupt void uart_a0_isr(void)
{    
  static   U16          txPointer;     
  static   U8           nDATA;         
  static   U8           atCOM;         
  static   U8           dtCOM;  
  
  U8                    statbyte;  
      
  switch( ConfWordCopy & DEVTYPE_MASK )
  {
   case M3_TYPE:
   /* ############# M3 ############### */
   switch(UCA0IV)
   {
    case 2: statbyte = UCA0STAT;
            InByte   = UCA0RXBUF;            
               
            // overhead && err
            if( ( RxCounter == UART_BUF_LEN ) || (statbyte & UCRXERR) ) RxCounter = 0;
            // load buffer
            UartBuffer[RxCounter++] = InByte;
            // перезапуск таймаута по каждому байту - 5 sym
            START_TA2_5ST;
            break;
             
    case 4: if( txPointer == TxByte2send ) 
            { 
              txPointer = 0;
              TXI_DISABLE; 
              START_TA2_5ST;  // включить прием ч-з 5 байт
            }                       
            else  UCA0TXBUF = UartBuffer[txPointer++];
            break;
   }
   break;
   /* ############# M3 ############### */
   
   case M7_TYPE:
  /* ############# M7 ################### */
   switch(UCA0IV)  
   {
    case 2:     
            statbyte = UCA0STAT;
            InByte   = UCA0RXBUF;
            
            // overhead && err
            if( ( RxCounter == UART_BUF_LEN ) || (statbyte & UCRXERR) ) 
            { 
              UartBuffer[0] = RxCounter = 0; 
              FORCE_TA2_INT; 
              break; 
            }
            
            // AT or DATA
            if( RxCounter == 0 )
            {
              if(InByte == 'A') 
              { 
                atCOM = 1;      // прием ответа от модема
                dtCOM = 0; 
              }
              else  
              {
                atCOM = 0;
                if(InByte == '!') dtCOM = 1; // прием данных по каналу BT
                else              break;
              }
            }
            
            // num DATA
            if( (RxCounter == 1) && dtCOM ) nDATA = InByte;     // количество байт для приема: !<NN>.... - посылка по каналу BT
            
            // load buffer
            UartBuffer[RxCounter++] = InByte;
      
            // check AT - конец 
            if( atCOM && (InByte == 0x0A) ) 
            {
              //  конец ответа от модема
              UartBuffer[RxCounter] = 0;
              FORCE_TA2_INT;
              break;
            }
            
            // check DATA
            if( dtCOM && (nDATA == RxCounter) ) 
            {
              //  конец данных по каналу BT
              FORCE_TA2_INT;  
              break;
            }          
            
            // перезапуск таймаута по каждому байту
            START_TA2(1);
            break;
              
             
    case 4: if( txPointer == TxByte2send )
            {
              UartBuffer[0] = txPointer = 0;
              TXI_DISABLE;              
              RXI_ENABLE;              
              START_TA2(5);                                       // таймаут приема - 5 сек
            }
            else UCA0TXBUF = UartBuffer[txPointer++];
            break;
   }
   break;
   /* ############# M7 ################### */  
   
   case K7_TYPE:
   case K1P_TYPE:     
   /* ############# K ############### */
   switch(UCA0IV)
   {
    case 2: statbyte = UCA0STAT;
            InByte   = UCA0RXBUF;            
               
            // overhead && err
            if( ( RxCounter == UART_BUF_LEN ) || (statbyte & UCRXERR) ) { RxCounter = 0; break; }
            // load buffer
            UartBuffer[RxCounter++] = InByte;
            // перезапуск таймаута, таймаут 5 байт
            START_TA2_B(5);
            break;
             
    case 4: if( txPointer == TxByte2send ) 
            { 
              txPointer = 0;
              TXI_DISABLE; 
              START_TA2_B(1);  // включить прием ч-з 1 байт
            }                       
            else  UCA0TXBUF = UartBuffer[txPointer++];
            break;
   }
   break;   
   /* ############# K ############### */
  }  
}

#pragma vector = USCI_B0_VECTOR
__interrupt void spi_b0_isr0(void)
{  
   volatile U8          inbyte;
   static   U8          txPointer;
      
   switch(UCB0IV)
   {
      case 2:  inbyte   =  UCB0RXBUF;   
               UCB0STAT =  0;
               break;
               
      case 4:  if(txPointer == R433_txnum)
               {                 
                 DP1203_SPI_TXI_DISABLE;
                 dp1203_sleep();
                 txPointer = R433_txnum = 0;
                 __low_power_mode_off_on_exit();
                 break;
               }
               UCB0TXBUF = UartBuffer[txPointer++];
               break;
   }
}

#pragma vector = PORT2_VECTOR
__interrupt void port2_isr(void)
{
  TXRX_READYINT_DIS;
  
  if( Flags.TransmitOn ) 
  {
    Flags.TransmitOn   = 0;
    Flags.TransmitDone = 1;
  }
  else Flags.ReceiveDone = 1;
  
  MS_Clock      = 0;
  Flags.timeout = 1;
  __low_power_mode_off_on_exit(); 
}

#pragma vector = UNMI_VECTOR
__interrupt void usernmi_isr(void)
{   
  SOFT_RESET;
}

#pragma vector = SYSNMI_VECTOR
__interrupt void sysnmi_isr(void)
{   
  SOFT_RESET;
}
