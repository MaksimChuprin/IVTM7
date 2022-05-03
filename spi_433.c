#include  "define.h"

static void xeput_char(U8 adr, U8 c)
{
  volatile U8 d;
    
  DP1203EN_LOW;
  SHIFT_BYTE_DP1203(0x80 + adr,d);
  SHIFT_BYTE_DP1203(c, d);
  SHIFT_BYTE_DP1203(0xff, d);
  DP1203EN_HI;
  return;
}

static Int8U xeget_char(U8 adr)
{
  volatile U8 d, c;
  
  DP1203EN_LOW;;
  SHIFT_BYTE_DP1203(0xA0 + adr, d);
  SHIFT_BYTE_DP1203(0xff, c);
  SHIFT_BYTE_DP1203(0xff, d);
  DP1203EN_HI;
  return c;
}

U8 check_sum_CRC8(pU8 buffer, U8 len)
{
  U8 crc = 0;
        
  for(U8 i = 0; i < len; i++) crc = Crc8Tab[buffer[i] ^ crc];
  return crc;
}

// ini xe1203 - sleep mode
U8  dp1203_ini(void)
{  
  spi2master();
  xeput_char(3, DP1203IniTab[3]);
  if(xeget_char(3) != DP1203IniTab[3])   return 1;
  
  for(U8 i = 0; i < 19; i++)  xeput_char(i, DP1203IniTab[i]);
  return 0;
}

// start quartz - standby mode
void  dp1203_stby(U8 channum)
{
  U8    l, h;
  S16   frq;
    
  frq  = -2000 + 400 * channum;
  l    =  frq & 0xff;
  h    =  frq >> 8;
  
  spi2master();
  U8 i = xeget_char(6);
  xeput_char(6, (i & ~BIT7) | BIT6);    // standby mode  
  xeput_char(7, h);                     // new band
  xeput_char(8, l);                     // new band

  DELAY_USB_MS(2);
  COM433_OFF;
}

// recieve mode
void  dp1203_rx(void)
{  
  spi2master();
  U8 i = xeget_char(6);
  xeput_char(6, (i & ~BIT6) | BIT7);
  COM433_RX;
  spi2slave();
}

// transmit mode
void  dp1203_tx(void)
{  
  spi2master();
  U8 i = xeget_char(6);
  xeput_char(6, i | BIT7 | BIT6);
  COM433_TX;
  spi2slave();
}

// sleep mode
void  dp1203_sleep(void)
{  
  spi2master();
  U8 i = xeget_char(6);
  xeput_char(6, i & ~(BIT7 + BIT6));
  spi2slave();
  COM433_OFF;
}

void  spi2master(void)
{
  MUX_MASTER;  
  UCB0CTL1  = UCSSEL1 + UCSWRST; 
  UCB0CTL0  = UCSYNC + UCMST + UCMSB;
  UCB0STAT  = 0;
  UCB0CTL1  = UCSSEL1; 
}

void  spi2slave(void)
{
  UCB0CTL1  = UCSSEL1 + UCSWRST; 
  UCB0CTL0  = UCSYNC + UCCKPH + UCMSB;
  UCB0STAT  = 0;
  UCB0CTL1  = UCSSEL1;
  UCB0TXBUF = 0xff;
  MUX_SLAVE;
}

const U8        DP1203IniTab[19]= 
{
0,                          // 0   config num - 1
(BIT6),                     // 1   SYNC-off, BARKER-on,RSSI-off,FEI-off,BW-200kHz,OSC-int,CLK_OUT-off
(BIT0),                     // 2   PreFilter-off,MOD-on,IQAmpl-off,SWITCH-out,PATTERN-off,BAND-433
55,                         // 3   divation - 55kHz
0,                          // 4   BAUDRATE - no meter (Barker)
0,                          // 5   must be 0
0x30,                       // 6   Reciveve mode - ModeA, Power - 15dBm
0,                          // 7   Freq highbyte - 434MHz
0,                          // 8   Freq lowbyte  - 434MHz
0,                          // 9   second config
0,                          // 10  second config
0,                          // 11  second config
0,                          // 12  OUT_RSSI & OUT_FEI
0,                          // 13  OUT_FEI
0,                          // 14  Patern_Size, ERR_NUM, CLKOUT_DIV, DATA_INV-off, BWControl - on
0,                          // 15  0-better
0,                          // 16  BARKER_TRS-hardware
BIT7,                       // 17  DATA_IN_OUT-off
0                           // 18
}; 

const U8        Crc8Tab[256]=
{ 
0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};
