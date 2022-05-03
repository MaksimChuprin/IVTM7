#include  "define.h"

#define NO_KEY            NONE
#define D_KEY             SW1
#define ENTR              SW2
#define MENU_ITEM_LEN     6

#define SAVE_VAR(var, val) WriteFlashString((unsigned int)&(var), (pU8)&(val), sizeof((val)))

// predeclaration
Int8S                     key(Int8U wait);
Int8U                     show_list(list_sruct  list, pInt8S  p);
Int16S                    set_int(Int16S i, Int16S down_range, Int16S up_range);
Flo32                     set_float(Flo32 f, Flo32 down_range, Flo32 up_range, Int8U a);

static signed char        delta;

const list_sruct   menu_off_on   =  { 1, "oFF  , on  " };
const list_sruct   menu_pressure =  { 1, "Pt St, rÏA " };
const list_sruct   menu_card     =  { 1, "RES  ,Cont " };
const list_sruct   menu_trsh     =  { 4, "HÏ t ,8Ï t ,HÏ h ,8Ï h ,out  " };
const list_sruct   menu_speed    =  { 5, " 4800, 9600,19200,38400,57800,11520" };
const list_sruct   menu_main_m2  =  { 6, "ÏOP  ,Snd  ,Per S,SLPSt,SLP  ,PreS ,out  " };
const list_sruct   menu_main_m3  =  { 8, "ÏOP  ,Snd  ,Per S,SLPSt,SLP  ,Adr  ,SPd  ,PreS ,out  " }; 
const list_sruct   menu_main_m4  =  { 6, "rAdIo,bAnd ,Adr  ,Per r,SLP  ,PreS ,out  " };
const list_sruct   menu_main_m4L =  { 8, "rAdIo,bAnd ,Adr  ,SPd  ,Por  ,Per r,SLP  ,PreS ,out  " };
const list_sruct   menu_main_m6  =  { 6, "ÏOP  ,Snd  ,Per S,SLPSt,SLP  ,PreS ,out  " };  
const list_sruct   menu_main_m7  =  { 7, "ÏOP  ,Snd  ,Per S,SLPSt,SLP  ,Bt Pr,PreS ,out  " };

// #pragma optimize=none
volatile U8        wp_m;

