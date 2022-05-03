// commandes
#define ID_ASK                  0
#define CONFIG_ASK              1
#define MEASURE_ASK             2
#define MEASURE_ASK2            3
#define ERROR_ASK               4

#define START_TA2_B(A)          { TA2CCR0 = SMCLK_HZ * 10 * (A) / 9600;   TA2CTL = TASSEL__SMCLK + ID_0 + TACLR + MC__UP + TAIE; }
#define START_TA2_S(A)          { TA2CCR0 = ACLK_HZ * (A);                TA2CTL = TASSEL__ACLK  + ID_0 + TACLR + MC__UP + TAIE; }
#define IS_TIMEOUT              ( TA2CTL & TASSEL__ACLK )

#define RS7K_RX_EN              P2OUT    &= ~(BIT2+BIT3)
#define RS7K_RX_DS              P2OUT    |=  (BIT2+BIT3)
#define RS7K_TX_EN              P2OUT    |=  BIT6
#define RS7K_TX_DS              P2OUT    &= ~BIT6

#define PWPR_ON                 { P6OUT |=  (BIT6 + BIT7); P2SEL |=  (BIT0+BIT1); }
#define PWPR_OFF                { P6OUT &= ~(BIT6 + BIT7); P2SEL &= ~(BIT0+BIT1); }

void    master_process          (void);
void    master_uart_process     (void);