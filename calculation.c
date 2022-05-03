#include  "define.h"

F32 serj_func   (F32 x);
F32 calc        (F32 input, U8 polnum);
F32 e_calc      (F32 t);
F32 tcalc       (F32 (*calc)(F32), F32 e);
F32 ePLUS_calc  (F32 t);

// нормированна€ платина 1,385
const F32       Pt385_Polynoms[6] =  { -2.43880E+02, 2.30354E+02, 1.57958E+01, -3.05764E+00, 7.82640E-01, -5.02791E-02 };
// давление насыщенного пара дл€ t>0
const F32       HPTP_polynoms[6] = { 6.1115, 4.377191e-1, 1.475077e-2, 2.5602536e-4, 2.8791907e-6, 2.719325e-8 };

#if  ( defined (__7M1_PRA_MSD_USER) || defined (__7M2_PRA_MSD_USER) )

void LPS22_Write(U8 adr, U8 data)
{
  adr     &= 0x7f;
  
  LPS22_CS_LOW;  
  SHIFT_BYTE_SX1276(adr);
  SHIFT_BYTE_SX1276(data);
  LPS22_CS_HIGH;  
}

U8 LPS22_Read(U8 adr)
{    
  adr     |= 0x80; 
  
  LPS22_CS_LOW; 
  SHIFT_BYTE_SX1276(adr);  
  adr = 0; 
  SHIFT_BYTE_SX1276(adr); 
  LPS22_CS_HIGH;  
  
  return adr; 
}

