#include  "define.h"

#define NO_KEY            NONE
#define D_KEY             SW1
#define ENTR              SW2
#define MENU_ITEM_LEN     5

#define SAVE_VAR(var, val) WriteFlashString((unsigned int)&(var), (pU8)&(val), sizeof((val)))

// predeclaration
Int8S                     key(Int8U wait);
Int8U                     show_list(list_sruct  list, pInt8S  p);
Int16S                    set_int(Int16S i, Int16S down_range, Int16S up_range);
Flo32                     set_float(Flo32 f, Flo32 down_range, Flo32 up_range, Int8U a);

// global
static signed char        delta = 1;

const list_sruct   menu_off_on  =  { 1, "oFF , on " };
const list_sruct   menu_main    =  { 5, "ÏOP ,Snd ,PerS,SLSt,SLEP,out " };
const list_sruct   menu_trsh    =  { 4, "HÏ t,8Ï t,HÏ h,8Ï h,out " };


// #pragma optimize=none
volatile U8        wp_m;

void  prog_serv(void)
{
  float          f;
  unsigned int   d;
  signed char    k, m= 0, m1, m2;
    
  SOUND_OFF;
  delta = 1;
  
  for(;;)
  {
    lcd_clear();
    k = show_list(menu_main,&m);
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
                            f = set_float(TrshVal[m1], -20., 60., 0); SAVE_VAR(TrshVal[m1], f);
                            break;
                    case 2:
                    case 3: LIGHT_PR;
                            f = set_float(TrshVal[m1], 1., 99., 0); SAVE_VAR(TrshVal[m1], f);
                            break;
                            
                    case 4: m2 = 0;
                            break;
                  }                  
                 }
                 break;
                 
     case  1:    ConfWordCopy = getConfigW();
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
                 
     case  3:    ConfWordCopy = getConfigW();
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
                  
       case  5:  lcd_clear(); return;
     }                      
  } // for(;;)
}

Flo32 set_float(Flo32 f, Flo32 down_range, Flo32 up_range, Int8U a)
{
  signed char  k;
  Flo32        d;
          
  switch(a)
  {
    case 0: d= 1.;    break;
    case 1: d= 0.1;   break;
    case 2: d= 0.01;  break;
    default: d= 0.001;
  }
  
  for(;;)
  {       
    printf("%#5.1f", f);      
    k = key(30);
    
    if(k==NO_KEY) { delta= 1; return(f); }
    if(k==ENTR)   { delta= 1; return(f); }
    else 
    {                       
      f= f + k * d;
      if (f<down_range) f= up_range;
      if (f>up_range)   f= down_range;
    }     
  }                               
}                  

Int16S set_int(Int16S i, Int16S down_range, Int16S up_range)
{
     signed char  k;
                  
     for(;;)
     {
       printf("%4d",i);
       k =key(45);
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
    else                          LPM3;       // ACLK only 
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

