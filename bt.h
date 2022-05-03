#define BT_BUFLEN               256
#define BT_MAX_DATA_LEN         BT_BUFLEN - 16

#define ASK_BTEXCH              0
#define NAK_BTEXCH              255

#define WR_BT                   0x00
#define WF_BT                   0x01
#define WI_BT                   0x02

#define RR_BT                   0x80
#define RF_BT                   0x81
#define RI_BT                   0x82
#define RD_BT                   0x84

#define DW_BT                   0x56
#define IR_BT                   0x8F

#define START_TA2(sec)          { TA2CCR0  =  ACLK_HZ * (sec); TA2CTL = TASSEL__ACLK + ID_0 + TACLR + MC__UP + TAIE; }
#define STOP_TA2                  TA2CTL  &= ~(TAIE + TAIFG + MC__UP) 
#define FORCE_TA2_INT             TA2CTL  |=  TAIFG
#define BT_CHECK                ( P6OUT & BIT3 )
#define BT_ON                   { P6OUT   |= BIT3; P2SEL |= (BIT4 + BIT5); DELAY_USB_MS(10); UCA0CTL1 = UCSSEL__SMCLK; BtStat = 0; RXI_ENABLE; START_TA2(5); }
#define BT_OFF                  { UCA0CTL1 = UCSSEL__SMCLK + UCSWRST; P6OUT &= ~BIT3; P2SEL &= ~(BIT4 + BIT5); }

void    bt_process              (void);


