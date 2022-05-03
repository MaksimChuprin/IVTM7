#define MAX_DATA_LEN            64
#define UART_BUF_LEN            256

#define START_TA2_5ST           TA2CTL  = TASSEL__SMCLK + ID_0 + TACLR + MC__UP + TAIE
#define RXI_ENABLE              { RxCounter = 0; InByte = UCA0RXBUF; UCA0IE |= UCRXIE; }
#define IS_RXIEN                ( UCA0IE & UCRXIE )
#define RXI_DISABLE             UCA0IE   &= ~UCRXIE
#define TXI_ENABLE              UCA0IE   |=  UCTXIE
#define TXI_DISABLE             UCA0IE   &= ~UCTXIE
#define TX_START                { UCA0IFG |= UCTXIFG; TXI_ENABLE; }

#define SET_U0_2SB              UCA0CTL0 |=  UCSPB
#define SET_U0_1SB              UCA0CTL0 &= ~UCSPB

#define RS_TX_EN                P6OUT    |=  BIT3
#define RS_TX_DS                P6OUT    &= ~BIT3
#define RS_CHECK                !(P7IN & BIT7)

#define NetAdr                  BtActivePeriod

void    slave_uart_process      (void);
Int8U   hex2char                (pU8 buffer, Int16U pointer);
Int16U  hex2int                 (pU8 buffer, Int16U pointer);
Int16U  char2hex                (U8 cnum);
Int8U   check_sum               (pU8 buffer, Int16U len);
void    hexbuffer_2_binbuffer   (pInt8U hexBuffer, pInt8U binBuffer, Int16U nBytes, Int16U Pointer);
void    binbuffer_2_hexbuffer   (pInt8U hexBuffer, Int8U __data20* binBuffer,Int16U nBytes,Int16U Pointer);