void  prog_serv(void)
{
  float          f;
  unsigned int   d;
  signed char    k, m = 0, m1, m2;
  
    
  SOUND_OFF;
  delta = 1;
  
  switch( ConfWordCopy & DEVTYPE_MASK )
  {                
    case M2_TYPE:
/* #######################  M2 ######################### */        
  for(;;)
  {
     lcd_clear();     
     k = show_list(menu_main_m2, &m);
     if(k == NO_KEY)  { lcd_clear(); return;  }
     switch(m)
     {
       case  0:  for(m1 = 0, m2 = 1; m2; )
                 {
                  lcd_clear();
                  k = show_list(menu_trsh, &m1);
                  if(k == NO_KEY) break;                  
                  
                  switch(m1)
                  {
                    case 0:
                    case 1: LIGHT_DEG;
                            f= set_int(TrshVal[m1], -45, 60); SAVE_VAR(TrshVal[m1], f); 
                            break;
                    case 2:
                    case 3: LIGHT_PR;
                            f= set_int(TrshVal[m1], 1, 99); SAVE_VAR(TrshVal[m1], f);
                            break;
                    case 4: m2 = 0;
                  }                  
                 }  
                 break;
                 
       case  1:  ConfWordCopy = getConfigW();
                 m1 = (ConfWordCopy & SOUND_ON) ? 1 : 0;
                 k  = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 ConfWordCopy = (ConfWordCopy & ~SOUND_ON) | (m1 * SOUND_ON);
                 SAVE_VAR(ConfigWord, ConfWordCopy);
                 break;
                                  
       case  2:  wp_m = WritePeriod / 60;
                 d    = set_int(wp_m, 1, 240) * 60;
                 SAVE_VAR( WritePeriod, d );
                 Time2Write =  d;
                 break;
                 
       case  3:  ConfWordCopy = getConfigW();
                 m1 = (ConfWordCopy & SLEEP_WR) ? 1 : 0;
                 k= show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 ConfWordCopy = (ConfWordCopy & ~SLEEP_WR) | (m1 * SLEEP_WR);
                 SAVE_VAR(ConfigWord, ConfWordCopy);
                 break;                 
                 
       case  4:  wp_m = SleepPeriod / 60;
                 m1   = wp_m ? 1 : 0;
                 k    = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 d = m1 ? 600 : 0;
                 SAVE_VAR( SleepPeriod, d);
                 SleepClock = d;
                 break;                 
                  
       case  5:  m1   = Flags.PressGPa ? 1 : 0;
                 k    = show_list(menu_pressure, &m1);
                 if(k==NO_KEY)   break;
                 Flags.PressGPa = m1;
                 break;
                 
       case  6:  lcd_clear(); return;
     }                      
  } // for(;;)      
/* #######################  M2 ######################### */      
                break;                
                
    case M3_TYPE:                 
/* #######################  M3 ######################### */
  for(;;)
  {
     lcd_clear();
     k = show_list(menu_main_m3, &m);
     if(k == NO_KEY)  { lcd_clear(); return;  }
     switch(m)
     {
       case  0:  for(m1 = 0, m2 = 1; m2; )
                 {
                  lcd_clear();
                  k = show_list(menu_trsh, &m1);
                  if(k == NO_KEY) break;                  
                  
                  switch(m1)
                  {
                    case 0:
                    case 1: LIGHT_DEG;
                            f= set_int(TrshVal[m1], -45, 60); SAVE_VAR(TrshVal[m1], f);
                            break;
                    case 2:
                    case 3: LIGHT_PR;
                            f= set_int(TrshVal[m1], 1, 99); SAVE_VAR(TrshVal[m1], f);
                            break;
                    case 4: m2 = 0;
                  }                 
                 }  
                 break;
                 
       case  1:  ConfWordCopy = getConfigW();
                 m1 = (ConfWordCopy & SOUND_ON) ? 1 : 0;
                 k  = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 ConfWordCopy = (ConfWordCopy & ~SOUND_ON) | (m1 * SOUND_ON);
                 SAVE_VAR(ConfigWord, ConfWordCopy);
                 break;
                                  
       case  2:  wp_m = WritePeriod / 60;
                 d    = set_int(wp_m, 1, 240) * 60;
                 SAVE_VAR( WritePeriod, d );
                 Time2Write =  d;
                 break;
                 
       case  3:  ConfWordCopy = getConfigW();
                 m1 = (ConfWordCopy & SLEEP_WR) ? 1 : 0;
                 k= show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 ConfWordCopy = (ConfWordCopy & ~SLEEP_WR) | (m1 * SLEEP_WR);
                 SAVE_VAR(ConfigWord, ConfWordCopy);
                 break;                
                 
       case  4:  wp_m = SleepPeriod / 60;
                 m1   = wp_m ? 1 : 0;
                 k    = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 d = m1 ? 600 : 0;
                 SAVE_VAR( SleepPeriod, d);
                 SleepClock = d;
                 break;                 
                 
       case  5:  d    = set_int(NetAdr, 1, 255);
                 SAVE_VAR( NetAdr, d);
                 break;                 
                 
       case  6:  ConfWordCopy = getConfigW();
                 m1 = (ConfWordCopy & SPEED_MASK) >> 4;
                 k  = show_list(menu_speed, &m1);
                 if(k == NO_KEY)   break;
                 ConfWordCopy = (ConfWordCopy & ~SPEED_MASK) | ((U16)m1 << 4);
                 SAVE_VAR(ConfigWord, ConfWordCopy);
                 choose_uart_speed(ConfWordCopy & SPEED_MASK);
                 break;
                 
       case  7:  m1   = Flags.PressGPa ? 1 : 0;
                 k    = show_list(menu_pressure, &m1);
                 if(k==NO_KEY)   break;
                 Flags.PressGPa = m1;
                 break;                 
                  
       case  8:  lcd_clear(); return;
     }                      
  } // for(;;)       
/* #######################  M3 ######################### */             
                break; 
  
    case M4_TYPE:
/* #######################  M4 ######################### */        
  for(;;)
  {
     lcd_clear();     
     k = show_list(menu_main_m4, &m);
     if(k == NO_KEY)  { lcd_clear(); return;  }
     switch(m)
     {
       case  0:  ConfWordCopy = getConfigW();
                 m1 = (ConfWordCopy & RADIO_ON) ? 1 : 0;
                 k  = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 ConfWordCopy = (ConfWordCopy & ~RADIO_ON) | (m1 * RADIO_ON);
                 ConfWordCopy = (ConfWordCopy & ~SLEEP_WR) | (m1 * SLEEP_WR);
                 SAVE_VAR(ConfigWord, ConfWordCopy);
                 break;
                 
       case  1:  wp_m    = UnitBand;
                 wp_m    = set_int(wp_m, 1, 9);
                 SAVE_VAR( UnitBand, wp_m );
                 break;
                 
       case  2:  d    = set_int(UnitAddress, 1, 255);
                 SAVE_VAR( UnitAddress, d);
                 break;                 
                                                   
       case  3:  wp_m = WritePeriod / 60;
                 d    = set_int(wp_m, 1, 240) * 60;
                 SAVE_VAR( WritePeriod, d );
                 Time2Write =  d;
                 break;
                                                   
       case  4:  wp_m = SleepPeriod / 60;
                 m1   = wp_m ? 1 : 0;
                 k    = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 d    = m1 ? 600 : 0;
                 SAVE_VAR( SleepPeriod, d);
                 SleepClock = d;
                 break;                 

       case  5:  m1   = Flags.PressGPa ? 1 : 0;
                 k    = show_list(menu_pressure, &m1);
                 if(k==NO_KEY)   break;
                 Flags.PressGPa = m1;
                 break;
                 
       case  6:  lcd_clear(); return;
     }                      
  } // for(;;)      
/* #######################  M4 ######################### */      
                break;  
  
      case M4L_TYPE:
/* #######################  M4L ######################### */        
  for(;;)
  {
     lcd_clear();     
     k = show_list(menu_main_m4L, &m);
     if(k == NO_KEY)  { lcd_clear(); return;  }
     switch(m)
     {
       case  0:  ConfWordCopy = getConfigW();
                 m1 = (ConfWordCopy & RADIO_ON) ? 1 : 0;
                 k  = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 ConfWordCopy = (ConfWordCopy & ~RADIO_ON) | (m1 * RADIO_ON);
                 ConfWordCopy = (ConfWordCopy & ~SLEEP_WR) | (m1 * SLEEP_WR);
                 SAVE_VAR(ConfigWord, ConfWordCopy);
                 break;
                 
       case  1:  wp_m    = UnitBand;
                 wp_m    = set_int(wp_m, 1, 15);
                 SAVE_VAR( UnitBand, wp_m );
                 break;
                 
       case  2:  d    = set_int(UnitAddress, 1, 255);
                 SAVE_VAR( UnitAddress, d);
                 saveCounter = 0;
                 break;                 

       case  3:  wp_m = set_int(RadioSpeed, 1, 4);
                 SAVE_VAR( RadioSpeed, wp_m);
                 break;                 
                 
       case  4:  wp_m = set_int(RadioPower, 2, 17);
                 SAVE_VAR( RadioPower, wp_m);
                 break;  
                                                                                    
       case  5:  wp_m = WritePeriod / 60;
                 d    = set_int(wp_m, 1, 240) * 60;
                 SAVE_VAR( WritePeriod, d );
                 Time2Write =  d;
                 break;                
                 
       case  6:  wp_m = SleepPeriod / 60;
                 m1   = wp_m ? 1 : 0;
                 k    = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 d    = m1 ? 600 : 0;
                 SAVE_VAR( SleepPeriod, d);
                 SleepClock = d;
                 break;                 

       case  7:  m1   = Flags.PressGPa ? 1 : 0;
                 k    = show_list(menu_pressure, &m1);
                 if(k==NO_KEY)   break;
                 Flags.PressGPa = m1;
                 break;
                 
       case  8:  lcd_clear(); return;
     }                      
  } // for(;;)      
/* #######################  M4L ######################### */      
                break; 
     
  case M6_TYPE:          
/* #######################  M6 ######################### */ 
  for(;;)
  {
     lcd_clear();
     k = show_list(menu_main_m6, &m);
     if(k == NO_KEY)  { lcd_clear(); return;  }
     switch(m)
     {
       case  0:  for(m1 = 0, m2 = 1; m2; )
                 {
                  lcd_clear();
                  k = show_list(menu_trsh, &m1);
                  if(k == NO_KEY) break;                  
                  
                  switch(m1)
                  {
                    case 0:
                    case 1: LIGHT_DEG;
                            f= set_int(TrshVal[m1], -45, 60); SAVE_VAR(TrshVal[m1], f);
                            break;
                    case 2:
                    case 3: LIGHT_PR;
                            f= set_int(TrshVal[m1], 1, 99); SAVE_VAR(TrshVal[m1], f);
                            break;
                    case 4: m2 = 0;
                  }                 
                 }  
                 break;
                 
       case  1:  ConfWordCopy = getConfigW();
                 m1 = (ConfWordCopy & SOUND_ON) ? 1 : 0;
                 k  = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 ConfWordCopy = (ConfWordCopy & ~SOUND_ON) | (m1 * SOUND_ON);
                 SAVE_VAR(ConfigWord, ConfWordCopy);
                 break;
                                  
       case  2:  wp_m = WritePeriod / 60;
                 d    = set_int(wp_m, 1, 240) * 60;
                 SAVE_VAR( WritePeriod, d );
                 Time2Write =  d;
                 break;
                 
       case  3:  ConfWordCopy = getConfigW();
                 m1 = (ConfWordCopy & SLEEP_WR) ? 1 : 0;
                 k= show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 ConfWordCopy = (ConfWordCopy & ~SLEEP_WR) | (m1 * SLEEP_WR);
                 SAVE_VAR(ConfigWord, ConfWordCopy);
                 break;                 
                 
       case  4:  wp_m = SleepPeriod / 60;
                 m1   = wp_m ? 1 : 0;
                 k    = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 d    = m1 ? 600 : 0;
                 SAVE_VAR( SleepPeriod, d);
                 SleepClock = d;
                 break;                                  
       
       case  5:  m1   = Flags.PressGPa ? 1 : 0;
                 k    = show_list(menu_pressure, &m1);
                 if(k==NO_KEY)   break;
                 Flags.PressGPa = m1;
                 break;
                 
       case  6:  lcd_clear(); return;
     }                      
  } // for(;;)     
/* #######################  M6 ######################### */               
                break;
                
  case M7_TYPE: 
/* #######################  M7 ######################### */           
  for(;;)
  {
     lcd_clear();
     k = show_list(menu_main_m7, &m);
     if(k == NO_KEY)  { lcd_clear(); return;  }
     switch(m)
     {
       case  0:  for(m1 = 0, m2 = 1; m2; )
                 {
                  lcd_clear();
                  k = show_list(menu_trsh, &m1);
                  if(k == NO_KEY) break;                  
                  
                  switch(m1)
                  {
                    case 0:
                    case 1: LIGHT_DEG;
                            f= set_int(TrshVal[m1], -45, 60); SAVE_VAR(TrshVal[m1], f);
                            break;
                    case 2:
                    case 3: LIGHT_PR;
                            f= set_int(TrshVal[m1], 1, 99); SAVE_VAR(TrshVal[m1], f);
                            break;
                    case 4: m2 = 0;
                  }                  
                 }  
                 break;
                 
       case  1:  ConfWordCopy = getConfigW();
                 m1 = (ConfWordCopy & SOUND_ON) ? 1 : 0;
                 k  = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 ConfWordCopy = (ConfWordCopy & ~SOUND_ON) | (m1 * SOUND_ON);
                 SAVE_VAR(ConfigWord, ConfWordCopy);
                 break;
                                  
       case  2:  wp_m = WritePeriod / 60;
                 d    = set_int(wp_m, 1, 240) * 60;
                 SAVE_VAR( WritePeriod, d );
                 Time2Write =  d;
                 break;
                 
       case  3:  ConfWordCopy = getConfigW();
                 m1 = (ConfWordCopy & SLEEP_WR) ? 1 : 0;
                 k= show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 ConfWordCopy = (ConfWordCopy & ~SLEEP_WR) | (m1 * SLEEP_WR);
                 SAVE_VAR(ConfigWord, ConfWordCopy);
                 break;                  
                 
       case  4:  wp_m = SleepPeriod / 60;
                 m1   = wp_m ? 1 : 0;
                 k    = show_list(menu_off_on, &m1);
                 if(k==NO_KEY)   break;
                 d    = m1 ? 600 : 0;
                 SAVE_VAR( SleepPeriod, d);
                 SleepClock = d;
                 break;                 
                 
       case  5:  wp_m = BtActivePeriod / 60;
                 d    = set_int(wp_m, 0, 60) * 60;
                 SAVE_VAR( BtActivePeriod, d);
                 SleepClock = d;
                 break;                 

       case  6:  m1   = Flags.PressGPa ? 1 : 0;
                 k    = show_list(menu_pressure, &m1);
                 if(k==NO_KEY)   break;
                 Flags.PressGPa = m1;
                 break;
                 
       case  7:  lcd_clear(); return;
     }                      
  } // for(;;)     
/* #######################  M7 ######################### */               
                break;
  }
  lcd_clear();  
}  
  
