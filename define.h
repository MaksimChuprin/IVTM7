#include  <msp430.h>
#include  "limits.h"
#include  "math.h"
#include  "string.h"
#include  "stdio.h"
#include  "stdlib.h"
#include  "stdarg.h"

#include  "comm.h"
#include  "device_struct.h"
#include  "eksis.h"
#include  "usb_msp.h"
#include  "rs.h"
#include  "bt.h"
#include  "sd.h"
#include  "7k.h"
#include  "R433.h"
#include  "SX1276Driver.h"
#include  "lps22hb.h"
#include  "global_declaration.h"

#if   defined (__7M1_PRA_MSD_USER)
  #include "define_m1.h"
#elif defined (__7M2_PRA_MSD_USER)
  #include "define_m2-7.h"
#elif defined (__7K_PRA_MSD_USER)
  #include "define_7k.h"
#else
  #error "No Type defined!"
#endif

#define SPI_BUF_LEN           16
#define RAMSIZE               256
#define INFOENDADR            0x19FF
#define RamBeginAddr          (Int16U)&Tempr
#define FLASHENDADR           0x28000

#define SERVICE_ADR           0xFFFF

#define SEG_SIZE              0x0200
#define SEG_A                 0x1980
#define SEG_B                 0x1900
#define SEG_C                 0x1880
#define SEG_D                 0x1800

#pragma segment = "DEFCONF"
#pragma segment = "INFO"
#pragma segment = "INFOA"

// TLV 6636
#define ADC_GAIN                *((pU16)0x1A16)
#define ADC_OFFSET              *((pS16)0x1A18)

// measure modes
#define WAITING                 0
#define START_PRES              1
#define START_RES               2
#define START_MES               3

// keys definition
#define NONE                  0
#define SW1                   1
#define SW2                   2
#define SW3                   3
#define LONG_SW1              5
#define LONG_SW2              6
#define LONG_SW3              7
#define LONGKEY_F             4 

// config's bits
#define DEVTYPE_MASK          (15<<0)
#define TYPE_NODEF            (0<<0)
#define M1_TYPE               (1<<0)
#define M2_TYPE               (2<<0)
#define M3_TYPE               (3<<0)
#define M4_TYPE               (4<<0)
#define M4L_TYPE              (5<<0)
#define M6_TYPE               (6<<0)
#define M7_TYPE               (7<<0)
#define K7_TYPE               (8<<0)
#define K1P_TYPE              (9<<0)

#define SPEED_MASK            (15<<4)
#define S4800                 (0<<4)
#define S9600                 (1<<4)
#define S19200                (2<<4)
#define S38400                (3<<4)
#define S57600                (4<<4)
#define S115200               (5<<4)

#define NO_HUM                BIT8
#define RADIO_ON              BIT9
#define PRESS_CALC            BITA
#define PRES                  BITB
#define SLEEP_WR              BITC
#define NOOVERLAP_STAT        BITD
#define RADIO_PROT2           BITD
#define SOUND_ON              BITE
#define BLUE_ON               BITF

// error bits
#define SD_ERROR              (1L<<0)
#define TS_ERROR              (1L<<1)
#define HS_ERROR              (1L<<2)
#define PW_ERROR              (1L<<3)
#define T1_ERROR              (1L<<4)
#define T2_ERROR              (1L<<5)
#define H1_ERROR              (1L<<6)
#define H2_ERROR              (1L<<7)
#define STAT_OVR              (1L<<8)
#define PRESS_ERROR           (1L<<9)
#define PR_ERROR              (1L<<10)
#define BT_ERROR              (1L<<11)
#define TIME_ERROR            (1L<<12)
#define LOPW_ERROR            (1L<<13)
#define RT_ERROR              (1L<<14)
#define INFOMEM_ERROR         (1L<<15)

#define SD_IN                 (1L<<16)
#define TNS_TYPE              (1L<<17)
#define TYPE_ERROR            (1L<<18)
#define TS2_ERROR             (1L<<19)

