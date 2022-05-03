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
                      
                      case HPR_MODE + T_MODE:
                      case HGM_MODE + T_MODE:    
                                        if(ConfWordCopy & PRES) Selector = P_MODE + Hum_mode; 
                                        break;
                                        
                      case HPR_MODE + P_MODE:
                      case HGM_MODE + P_MODE:    
                                        Selector = T_MODE + Hum_mode; 
                                        break;      
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
                                        GenClock1 = 2;
                                        switch( ConfWordCopy & DEVTYPE_MASK )
                                        {
                                          case M1_TYPE:
                                          case M2_TYPE:
                                          case M3_TYPE:
                                          case M6_TYPE:   if( !Flags.fromsleep ) Time2Write = PRE_SAVE_TIME; 
                                                          break;
                                                          
                                          case M4_TYPE:   Errors  &= ~(RT_ERROR);
                                                          Errors  |= (dp1203_ini() ? RT_ERROR : 0);
                                                          dp1203_sleep();
                                                          if( !Flags.fromsleep ) Time2Write = PRE_SAVE_TIME;
                                                          break;                                 
                                                          
                                          case M4L_TYPE:  Errors  &= ~(RT_ERROR);
                                                          Errors  |= (SX1276_Init() ? RT_ERROR : 0);
                                                          SX1276_Sleep();
                                                          if( !Flags.fromsleep ) Time2Write = PRE_SAVE_TIME;
                                                          break;                                                          
                                            
                                          case M7_TYPE:   if( !Flags.fromsleep ) 
                                                          { 
                                                            BT_ON; 
                                                            BlueClock  = BtActivePeriod; 
                                                            Time2Write = PRE_SAVE_TIME; 
                                                          }                                                          
                                                          break;                                         
                                        }
                                        break;
                                  
                    default:          
                                        if( !ISSTART_MODE && !ISSD_MODE && !ISRT_MODE )  SaveSel = Selector;
                                        GenClock1        = 0;                              
                                        Selector         = OFF_MODE;       
                                        switch( ConfWordCopy & DEVTYPE_MASK )
                                        {
                                          case M1_TYPE:
                                          case M2_TYPE:
                                          case M3_TYPE:
                                          case M6_TYPE:                                                             
                                          case M4_TYPE:                                                             
                                          case M4L_TYPE:  
                                                          break;                                                          
                                            
                                          case M7_TYPE:   BT_OFF;                                                          
                                                          break;                                         
                                        }                              
                                        
                                        if( Time2Write < PRE_SAVE_TIME + 5 ) Time2Write = PRE_SAVE_TIME + 5;
                                        break;      
                    }
                    break;

    case LONG_SW1:   switch(Selector)
                    {
                      case START_MODE:
                                        break;
                                        
                      case OFF_MODE:
                      case CHARGE_MODE:       prog_serv(); 
                                              if( Time2Write < PRE_SAVE_TIME + 5 ) Time2Write = PRE_SAVE_TIME + 5;
                                              break;
                      
                      case HPR_MODE + T_MODE:
                      case HPR_MODE + P_MODE:
                                              Hum_mode = HGM_MODE; Selector = Selector - HPR_MODE + Hum_mode; break;
                                        
                      case HGM_MODE + T_MODE:
                      case HGM_MODE + P_MODE: 
                                              Hum_mode = HPR_MODE; Selector = Selector - HGM_MODE + Hum_mode; break;
                    }                                                              
                    break;                                                                                                    
  }
  
  Flags.key         = 0;
  Flags.lcd_update  = 1;
  SleepClock        = SleepPeriod;         // перезапуск таймера сна
}