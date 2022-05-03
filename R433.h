#define SHIFT_BYTE_DP1203(A,B)  { UCB0TXBUF = (A); while(UCB0STAT & UCBUSY); (B) = UCB0RXBUF; }

#define COM433_TX               { P6OUT |=  BIT7; P6OUT &= ~BIT6; }
#define COM433_RX               { P6OUT |=  BIT6; P6OUT &= ~BIT7; }
#define COM433_OFF              P6OUT   &= ~(BIT6 + BIT7)
#define MUX_SLAVE               P6OUT   &= ~BIT5
#define MUX_MASTER              P6OUT   |=  BIT5

#define DP1203EN_HI             P2OUT   |=  BIT0
#define DP1203EN_LOW            P2OUT   &= ~BIT0

#define DP1203_SPI_TX_START     { UCB0STAT = 0; UCB0IE |= UCTXIE; }
#define DP1203_SPI_TXI_DISABLE  UCB0IE  &= ~UCTXIE
#define DP1203_TX_INPROGRESS    (UCB0IE & UCTXIE)

#define UnitAddress             BtActivePeriod
#define UnitBand                (*((pU8)TrshVal))
#define RadioSpeed              (*((pU8)TrshVal + 1))
#define RadioPower              (*((pU8)TrshVal + 2))
#define PacketNum               (*((pU8)TrshVal + 3))

#define BYTE_TIME_MS            7

void  spi2slave(void);
void  spi2master(void);
void  dp1203_sleep(void);
void  dp1203_tx(void);
void  dp1203_rx(void);
void  dp1203_stby(U8 channum);
U8    dp1203_ini(void);
U8    check_sum_CRC8(pU8 buffer, U8 len);
