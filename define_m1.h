// lcd modes
#define OFF_MODE                0
#define START_MODE              1
#define P_MODE                  2
#define T_MODE                  3
#define SD_MODE                 4
#define CHARGE_MODE             5
#define BAT_MODE                6

#define HPR_MODE                10
#define HTR_MODE                11
#define HGM_MODE                12
#define HMGM_MODE               13
#define HPPM_MODE               14
#define HTVT_MODE               15
#define THC1_MODE               16
#define THC2_MODE               17

// LCD
#define LCD_SIZE                4
#define PRE_SAVE_TIME           10
 
#define PS_ON                   PJOUT |=  BIT2
#define PS_OFF                  PJOUT &= ~BIT2
#define HS_ON                   P6OUT |=  (BIT6 + BIT7)
#define HS_OFF                  P6OUT &= ~(BIT6 + BIT7)
#define TS_ON                   PJOUT |=  BIT0
#define TS_OFF                  PJOUT &= ~BIT0
#define ALL_SENS_OFF            { PS_OFF; HS_OFF; TS_OFF; }

#define LIGHT_DEG               *(LCDMEM + 18)|= 0x10
#define LIGHT_POWER             *(LCDMEM + 16)|= 0x01
#define LIGHT_PR                *(LCDMEM + 17)|= 0x01
#define LIGHT_GM3               *(LCDMEM + 18)|= 0x01
#define LIGHT_LOW_TR            *(LCDMEM + 16)|= 0x10
#define LIGHT_UP_TR             *(LCDMEM + 15)|= 0x10
#define LIGHT_P                 *(LCDMEM + 19)|= 0x10
#define LIGHT_T                 *(LCDMEM + 19)|= 0x01

#define CLR_DEG                 *(LCDMEM + 18)&= 0x01
#define CLR_POWER               *(LCDMEM + 16)&= 0x10
#define CLR_PR                  *(LCDMEM + 17)&= 0x10
#define CLR_GM3                 *(LCDMEM + 18)&= 0x10
#define CLR_LOW_TR              *(LCDMEM + 16)&= 0x01
#define CLR_UP_TR               *(LCDMEM + 15)&= 0x01
#define CLR_P                   *(LCDMEM + 19)&= 0x01
#define CLR_T                   *(LCDMEM + 19)&= 0x10

#define CLR_ALL_SYM             { CLR_UP_TR; for(U8 i = 16; i < 26; i++) *(LCDMEM + i) = 0x00; }



