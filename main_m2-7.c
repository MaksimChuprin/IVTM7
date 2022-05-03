#include  "define.h"

void main(void)
{    
  // общая инициализация
  ini_common();
  ini_ram();
  CheckInfoFlash();
  ConfWordCopy = getConfigW();
  __enable_interrupt();  
  msp_usb_init ();
  
  // проверка конфигурации   
  if( CHECK_TYPE(TYPE_NODEF) || CHECK_TYPE(M1_TYPE) || CHECK_TYPE(K7_TYPE) || CHECK_TYPE(K1P_TYPE) ) printf("CnFnO");
  for(; CHECK_TYPE(TYPE_NODEF) || CHECK_TYPE(M1_TYPE) || CHECK_TYPE(K7_TYPE) || CHECK_TYPE(K1P_TYPE); WDRCLR)
  {
    msp_usb_loop ();
    if (usb_on_off)          ;
    else                 LPM3;
    ConfWordCopy = getConfigW();
  }
  
  // включение
  ini_spec();
  KeyCode = SW2;
  key_process();    
    
  // main loop
  for(U16 a = 0;;WDRCLR)
  {        
    a =  Flags.measure_ready || Flags.systick || Flags.key || Flags.uart_receive || Flags.timeout;
    if( !a )
    {
      if(usb_on_off)                    ;       // MCLK + SMCLK
      else if(BT_CHECK || RS_CHECK) LPM0;       // SMCLK (FLL+) 
      else                          LPM3;       // ACLK only
    }
    
    // для борьбы с оптимизатором
    ConfWordCopy = getConfigW();
            
    // ВТ автовыкл
    a =  !BlueClock && BT_CHECK && BtActivePeriod && CHECK_TYPE(M7_TYPE);
    if( a )  
    {
      BT_OFF;
    }
    
    // в слип режим
    a = !SleepClock && !BlueClock && !ISSLEEP_MODE && (SleepPeriod || Flags.fromsleep);
    if( a )
    {
      Flags.fromsleep           = 0;
      KeyCode                   = SW2;
      key_process();
    }
    
    // запись в выкл режиме
    a = ISSLEEP_MODE && (Time2Write == PRE_SAVE_TIME) && (ConfWordCopy & SLEEP_WR) && !( CHECK_TYPE(M4_TYPE) && !(ConfWordCopy & RADIO_ON) ) 
                                                                       && !( CHECK_TYPE(M4L_TYPE) && !(ConfWordCopy & RADIO_ON) );
    if( a )
    {            
      Flags.fromsleep           = 1;
      KeyCode                   = SW2;
      key_process();
      SleepClock                = PRE_SAVE_TIME + 2;
    }

    // USB
    msp_usb_loop ();
    // keys
    if(Flags.key)               key_process();     
    // uart прием
    if(Flags.uart_receive)           
    {
      switch( ConfWordCopy & DEVTYPE_MASK )
      {                  
        case M3_TYPE:   slave_uart_process(); 
                        break;       
                        
        case M7_TYPE:   bt_process();
                        break;
      }      
    }
    // 868
    if(Flags.timeout) 
    {
      switch( ConfWordCopy & DEVTYPE_MASK )
      { 
        case M4L_TYPE:  radio868_process();
                        break;
      }
    }
    // ADC
    if(Flags.measure_ready)     measure_process_7m();    
    // 1 сек процессы    
    if(Flags.systick)            
    {
      Flags.systick    = 0;
      Flags.lcd_update = 1;      
            
      // проверка на валидность времени
      if( Time < ValidTime )    Errors  |=  TIME_ERROR; 
      else                      Errors  &= ~TIME_ERROR; 
      
      if( CHECK_TYPE(TYPE_NODEF))  SOFT_RESET;
      
      switch( ConfWordCopy & DEVTYPE_MASK )
      {
        case M6_TYPE:   sd_process();
                        break;                          
                        
        case M4_TYPE:   radio433_process();
                        break;                        
                        
        case M4L_TYPE:  radio868_process();
                        break;                                                
      }
      if( !Time2Write ) statistic_process();
      tresholds_process();
    }
    
    // ЖКИ
    if(Flags.lcd_update)      lcd_process();   
  } // for        
}