void  measure_process_7m(void)
{     
  F32           f;
  pU16          p;
  U16           av;
  U8            i;
            
  Errors              &= ~(HS_ERROR + TS_ERROR + PRESS_ERROR);  
  if( ISNOMEAS_MODE ) { Flags.measure_ready  =  0; return; }
  
  // расчет напр€жени€ питани€
  Errors &= ~(PW_ERROR + LOPW_ERROR);
  f =  ADC12MEM8 * Ref15 * 3.128;
  if( fabs(Power - f) > 0.02 )          Power   = f;                   // колебани€ свыше 20 м¬ без фильтра
  else                                  Power  += 0.05 * (f - Power);  // фильтр  

  if( ISCHARGE_MODE ) 
  { 
    Flags.measure_ready = 0; 
    return; 
  }
      
  if(Power < 3.3)               
  {    
    KeyCode              = SW2;
    key_process();
    Errors              |= PW_ERROR;
    Flags.measure_ready  =  0;
    return;
  }
  
  // уровень птани€ без USB
  if(!usb_on_off)
    CALCPOW_PR(PowerShow, Power, 4.2, 3.3);
  if( PowerShow < 10 ) Errors |= LOPW_ERROR;
  
  // расчет температуры  !Flags.bad_tempr
  for(i = 0, av = 0, p = (pU16)ADC12MEM0_; i < 8; i++, p++) av += *p;
  f = 4095. / (av / 8) - 1.;
  f = 1. / f; 
  
  if( CHECK_FLOAT(f) )              Errors |= (TS_ERROR);
  else
  {
    if((f < .7) || (f > 1.7))       Errors |= (TS_ERROR);  
    else
    {
      if( fabs(Res - f) > 0.0038)   Res  = f;                  // колебани€ свыше 1, гр без фильтра
      else                          Res += 0.05 * (f - Res);   
        
      Ias_Par = calc(Res   , 0);
      Ras_Par = calc(Ias_Par, 1);
      f       = calc(Ras_Par, 2);
   
      if( CHECK_FLOAT(f) )          Errors |= (TS_ERROR);
      else
      {
        __disable_interrupt(); // защита от обмена 
        Tempr  =  f;
        if(Tempr > 155.)          { Errors |= TS_ERROR; Tempr = 155.; }
        else if(Tempr < -65.)     { Errors |= TS_ERROR; Tempr = -65.; }
        __enable_interrupt(); // защита от обмена 
      }
    }
  }
  
  // расчет влажности
  for(i = 0, av = 0, p = (pU16)ADC12MEM10_; i < 6; i++, p++) av += *p;
  
  f  =  ((F32)av * AdcGain / 6. + ADC_OFFSET) * Ref15;  
  if( fabs(HumidyParam - f) > 0.005)  HumidyParam  = f;                          // колебани€ свыше 5 м¬ без фильтра
  else                                HumidyParam += 0.05 * (f - HumidyParam);   // 
  
  f  = HumidyParam / (C0_H + Tempr * C1_H);
  f  = W1 * serj_func(W11 * f + B11) + W2 * serj_func(W12 * f + B12) +  B2;
  if( CHECK_FLOAT(f) )        Errors |= (HS_ERROR); 
  else 
  {
    __disable_interrupt(); // защита от обмена 
    Humidy = f;  
    if(Humidy > 110.)         { Errors |= HS_ERROR; Humidy= 99.9; }
    else if(Humidy > 99.9)    Humidy = 99.9;
    else if(Humidy < -10.)    { Errors |= HS_ERROR; Humidy= 0.1; }
    else if(Humidy <  0.1)    Humidy = 0.1;
    __enable_interrupt(); // защита от обмена
  }
  
  // расчет давлени€
  if(ConfWordCopy & PRES)
  {
    if(Flags.LPS22HB)
    {
      if( SD_Flags.sd_inserted ) 
      {         
        if((usb_on_off == 0) || (SD_Flags.sd_InsertedAndValid == 0)) { SD_ON; DELAY_USB_MS(5); }
      }
      
      U8  pxlsb   = LPS22_Read(LPS22_PRESS_OUT_XL);
      U8  plsb    = LPS22_Read(LPS22_PRESS_OUT_L);
      U8  pmsb    = LPS22_Read(LPS22_PRESS_OUT_H);      
      LPS22_Write(LPS22_CTRL_REG2, LPS22_ONE_SHORT + LPS22_I2C_DIS);
      
      if((usb_on_off == 0) || (SD_Flags.sd_InsertedAndValid == 0)) SD_OFF;
        
      PressureParam = (pxlsb + ((U16)plsb << 8) + ((U32)pmsb << 16)) / 4096. / 1.3332;
      f             = PressureParam * C1_P + C0_P;
      
      if( CHECK_FLOAT(f) )            Errors |= PRESS_ERROR;
      else
      {
        __disable_interrupt(); // защита от обмена
        Pressure = f;
        RelPress = round(Pressure - 700);
        if( Pressure > 827 )        { RelPress =  127; }
        else if( Pressure < 573 )   { RelPress = -127; }  
        __enable_interrupt(); // защита от обмена
      }        
    }
    else
    {
      f = (F32)ADC12MEM9;
      if( fabs(PressureParam - f) > 20 )  PressureParam  = f;                             // колебани€ свыше 20 разр€дов без фильтра 
      else                                PressureParam += 0.05 * (f - PressureParam);    // 

      f = PressureParam * C1_P + C0_P;
      if( CHECK_FLOAT(f) )            Errors |= PRESS_ERROR;
      else
      {
        __disable_interrupt(); // защита от обмена
          Pressure = f;
          RelPress = round(Pressure - 700);
          if( Pressure > 827 )        { Errors  |= PRESS_ERROR;  Pressure = 827;  RelPress =  127;  }
          else if( Pressure < 573 )   { Errors  |= PRESS_ERROR;  Pressure = 573;  RelPress = -127;  }
       __enable_interrupt(); // защита от обмена
      }
    }
  }
  else Errors |= PRESS_ERROR; // дл€ статистики
  
  Flags.measure_ready  =  0;
}

// пересчет единиц влажности
F32 hum_calc_7m(U8 sel)
{
  F32   e;
    
  e =  Humidy / 100. * e_calc(Tempr);

  switch(sel)
  {    
    case  HTR_MODE:  return  tcalc(e_calc, e);                                     // oC dew point
    case  HGM_MODE:  return  217. * e / (273.15 + Tempr);                          // g/m3
  }
  
  return    0;
}

