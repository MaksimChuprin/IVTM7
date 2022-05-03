#include  "define.h"

void  lcd_process(void)
{     
    S16         p;
    
    Flags.lcd_update = 0;    
        
    if(Errors & INFOMEM_ERROR) 
    { 
      lcd_clear();     
      printf("E-Fh");
      Selector =  OFF_MODE;
      return;    
    }    
         
    switch(Selector)
    {
       case  HPR_MODE:   
                      if( Errors & (PR_ERROR + TYPE_ERROR) ) goto err_out;
                      if(Errors & HS_ERROR) printf("----");
                      else                  printf("%#5.1f", hum_calc_7k(Selector));
                      CLR_ALL_SYM;
                      if(Errors & H1_ERROR) LIGHT_LOW_TR;
                      if(Errors & H2_ERROR) LIGHT_UP_TR;
                      LIGHT_PR;
                      break;
                      
       case  HGM_MODE:
                      if( Errors & (PR_ERROR + TYPE_ERROR) ) goto err_out;
                      if(Errors & HS_ERROR) printf("----");
                      else                  printf("%#5.2f", hum_calc_7k(Selector) );
                      CLR_ALL_SYM;
                      if(Errors & H1_ERROR) LIGHT_LOW_TR;
                      if(Errors & H2_ERROR) LIGHT_UP_TR;
                      LIGHT_GM3;
                      break;                      
                      
       case  HMGM_MODE:                                              
                      if( Errors & (PR_ERROR + TYPE_ERROR) ) goto err_out;
                      if(Errors & HS_ERROR) printf("----");
                      else                  printf("%#5.0f", hum_calc_7k(Selector) );
                      CLR_ALL_SYM;
                      if(Errors & H1_ERROR) LIGHT_LOW_TR;
                      if(Errors & H2_ERROR) LIGHT_UP_TR;
                      LIGHT_GM3; LIGHT_M;
                      break;                      
                      
       case  HTR_MODE:                                              
                      if( Errors & (PR_ERROR + TYPE_ERROR) ) goto err_out;
                      if(Errors & HS_ERROR) printf("----");
                      else                  printf("%#5.1f", hum_calc_7k(Selector) );
                      CLR_ALL_SYM;
                      if(Errors & H1_ERROR) LIGHT_LOW_TR;
                      if(Errors & H2_ERROR) LIGHT_UP_TR;
                      LIGHT_DEG; LIGHT_P; LIGHT_T;
                      break;                      
                      
       case  HPPM_MODE:                                              
                      if( Errors & (PR_ERROR + TYPE_ERROR) ) goto err_out;
                      if(Errors & HS_ERROR) printf("----");
                      else                  printf("%#5.0f", hum_calc_7k(Selector) );
                      CLR_ALL_SYM;
                      if(Errors & H1_ERROR) LIGHT_LOW_TR;
                      if(Errors & H2_ERROR) LIGHT_UP_TR;
                      LIGHT_PM; LIGHT_P; 
                      break;                      
                                            
       case  HTVT_MODE:  
                      if( Errors & (PR_ERROR + TYPE_ERROR) ) goto err_out;
                      if( Errors & (TS_ERROR + HS_ERROR)) printf("----");
                      else                   printf("%#5.1f", hum_calc_7k(Selector));
                      CLR_ALL_SYM;                      
                      LIGHT_DEG; LIGHT_M;
                      break;                      
                      
       case  THC1_MODE:  
                      if( Errors & (PR_ERROR + TYPE_ERROR) ) goto err_out;
                      if(GenClock1)     { printf("tHC1"); CLR_ALL_SYM; break; }
                      
                      if( Errors & (TS2_ERROR  + TS_ERROR + HS_ERROR))  printf("----");
                      else                                              printf("%#5.1f", hum_calc_7k(Selector));
                      CLR_ALL_SYM;                      
                      LIGHT_DEG; LIGHT_T;
                      break;
                      
       case  THC2_MODE:  
                      if( Errors & (PR_ERROR + TYPE_ERROR) ) goto err_out;
                      if(GenClock1)     { printf("tHC2"); CLR_ALL_SYM; break; }
                      
                      if( Errors & (TS2_ERROR  + TS_ERROR + HS_ERROR))  printf("----");
                      else                                              printf("%#5.1f", hum_calc_7k(Selector));
                      CLR_ALL_SYM;                      
                      LIGHT_DEG; LIGHT_T;
                      break;                      
                                     
       case  P_MODE:  
                      if( Errors & (PR_ERROR + TYPE_ERROR) ) goto err_out;
                      if(Errors & PRESS_ERROR) printf("----");
                      else 
                      {
                        if (Press_mode == MMHG ) printf("%#5.1f", Pressure);
                        else                     printf("%#5.0f", Pressure * 1.3332);
                      }
                      CLR_ALL_SYM;
                      LIGHT_P;
                      break;
                      
       case  T_MODE:  
                      if( Errors & (PR_ERROR + TYPE_ERROR) ) goto err_out;
                      if(Errors & TS_ERROR) printf("----");
                      else                  printf("%#5.1f", Tempr);
                      CLR_ALL_SYM;
                      if(Errors & T1_ERROR) LIGHT_LOW_TR;
                      if(Errors & T2_ERROR) LIGHT_UP_TR;
                      LIGHT_DEG;
                      break;                      
                      
       case START_MODE: 
                      CLR_ALL_SYM;
                      printf("%s", OfficialVersion);
                      if(GenClock1)     break;
                      Selector = SaveSel;
                      break;

       case BAT_MODE:                       
                      printf("P%3d", PowerShow);
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
                      printf("P%3d", p);
                      CLR_ALL_SYM;
                      LIGHT_PR;
                      if(!usb_on_off) Selector = OFF_MODE;
                      break;
                      
err_out:                      
                      switch ( Errors & (PR_ERROR + TYPE_ERROR) )
                      {
                        case TYPE_ERROR:        printf("E-tp"); break;
                        default:                printf("E-Ln"); 
                      }
                      Time2Write = PRE_SAVE_TIME; 
                      break;
                      
       default:       Hum_mode = Selector = HPR_MODE;
    }

    // мигание значков и проч
    if( (PowerShow < 10) && !ISCHARGE_MODE ) LIGHT_POWER;
    if(Time & 1)
    {
       CLR_POWER; 
       CLR_LOW_TR; 
       CLR_UP_TR;
    } 
    if( Errors & PW_ERROR ) LIGHT_POWER;
}
