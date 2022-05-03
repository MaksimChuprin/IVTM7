#include  "define.h"

void  key_process(void)
{    
  if(Errors & (INFOMEM_ERROR + PW_ERROR)) 
  { 
    Flags.key = 0; 
    return; 
  }
               
  switch(KeyCode)
  {
    case SW1:       switch(Selector)
                    {
                      case OFF_MODE:    Selector  = BAT_MODE; 
                                        GenClock1 = 2; 
                                        break;
                      
                      case START_MODE:
                      case CHARGE_MODE: break; 
                      
                      case HPR_MODE:
                      case HTR_MODE:
                      case HGM_MODE:    Selector = T_MODE; break;
                      
                      case T_MODE:      if(ConfWordCopy & PRES) Selector = P_MODE; 
                                        else                    Selector = Hum_mode; 
                                        break;
      
                      case P_MODE:      Selector = Hum_mode; break;
                    }
                    break;
                          
    case SW2:
    case LONG_SW2:
                    switch(Selector)
                    {
                      case BAT_MODE:
                      case OFF_MODE:
                      case CHARGE_MODE:
                                      Selector  = START_MODE; 
                                      GenClock1 = 3;
                                      if( !Flags.fromsleep ) Time2Write = PRE_SAVE_TIME;                                      
                                      break;
                                  
                      default:         
                                      if( !ISSTART_MODE ) SaveSel = Selector;
                                      GenClock1        = 0;                              
                                      Selector         = OFF_MODE;
                                      if( Time2Write < PRE_SAVE_TIME + 5 ) Time2Write = PRE_SAVE_TIME + 5;
                                      break;      
                    }
                    break;

    case LONG_SW1:   switch(Selector)
                    {
                      case T_MODE:
                      case START_MODE:
                                        break;
                                        
                      case OFF_MODE:
                      case CHARGE_MODE: prog_serv(); 
                                        if( Time2Write < PRE_SAVE_TIME + 5 ) Time2Write = PRE_SAVE_TIME + 5;
                                        break;
                      
                      case HPR_MODE:    Selector = Hum_mode = HTR_MODE; break;
                      case HTR_MODE:    Selector = Hum_mode = HGM_MODE; break;
                      case HGM_MODE:    Selector = Hum_mode = HPR_MODE; break;
                            
                      case P_MODE:      if (Press_mode == MMHG ) Press_mode = gPA;
                                        else                     Press_mode = MMHG;
                                        break;
                    }                                                              
                    break;                                                                                                    
  }
  
  Flags.key         = 0;
  Flags.lcd_update  = 1;
  SleepClock        = SleepPeriod;         // перезапуск таймера сна
}