#define RESET_STAT            (1L<<31)
  /* резерв */

#define MMHG                    0
#define gPA                     1

#define HUM_MODE                0
#define TEMP_MODE               1
#define PRESS_MODE              2

#define HALF_ADAY                (3600*12L)

#define SAVEMAX                 256
#define MESSAGELEN              16

/* macro */
#define WDRCLR                  WDTCTL  = WDTPW + WDTSSEL__VLO + WDTCNTCL + WDTIS__32K
#define BOOT_RESET              PMMCTL0 = PMMPW + PMMSWBOR
#define SOFT_RESET              WDTCTL  = 0

#define WriteFlashChar(adr,b)   WriteFlashString((Int16U)(adr),(pInt8U)&(b),sizeof(Int8U))
#define WriteFlashInt(adr,i)    WriteFlashString((Int16U)(adr),(pInt8U)&(i),sizeof(Int16U))
#define WriteFlashFloat(adr,f)  WriteFlashString((Int16U)(adr),(pInt8U)&(f),sizeof(Flo32))

#define SOUND_OFF               TB0CCTL1 =  OUTMOD_0 + OUT
#define SOUND_ON1               { TB0CCR0= 768; TB0CCR1= 128; TB0CCTL1= OUTMOD_3; } 
#define SOUND_ON2               { TB0CCR0= 256; TB0CCR1= 128; TB0CCTL1= OUTMOD_3; } 
#define SOUND_ON3               { TB0CCR0= 256; TB0CCR1= 64;  TB0CCTL1= OUTMOD_3; }

#define ISBAT_MODE              (Selector == BAT_MODE)
#define ISOFF_MODE              (Selector == OFF_MODE)
#define ISCHARGE_MODE           (Selector == CHARGE_MODE)
#define ISSTART_MODE            (Selector == START_MODE)
#define ISSD_MODE               (Selector == SD_MODE)
#define ISRT_MODE               (Selector == RT_MODE)

#define ISNOTRSH_MODE           ( ISOFF_MODE || ISCHARGE_MODE || ISSTART_MODE || ISBAT_MODE || ISSD_MODE )
#define ISNOSTAT_MODE           ( ISOFF_MODE || ISCHARGE_MODE || ISSTART_MODE || ISBAT_MODE )
#define ISSLEEP_MODE            ( ISOFF_MODE || ISCHARGE_MODE || ISBAT_MODE )
#define ISNOMEAS_MODE           ( ISOFF_MODE || ISBAT_MODE )

#define IS_DEV_TNS              ( Errors & TNS_TYPE )
#define CHECK_TYPE(type)        ( (ConfWordCopy & DEVTYPE_MASK) == (type) )
#define IS_DEV_IVG              CHECK_TYPE(K1P_TYPE)

#define CALCPOW_PR(lev , vol, hvol, lvol)   { \
                                                (lev) = (S16)(((vol) - (lvol)) / ((hvol)-(lvol)) * 100 + .5); \
                                                if((lev) > 100) (lev) = 100;  \
                                                else if((lev) < 0) (lev) = 0; \
                                            }
#define CHECK_LIM(v, h, l)      { \
                                  if((v) > (h)) (v) = (h);  \
                                  else if((v) < (l)) (v) = (l); \
                                }

#define  STACKPLUS              { \
                                  for(U16 j = saveCounter; j > 0; j--) memcpy( &saveBuffer[j][0], &saveBuffer[j - 1][0], MESSAGELEN); \
                                  if( saveCounter < (SAVEMAX - 1) ) saveCounter++; \
                                }

#define  STACKMINUS             { \
                                  for(U16 j = 1; j < saveCounter; j++) memcpy( &saveBuffer[j - 1][0], &saveBuffer[j][0], MESSAGELEN); \
                                  if( saveCounter ) saveCounter--; \
                                }

#define LPS22_CS_LOW            PJOUT   &= ~BIT3
#define LPS22_CS_HIGH           PJOUT   |=  BIT3

// time
#define SEC(A)                  (U16)((ACLK_HZ)*(A))

