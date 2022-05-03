#include  "define.h"

void main(void)
{ 
  // общая инициализация
  ini_common();
  CheckInfoFlash();
  ConfWordCopy = getConfigW();      
  __enable_interrupt();  
  msp_usb_init ();
    
  // проверка конфигурации   
  if( !CHECK_TYPE(M1_TYPE) ) printf("CFnO");  
  for(; !CHECK_TYPE(M1_TYPE); WDRCLR)
  {
    msp_usb_loop ();
    if (usb_on_off == 1) ;
    else LPM3;
    ConfWordCopy = getConfigW();
  }  
  
  // включение
  ini_spec();
  KeyCode = SW2;
  key_process();  
    
  // main loop
  for(U16 a = 0; ; WDRCLR)
  {
    a =  Flags.measure_ready || Flags.systick || Flags.key;
    if( !a )
    {
      if(usb_on_off == 1)               ;       // MCLK + SMCLK
      else                          LPM3;       // ACLK only
    }
    
    // для борьбы с оптимизатором
    ConfWordCopy = getConfigW();    
    
    // в слип режим
    if( !SleepClock && (SleepPeriod || Flags.fromsleep) && !ISSLEEP_MODE )
    {          
      Flags.fromsleep           = 0;
      KeyCode                   = SW2;
      key_process();
    }  
            
    // запись в выкл режиме
    if( ISSLEEP_MODE && (Time2Write == PRE_SAVE_TIME) && (ConfWordCopy & SLEEP_WR) )
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
    // ADC
    if(Flags.measure_ready)     measure_process_7m();    
    // 1 сек процессы    
    if(Flags.systick)            
    {
      Flags.lcd_update = 1;      
      Flags.systick    = 0;
      
      // проверка на валидность времени
      if( Time < ValidTime )    Errors  |=  TIME_ERROR;
      else                      Errors  &= ~TIME_ERROR;      
      
      if( CHECK_TYPE(TYPE_NODEF))  SOFT_RESET;
      
      if( !Time2Write ) statistic_process(); 
      tresholds_process();          
    }    
    
    // ЖКИ
    if(Flags.lcd_update)        lcd_process();   
  } // for        
}