#elif defined (__7K_PRA_MSD_USER)
void  measure_process_7k(void)
{     
  F32           f;
  
  Errors             &= ~PRESS_ERROR;
  if( ISNOMEAS_MODE ) { Flags.measure_ready = 0; return; }
  
  // расчет напр€жени€ питани€
  Errors &= ~PW_ERROR;
  f =  ADC12MEM8 * Ref15 * 3.128;
  if( fabs(Power - f) > 0.02 )          Power   = f;                   // колебани€ свыше 20 м¬ без фильтра
  else                                  Power  += 0.05 * (f - Power);  // фильтр   
  
  if( ISCHARGE_MODE ) { Flags.measure_ready = 0; return; }
      
  if(Power < 3.3)               
  {
    KeyCode              = SW2;
    key_process();
    Errors              |= PW_ERROR;    
    Flags.measure_ready = 0; 
    return;
  }
  
  // уровень птани€ без USB
  if(!usb_on_off)
    CALCPOW_PR(PowerShow, Power, 4.2, 3.3);
  
  // расчет давлени€
  if(ConfWordCopy & PRES)
  {
    f=  (F32)ADC12MEM9;
    if( fabs(PressureParam - f) > 20 )  PressureParam  = f;                             // колебани€ свыше 20 разр€дов без фильтра 
    else                                PressureParam += 0.05 * (f - PressureParam);    // 

    f = PressureParam * C1_P + C0_P;
    if( CHECK_FLOAT(f) ) Errors  |= PRESS_ERROR;
    else
    {
      __disable_interrupt(); // защита от обмена
      Pressure = f;
      RelPress = round(Pressure - 700);
      if( Pressure > 827 )        { Errors  |= PRESS_ERROR;  Pressure = 827;  RelPress =  127;  }
      else if( Pressure < 573 )   { Errors  |= PRESS_ERROR;  Pressure = 573;  RelPress = -127;  }
      __enable_interrupt(); // защита от обмена
    }
  }
  
  Flags.measure_ready = 0;
}

F32 hum_calc_7k (U8 sel)
{
  F32   Ep = 0.1, f = 0.1;
  F32   p2 = (ConfigWord & PRESS_CALC) ? Pressure2 : 1.0;
  F32   pressA = 0;
  
  switch( ConfWordCopy & DEVTYPE_MASK )
  {                  
    case K1P_TYPE:  f       =  e_calc( Humidy_original ) * ((ConfigWord & PRESS_CALC) ? (Pressure2 / Pressure1) : 1.0);
                    if( !CHECK_FLOAT(f) ) Ep = f;
                    Humidy  =  tcalc(e_calc, Ep);  
                    break;       
                        
    case K7_TYPE:   f       =  Humidy_original / 100. * e_calc(Tempr)  * ((ConfigWord & PRESS_CALC) ? (Pressure2 / Pressure1) : 1.0);
                    if( !CHECK_FLOAT(f) ) Ep = f;
                    Humidy  =  Ep / e_calc(Tempr) * 100.;
                    if( (ConfigWord & PRES) && !(Errors & PRESS_ERROR) ) pressA = Pressure * 1.3332;  // в гѕа
                    else                                                 pressA = 1013.25;
                    break;
  } 
    
  f = 0;
  switch(sel)
  { 
    case  HPR_MODE:   f = Ep / e_calc(Tempr) * 100.; CHECK_LIM(f, 100, 0); break;
                                            
    case  HTR_MODE:   f = tcalc(e_calc, Ep); CHECK_LIM(f, 40, -99.9); break;
    
    case  HGM_MODE:   f = 217. * Ep / (273.15 + Tempr); CHECK_LIM(f, 99.9, 0); break;
      
    case  HMGM_MODE:  f = 217. * Ep / (273.15 + Tempr) * 1000; CHECK_LIM(f, 9999, 0); break;
      
    case  HPPM_MODE:  f = Ep / (1013.25 * p2 - Ep) * 1000000.; CHECK_LIM(f, 9999, 0); break;
      
    case  HTVT_MODE:  f = tcalc(ePLUS_calc, Ep + 6.62e-4 * pressA * Tempr); CHECK_LIM(f, 99.9, -99.9); break;
      
    case  THC1_MODE:  f = 0.7 * tcalc(ePLUS_calc, Ep + 6.62e-4 * pressA * Tempr) + 0.3 * TemprBlack; CHECK_LIM(f, 999.9, 0); break;
      
    case  THC2_MODE:  f = 0.7 * tcalc(ePLUS_calc, Ep + 6.62e-4 * pressA * Tempr) + 0.2 * TemprBlack + 0.1 * Tempr; CHECK_LIM(f, 999.9, 0); break; 
  }
  
  return f;
}
#else
  #error "No Type defined!"
#endif 

void  tresholds_process(void)
{  
  Errors &= ~(T1_ERROR + T2_ERROR + H1_ERROR + H2_ERROR);
  
  if( ISNOTRSH_MODE || !(ConfWordCopy & SOUND_ON) ) 
  {
    SOUND_OFF;
    return;
  }
  
  if( Errors & (HS_ERROR + TS_ERROR + PR_ERROR + TYPE_ERROR) ) 
  { 
    SOUND_ON1; 
    return; 
  }
  
  if(Tempr < TrshVal[0])  Errors |= T1_ERROR;
  if(Tempr > TrshVal[1])  Errors |= T2_ERROR;  
  if(Humidy < TrshVal[2]) Errors |= H1_ERROR;
  if(Humidy > TrshVal[3]) Errors |= H2_ERROR;    
  
  if( Errors & (T1_ERROR + T2_ERROR + H1_ERROR + H2_ERROR) )   
  { 
    SOUND_ON2; 
    return; 
  }  
  
  SOUND_OFF;
}

