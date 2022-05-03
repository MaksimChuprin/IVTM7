
// lcd modes
#define OFF_MODE                0
#define START_MODE              1
#define P_MODE                  2
#define T_MODE                  3
#define SD_MODE                 4
#define CHARGE_MODE             5
#define BAT_MODE                6
#define RT_MODE                 7

#define HPR_MODE                10
#define HGM_MODE                20
#define HTR_MODE                30

// radio config
#define STACK_DIS               BIT7

// LCD
#define LCD_SIZE                5
#define PRE_SAVE_TIME           10
 
#define PS_ON                   PJOUT |=  BIT2
#define PS_OFF                  PJOUT &= ~BIT2
#define HS_ON                   PJOUT |=  BIT1
#define HS_OFF                  PJOUT &= ~BIT1
#define TS_ON                   PJOUT |=  BIT0
#define TS_OFF                  PJOUT &= ~BIT0
#define ALL_SENS_OFF            { PS_OFF; HS_OFF; TS_OFF; }

#define LIGHT_MINUS             *(LCDMEM +  3)|= 0x10
#define LIGHT_DEG               *(LCDMEM + 11)|= 0x10
#define LIGHT_POWER             *(LCDMEM + 20)|= 0x01
#define LIGHT_PR                *(LCDMEM + 19)|= 0x10
#define LIGHT_GM3               *(LCDMEM + 20)|= 0x10

#define CLR_DEG                 *(LCDMEM + 11)&= 0x01
#define CLR_POWER               *(LCDMEM + 20)&= 0x10
#define CLR_PR                  *(LCDMEM + 19)&= 0x01
#define CLR_GM3                 *(LCDMEM + 20)&= 0x01
#define CLR_MINUS               *(LCDMEM +  3)&= 0x01

#define CLR_ALL_SYM             { CLR_DEG; CLR_PR; for(U8 i = 20; i < 26; i++) *(LCDMEM + i) = 0x00; }