Int16S set_int(Int16S i, Int16S down_range, Int16S up_range)
{
     signed char  k;
                  
     for(;;)
     {
       printf("%3d  ",i);
       
       k = key(45);
       if((k==NO_KEY)||(k==ENTR)) { delta= 1; return(i); }
          else { 
                      i= i + k;
                      if (i<down_range) i= up_range;
                      if (i>up_range) i= down_range;
                }
     }                    
}

Int8S key(U8 wait)
{
  Flags.key = 0;
  GenClock1 = wait;
  KeyCode   = NO_KEY;
  
  for(;GenClock1;WDRCLR)
  {     
    if(Flags.key)               { Flags.key = 0; break; }
    msp_usb_loop ();
    if(Flags.measure_ready)     measure_process_7m();
    if(Flags.systick)           
    { 
      Flags.systick = 0; 
    }
    
    if(usb_on_off)                    ;       // MCLK + SMCLK
    else if(BT_CHECK || RS_CHECK) LPM0;       // SMCLK (FLL+) 
    else                          LPM3; 
  }
    
  switch(KeyCode)
  {
    case SW1:       return delta;
    case LONG_SW2:
    case SW2:       return  ENTR;
    case LONG_SW1:  delta = -delta;  
                    return delta;
  }
  
  return NO_KEY;
}

Int8U show_list(list_sruct  list, pInt8S  p)
{
  U8    *ps, s[8];
  S8     k;
  
  for(s[MENU_ITEM_LEN-1] = 0;;)
  {
    if(*p > list.up) *p = 0;
    else if(*p < 0)  *p = list.up;
    ps = (pU8)((U16)list.strings_p + (*p * MENU_ITEM_LEN));
    memcpy(s, ps, MENU_ITEM_LEN-1); 
  
    printf("%s", s);
    k = key(30);
    if(k == NO_KEY)     return NO_KEY;
    else if(k == ENTR)  return ENTR;
    (*p) += k;
  }
}
