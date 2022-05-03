#include  "define.h"

// глобальные
F32                       AdcGain;
F32                       Ref15;
F32                       Power = 4.0;
U32                       InfoTime;

volatile sd_flags_struct  SD_Flags;
volatile Flags_t          Flags;
volatile U16              MS_Clock;
volatile U16              SD_Clock;
volatile U16              SleepClock;
volatile U16              BlueClock;
volatile U16              RxCounter;
volatile U16              TxByte2send;
volatile U16              ConfWordCopy;
volatile U8               GenClock1;
volatile U8               InByte;
volatile U8               TimeSDNoOff;
volatile U8               R433_txnum;

U8                        UartBuffer[UART_BUF_LEN];
U8                        ID[48];
U8                        KeyCode;
U8                        Press_mode = MMHG;
U8                        Hum_mode   = HPR_MODE;
U8                        Selector   = OFF_MODE;
U8                        SaveSel    = HPR_MODE;

// глобальные в сегменте RAM - начальный внешний адрес = 0х0000
#pragma dataseg = RAM
__root __no_init F32    Tempr;                  // 0000
__root __no_init F32    Humidy;                 // 0004
__root __no_init F32    Pressure;               // 0008
__root __no_init F32    TemprBlack;             // 000C

__root __no_init F32    Res;                    // 0010
__root __no_init F32    PressureParam;          // 0014
__root __no_init U32    WriteAdr;               // 0018
__root __no_init U32    Errors;                 // 001C

__root __no_init volatile U32  Time;            // 0020
__root __no_init S16    RelPress;               // 0024

__root __no_init F32    UC_Temp;                // 0026
__root __no_init F32    Ias_Par;                // 002A
__root __no_init F32    Ras_Par;                // 002E

__root __no_init U32    STATSize;               // 0032
__root __no_init S16    PowerShow;              // 0036

__root __no_init volatile U32 Time2Write;       // 0038
__root __no_init F32    HumidyParam;            // 003C
__root __no_init F32    Humidy_original;        // 0040

__root __no_init U8     HotReset[8];
#pragma memory = default

//  константы в спец сегментах DISCRIPTOR
#pragma constseg = DISCRIPTOR
#pragma data_alignment = 512
__root const U8         DiscriptorArray[1024];          //  обозначаем начало массива 

//  константы в спец сегментах
#pragma constseg = STATISTIC
#pragma data_alignment = 512
__root const U8         StatisticArray = 0x12;         //  обозначаем начало массива статистики

#pragma constseg = INFOD   // 0x1800
__root const U16        ConfigWord;
__root const U16        WritePeriod;
__root const U16        SleepPeriod;
__root const U16        dummy1;
__root const F32        TrshVal[4];
__root const F32        Pressure1;
__root const F32        Pressure2;
__root const U16        CheckSumD = 0x1234;

#pragma constseg = INFOB   // 0x1900
__root const U16        __ConfigWord        =   TYPE_NODEF;
__root const U16        __WritePeriod       =   300;
__root const U16        __SleepPeriod       =   600;
__root const U16        __dummy1;
__root const F32        __TrshVal[4]        =   { -20.0, 60.0, 1.0, 99.0 };
__root const F32        __Pressure1          =  1.0;
__root const F32        __Pressure2          =  1.0;
#pragma constseg = default

__root const U16        CheckSumC           =  0;
__root const U8         DataSizeSegD        =  32;
__root const U8         DataSizeSegC        =  0;

#pragma location = 0x1948
__root const F32        C0_P =   0;
#pragma location = 0x194C
__root const F32        C1_P =   0.375;

#pragma location = 0x1950
__root const U8         SerialNumberStr[]  = "10000000";

#pragma location = 0x1964
__root const U32        ValidTime = 1514764800L;        // 1/1/2018

// для совместимости общих модулей кода
__root const F32        ICorr_polynoms[];
__root const F32        RCorr_polynoms[];
__root const F32        W1, W2, W11, W12, W21, W22, B2 , B11, B12;
__root const F32        C0_H, C1_H, Uref  = 1.5;

/* стандартые константы */
__root const U32        StartStatistic    = (Int32U) &StatisticArray;
__root const U32        StartDescriptor   = (Int32U)&DiscriptorArray[0];
__root const U8         HotResetPattern[] = "RAM Val";
__root const U8         OfficialVersion[] = "r1.07";
__root const U8         ID_K7[]           = "ИBTМ-7К";
__root const U8         ID_KP1[]          = "ИBГ-1К-П";
__root const U8         Version[]         = "r1.02";
/*
 -              1.01 баг со статистикой
21/04/21        1.02 не показывал температуру влажного термометра с обычным преобразователем (без ТНС)
*/
