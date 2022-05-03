#ifndef _GLOBAL_VAR_H
#define _GLOBAL_VAR_H

// predeclaration
extern volatile Flags_t           Flags;  
extern volatile sd_flags_struct   SD_Flags;
extern volatile U32               Time2Write, Time;
extern volatile U16               SD_Clock, MS_Clock, SleepClock, BlueClock;
extern volatile U16               RxCounter, TxByte2send, ConfWordCopy;
extern volatile U8                GenClock1, InByte, TimeSDNoOff, R433_txnum;               

extern F32                        Tempr, Humidy, TemprBlack, Res, Pressure, UC_Temp, Power, AdcGain, Ref15;
extern F32                        PressureParam, HumidyParam, Ias_Par, Ras_Par, Humidy_original; 
extern U32                        WriteAdr, Errors, STATSize, PageAdr, InfoTime;
extern U16                        Time2Measure, saveCounter;
extern S16                        RelPress, PowerShow;
extern U8                         FlashBuffer[], HotReset[], ID[], UartBuffer[], saveBuffer[256][16];
extern U8                         Lcd_Buffer[], LcdPointer, Selector, KeyCode, Hum_mode, Press_mode, SaveSel, BtStat;

extern const F32                  W1, W2, W11, W12, W21, W22, B2, B11, B12, C1_P, C0_P, C0_H, C1_H, Uref, Pressure1, Pressure2;
extern const F32                  E_TAB[], ICorr_polynoms[], RCorr_polynoms[], Pt385_Polynoms[], HP_polynoms[];
extern const U32                  ValidTime, StartStatistic, StartDescriptor;
extern const U16                  CheckSumD, CheckSumC;
extern const U8                   OfficialVersion[], Version[], SerialNumberStr[], MonthSize[], HotResetPattern[];
extern const U8                   ID_M1[], ID_M2[], ID_M3[], ID_M4[], ID_M4L[], ID_M6[], ID_M7[], ID_K7[], ID_KP1[];
extern const U8                   DataSizeSegC, DataSizeSegD, DP1203IniTab[], Crc8Tab[], DiscriptorArray[];

extern const F32                  TrshVal[], Pessure1, Pessure2;
extern const U16                  ConfigWord, WritePeriod, SleepPeriod, BtActivePeriod;

void ini_common                   (void);
void ini_spec                     (void);
void ini_ram                      (void);
void lcd_process                  (void); 
void measure_process_7m           (void);
void measure_process_7k           (void);
void statistic_process            (void);
void tresholds_process            (void);
void sd_process                   (void);
void key_process                  (void);
void prog_serv                    (void);
void lcd_clear                    (void);
void radio433_process             (void);
void radio868_process             (void);
void choose_uart_speed            (U16 speed);
void binbuffer_2_binbuffer        (Int8U __data20 * pBufferSource, Int8U __data20 * pBufferDestination, Int16U nBytes);

U16 getConfigW                    (void);
F32 hum_calc_7m                   (U8 sel);
F32 hum_calc_7k                   (U8 sel);
void LPS22_Write                  (U8 adr, U8 data);
U8   LPS22_Read                   (U8 adr);

U16  calcCHK16                    (pInt8U buffer, Int16U len);
U16  WriteFlashString             (Int32U adr, pInt8U bufpoint, Int16U len);
U16  writeFlash                   (Int32U writeAdr, pInt8U buffer, Int16U num);
void CheckInfoFlash               (void);

extern void msp_usb_init ();
extern void msp_usb_loop ();
extern volatile uint8_t usb_on_off;
extern volatile uint8_t usb_write_data;
void bin2str (uint8_t *buf, uint8_t *s);
extern uint8_t get_bin_size ();

#endif

