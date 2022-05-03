/*******************************************************************
** File        : XE1205driver.h                                   **
********************************************************************
**                                                                **
** Version     : V 1.0                                            **
**                                                                **
** Written by   : Miguel Luis & Grégoire Guye                     **
**                                                                **
** Date        : 19-01-2004                                       **
**                                                                **
** Project     : API-1205                                         **
**                                                                **
********************************************************************
** Changes     : V 2.1 / MiL - 24-04-2004                         **
**                                                                **
**             : V 2.2 / MiL - 30-07-2004                         **
**               - Removed workaround for RX/TX switch FIFO clear **
**                 (chip correction)                              **
**                                                                **
** Changes     : V 2.3 / CRo - 06-06-2006                         **
**               - I/O Ports Definitions section updated          **
**                                                                **
** Changes     : V 2.4 / CRo - 09-01-2007                         **
**               - No change                                      **
**                                                                **
**                                                                **
********************************************************************
** Description : XE1205 transceiver drivers Implementation for the**
**               XE8000 family products (1205 buffered mode)      **
*******************************************************************/

#ifndef __SX1276DRIVER__
#define __SX1276DRIVER__

#include  <msp430.h>
#include  "msp430_comm.h"
#include  "sx1276.h"

#define ASK_LORA                  0
#define ASKTIME_LORA              1
#define NOASK_LORA                2
#define NOASKTIME_LORA            3

#define FREQ_CARRY                865000000L
#define CHANEL_SIZE               1000000L

#define SAVEMAX                   256
#define MESSAGEMAXLEN             16
#define MESSAGELEN                16

#define BW_Setting                6
#define DATARATE_Setting          12
#define PREAMBLELEN_Setting       6
#define CODERATE_Setting          1
#define POWEROUT_Setting          2
#define LOW_TRESH_FREE            -80

U8                                GetPaSelect ( U32 channel );
U8                                spi_write   (U8 byte);
void                              spi_nss     (U8 NSS);
void                              wait_ms     (U32 time);
void                              Radio_Reset (void);

#define NSS_HI                    P2OUT |=  (1<<0)
#define NSS_LOW                   P2OUT &= ~(1<<0)
#define RES_HI                    P6DIR &= ~(1<<4)
#define RES_LOW                   P6DIR |=  (1<<4)
#define SHIFT_BYTE_SX1276(A)    { UCB0TXBUF= (A); while(UCB0STAT & UCBUSY); (A) = UCB0RXBUF; }
#define TXRX_READYINT_EN        { P2IFG =  0; P2IE = (1 << 6); }
#define TXRX_READYINT_DIS       { P2IFG =  0; P2IE = 0; }
#define TXRX_INPROGRESS           ( P2IE & (1 << 6) )

#endif /* __SX1276DRIVER__ */
