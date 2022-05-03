#include  "define.h"

void  lcd_process(void)
{     
    U8          minus = 0;
    S16         p;
    F32         f;
    
    Flags.lcd_update = 0; 
        
    if(Errors & INFOMEM_ERROR) 
    { 
      lcd_clear();     
      printf("Er-F ");
      Selector = OFF_MODE;
      return;    
    }    
         
    switch(Selector)
    {
       case  HPR_MODE + T_MODE:                                              
                      if(Errors & TS_ERROR) printf("---");
                      else                  
                      {
                        if( Tempr > 99.9 ) f = 99.9;
                        else if( Tempr <= -9.95 ) { f = -Tempr; minus = 1; }
                        else               f = Tempr;
                        printf("%#4.1f", f);
                      }
                      LIGHT_DEG;
                      if(Errors & HS_ERROR) printf("--");
                      else                  
                      {
                        f = Humidy;
                        if ( f > 99) { f = 99; p = 0; }
                        else if ( f >= 9.95) { p = 0; }
                        else p = 1;
                        printf("%#3.*f", p, f );
                      }
                      CLR_ALL_SYM;
                      if( minus ) LIGHT_MINUS;
                      LIGHT_PR; LIGHT_DEG;
                      break;
                      
       case  HPR_MODE + P_MODE:      
                      if(Errors & PRESS_ERROR) printf("---");
                      else
                      {
                        if(!Flags.PressGPa) printf("%#4.0f", Pressure);
                        else 
                        {
                          printf ("%#6.1f", Pressure*1.3332);
                          CLR_ALL_SYM;
                          break;
                        }
                      }
                      if(Errors & HS_ERROR) printf("--");
                      else                  
                      {
                        f = Humidy;
                        if ( f > 99) { f = 99; p = 0; }
                        else if ( f >= 9.95) { p = 0; }
                        else p = 1;
                        printf("%#3.*f", p, f );
                      }
                      CLR_ALL_SYM;
                      LIGHT_PR;
                      break;
                      
       case  HGM_MODE + T_MODE:                                              
                      if(Errors & TS_ERROR) printf("---");
                      else                  
                      {
                        if( Tempr > 99.9 ) f = 99.9;
                        else if( Tempr <= -9.95 ) { f = -Tempr; minus = 1; }
                        else               f = Tempr;
                        printf("%#4.1f", f);
                      }                      
                      LIGHT_DEG;
                      if(Errors & HS_ERROR) printf("--");
                      else                  
                      {
                        f = hum_calc_7m(Selector - T_MODE);
                        if ( f > 99) { f = 99; p = 0; }
                        else if ( f >= 9.95) { p = 0; }
                        else p = 1;
                        printf("%#3.*f", p, f );
                      }
                      CLR_ALL_SYM;
                      if( minus ) LIGHT_MINUS;
                      LIGHT_GM3; LIGHT_DEG;
                      break;                    
                      
       case  HGM_MODE + P_MODE:    
                      if(Errors & PRESS_ERROR) printf("---");
                      else
                      {                        
                        if(!Flags.PressGPa) printf("%#4.0f", Pressure);
                        else 
                        {
                          printf ("%#6.1f", Pressure*1.3332);
                          CLR_ALL_SYM;
                          break;
                        }
                      }
                      if(Errors & HS_ERROR) printf("--");
                      else                  
                      {
                        f = hum_calc_7m(Selector - P_MODE);
                        if ( f > 99) { f = 99; p = 0; }
                        else if ( f >= 9.95) { p = 0; }
                        else p = 1;
                        printf("%#3.*f", p, f );
                      }
                      CLR_ALL_SYM;
                      LIGHT_GM3;
                      break;                                                                                                                       
                      
       case START_MODE: 
                      CLR_ALL_SYM;
                      printf("%s", OfficialVersion);
                      if(GenClock1)     break;
                      switch( ConfWordCopy & DEVTYPE_MASK )
                      {
                        case M1_TYPE:
                        case M2_TYPE:
                        case M3_TYPE:   
                        case M7_TYPE:   Selector = SaveSel;
                                        break;
                                                
                        case M4_TYPE:                                                             
                        case M4L_TYPE:  GenClock1 = 2;
                                        Selector  = RT_MODE;
                                        break;
                                          
                        case M6_TYPE:   GenClock1 = 2;
                                        Selector  = SD_MODE;
                                        break;
                                            
                      }                        
                      break;

       case BAT_MODE:                       
                      printf(" P%3d", PowerShow);
                      CLR_ALL_SYM;
                      LIGHT_PR;
                      if(GenClock1)     break;
                      Selector = OFF_MODE;
                      break;
                      
       case OFF_MODE: lcd_clear(); 
                      if(!usb_on_off) break;
                      Selector = CHARGE_MODE;                      
                      
       case CHARGE_MODE:                       
                      CALCPOW_PR(p, Power, 4.4, 3.3);
                      printf(" P%3d", p);
                      CLR_ALL_SYM;
                      LIGHT_PR;
                      if(!usb_on_off) Selector = OFF_MODE;
                      break;
                      
       case SD_MODE:  if( SD_Flags.sd_InsertedAndValid )        printf(" InSd");
                      else if( !SD_Flags.sd_inserted )          printf("OutSd");
                      else                                      printf("ErrSd");
                      if(GenClock1)     break;
                      Selector = SaveSel;
                      break;
                      
       case RT_MODE:  if( Errors & RT_ERROR )                   printf("Errrt");
                      else  if( !(ConfWordCopy & RADIO_ON) )    printf("Offrt");
                      else                                      printf("bnd%2d", UnitBand);
                      
                      if(GenClock1)     break;
                      Selector = SaveSel;
                      break;                      
                      
       default:       Hum_mode = HPR_MODE; SaveSel = Selector = HPR_MODE + T_MODE;
    }

    // мигание значков и проч
    if( (PowerShow < 10) && !ISCHARGE_MODE )    LIGHT_POWER;
    if( Time & 1 )                              CLR_POWER;
    if( Errors & PW_ERROR )                     LIGHT_POWER;
}