F32 calc(F32 input, U8 polnum)
{
    F32         f = 1., out, *poly;
    U8          i, plen;
    
    switch(polnum)
    {
      case 0:   poly = (float *)Pt385_Polynoms; plen = 6; break;
      case 1:   poly = (float *)ICorr_polynoms; plen = 2; break;
      case 2:   poly = (float *)RCorr_polynoms; plen = 3; break;
      case 3:   poly = (float *)HPTP_polynoms;  plen = 6; break;
    
      default:  return 1e38;
    }
            
    for(out = poly[0], i = 1; i < plen; i++)
    {
      f   *=  input;
      out +=  poly[i] * f;
    }
    return out;
}

F32 serj_func(F32 x)
{
    F32 y;
        
    if(x < 0) y= 1/(2 - x + x*x*.5);
    else      y= 1-1/(2 + x + x*x*.5);
    
    return (y*2 - 1);
}

// расчет давлени€ насыщенного пара (от температуры)
F32 e_calc(F32 t) 
{   
  if(t>=0)
        return calc(t, 3);

  // t < 0 - линейна€ интерпол€ци€ [интервал] = 1 градус
  S8    i;
  F32   y1, y2, a, b;
  
  if( t < -99 ) t = -99;
  i   =  -t; 
  y1  =  E_TAB[i]; 
  y2  =  E_TAB[i+1];
  a   =  (y1 - y2);
  b   =  y1 + a * i;
    
  return a * t + b;    
}


// рас2чет давлени€ насыщенного пара дл€ психрометра (вл. термометр)
F32 ePLUS_calc(F32 t) 
{
  return (e_calc(t) + 6.62e-4 * 1013.25 * t);
}

// расчет температурыю насыщенного пара (от его давлени€, методом делени€ попалам)
F32 tcalc(F32 (*calc)(F32),F32 e) 
{  
  F32   tt, ee;
  F32   t1= -99, t2= 99;
  U8    i;

  for(i = 0; i < 12; i++)
  {
    tt = (t1 + t2) / 2;
    ee = (*calc)(tt);

    if(e >= ee) t1 = tt;
           else t2 = tt;

    if((t2-t1) < 0.05) return tt;
  }
  return  tt;
}
    
// 0 ... -100 давление насыщенного пара от температуры ¬ћќ
__root const F32        E_TAB[101] = {
6.1114,                 //  0
5.6266,                 // -1
5.177,                  // -2
4.7605,                 // -3
4.3746,                 // -4
4.0175,                 // -5
3.6872,
3.3818,
3.0997,
2.8393,
2.5989,
2.3773,
2.171165305,
1.983281669,
1.810384169,
1.651385507,
1.505268814,
1.371083691,
1.247942432,
1.135016429,
1.031532749,
0.936770871,
0.850059584,
0.770774042,
0.698332957,
0.632195945,
0.571861002,
0.516862109,
0.466766967,
0.421174853,
0.379714586,
0.342042608,
0.307841171,
0.276816624,
0.248697798,
0.223234477,
0.200195972,
0.179369761,
0.160560224,
0.143587446,
0.128286096,
0.114504374,
0.102103021,
0.090954399,
0.080941624,
0.071957751,
0.063905025,
0.056694166,
0.050243716,
0.044479422,
0.039333663,
0.034744918,
0.03065727,
0.027019947,
0.023786896,
0.020916383,
0.018370633,
0.016115488,
0.014120092,
0.012356603,
0.010799927,
0.009427472,
0.008218921,
0.007156022,
0.0062224,
0.005403381,
0.004685829,
0.004058,
0.003509408,
0.003030703,
0.002613554,
0.002250554,
0.001935119,
0.001661411,
0.001424256,
0.001219076,
0.001041826,
0.000888937,
0.000757265,
0.000644044,
0.000546844,
0.000463532,
0.000392241,
0.00033134,
0.000279401,
0.000235182,
0.000197602,
0.00016572,
0.000138721,
0.0001159,
9.66459E-05,
8.04317E-05,
6.6804E-05,
5.53725E-05,
4.58024E-05,
3.78068E-05,
3.11403E-05,
2.55937E-05,
2.09886E-05,
1.71735E-05,
1.40198E-05     // -100
};

