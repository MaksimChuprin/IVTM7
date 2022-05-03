#include  "define.h"

// глобальные
F32                       AdcGain;
F32                       Ref15;
F32                       Power = 4.0;
U32                       InfoTime;
F32                       TemprBlack;

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
__root __no_init F32    HumidyParam;            // 000C

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

__root __no_init U8     HotReset[8];
#pragma memory = default

//  константы в спец сегментах DISCRIPTOR
#pragma constseg = DISCRIPTOR
#pragma data_alignment = 512
__root const U8         DiscriptorArray[1024];          //  обозначаем начало массива с

//  константы в спец сегментах
#pragma constseg = STATISTIC
#pragma data_alignment = 512
__root const U8         StatisticArray = 0x12;         //  обозначаем начало массива статистики

#pragma constseg = INFOD   // 0x1800
__root const U16        ConfigWord;             // 0x1800
__root const U16        WritePeriod;            // 0x1802
__root const U16        SleepPeriod;            // 0x1804
__root const U16        BtActivePeriod;         // 0x1806
__root const F32        TrshVal[4];             // 0x1808
__root const U16        CheckSumD = 0x1234;

#pragma constseg = INFOB   // 0x1900
__root const U16        __ConfigWord        =   M1_TYPE + SLEEP_WR;
__root const U16        __WritePeriod       =   60;
__root const U16        __SleepPeriod       =   120;
__root const U16        __BtActivePeriod    =   120;
__root const F32        __TrshVal[4]        =   { -20.0, 60.0, 1.0, 99.0 };
#pragma constseg = default

__root const U16        CheckSumC           =  0;
__root const U8         DataSizeSegD        =  24;
__root const U8         DataSizeSegC        =  0;

#pragma location = 0x191C
__root const F32        C0_P=   0;
#pragma location = 0x194C
__root const F32        C1_P=   0.375;

#pragma location = 0x1920
__root const F32        W1;
#pragma location = 0x1924
__root const F32        W2;
#pragma location = 0x1928
__root const F32        W11;
#pragma location = 0x192C
__root const F32        W12;
#pragma location = 0x1930
__root const F32        W21;
#pragma location = 0x1934
__root const F32        W22;
#pragma location = 0x1938
__root const F32        B2=     50.;
#pragma location = 0x193C
__root const F32        B11;
#pragma location = 0x1940
__root const F32        B12;
#pragma location = 0x1944
__root const F32        C0_H =   1.;
#pragma location = 0x1948
__root const F32        C1_H;

#pragma location = 0x1950
__root const U8         SerialNumberStr[]  = "10000000";

#pragma location = 0x1960
__root const F32        Uref = 1.5;

#pragma location = 0x1964
__root const U32        ValidTime = 1514764800L;        // 1/1/2018

// калибровка температуры
#pragma location = 0x196C
__root const F32        ICorr_polynoms[] = { 0, 1 };
#pragma location = 0x1974
__root const F32        RCorr_polynoms[] = { 0, 1, 0 };


// для совместимости общих модулей кода
__root const F32        Pessure1, Pessure2;

/* стандартые константы */
__root const U32        StartStatistic    = (Int32U) &StatisticArray;
__root const U32        StartDescriptor   = (Int32U)&DiscriptorArray[0];
__root const U8         HotResetPattern[] = "RAM Val";
__root const U8         OfficialVersion[] = "r4.06";
__root const U8         ID_M1[]           = "ИBTМ-7М1";
__root const U8         Version[]         = "r1.03";
/* история версий
1.01 баг со статистикой
02/07/21 1.02 - Борьба с всплесками в измерениях
17/11/21 1.03 - Ошибка пересчета точки росы в М1 и М2...7
*/