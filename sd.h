//SD Standard Commands
#define GO_IDLE_STATE 				0x40
#define SEND_OP_COND  				0x41
#define SWITCH_FUNC				0x46
#define SEND_IF_COND				0x48
#define SEND_CSD	  			0x49
#define SEND_CID	  			0x4A
#define	STOP_TRANSMISSION			0x4C
#define SEND_STATUS 				0x4D
#define	SET_BLOCKLEN				0x50
#define READ_SINGLE_BLOCK 			0x51
#define READ_MULTIPLE_BLOCK			0x52
#define WRITE_BLOCK 				0x58
#define	WRITE_MULTIPLE_BLOCK			0x59
#define PROGRAM_CSD				0x5B
#define SET_WRITE_PROT				0x5C
#define CLR_WRITE_PROT				0x5D
#define SEND_WRITE_PROT				0x5E
#define ERASE_WR_BLK_START_ADDR 		0x60
#define ERASE_WR_BLK_END_ADDR 			0x61
#define ERASE_CMD				0x66
#define	LOCK_UNLOCK				0x6A
#define APP_CMD					0x77
#define GEN_CMD					0x78
#define READ_OCR	 			0x7A
#define CRC_ON_OFF				0x7B

//SD Application Specific Commands
#define	SD_STATUS				0x4D
#define SEND_NUM_WR_BLOCKS			0x56
#define SET_WR_BLK_ERASE_COUNT			0x57
#define SD_SEND_OP_COND				0x69
#define SET_CLR_CARD_DETECT			0x6A
#define SEND_SCR				0x73

//Data Tokens
#define INITIAL_CRC 				0x95
#define INTERFACE_COND_CRC                      0x87
#define DUMMY_CRC 				0xFF
#define DUMMY_DATA 				0xFF
#define STUFF_BITS 			        0x00
#define START_BLOCK				0xFE
#define BLOCK_LENGTH				0x0200
#define HIGH_CAPACITY				0x40

#define INTERFACE_COND				0x000001AA
#define VER2_OP_COND				1073741824L
#define AU_SIZE                                 128

//Status and Error Codes
#define IN_IDLE_STATE 				0x01
#define SUCCESS 				0x00
#define DATA_ACCEPTED 				0x05
#define CRC_ERROR 				0x0B
#define WRITE_ERROR 				0x0D
#define ERROR					0x01
#define CC_ERROR				0x02
#define CARD_ECC_FAILED				0x04
#define OUT_OF_RANGE				0x08
#define ILLEGAL_COMMAND_IDLE_STATE  		0x05

// response len
#define R1_RESPONSE                             0x01
#define R2_RESPONSE                             0x02
#define R3_RESPONSE                             0x04
#define R7_RESPONSE                             0x04

U8 sd_initialization                            (void);
U8 sd_version1_initialization                   (void);
U8 sd_version2_initialization                   (void);
U8 sd_read_block                                (U32 sector, pU8 pBuffer);
U8 sd_read_buf                                  (U32 sector, pU8 pBuffer);
U8 sd_write_block                               (U32 sector);
U8 sd_write_buf                                 (U32 sector, const U8 *buf);
U8 sd_erase_block                               (U32 starting_sector, U32 total_sectors);
U8 spi_xmit_byte                                (U8  byte);
void sd_spi_clear                               (void);

#define SHIFT_BYTE_SD(A,B)                      { UCB0TXBUF = (A); while(UCB0STAT & UCBUSY); (B) = UCB0RXBUF; }
#define SET_SD_SPI_300K                         { UCB0CTL1 |= UCSWRST; UCB0BRW=    7; UCB0CTL1 &= ~UCSWRST; }
#define SET_SD_SPI_1M                           { UCB0CTL1 |= UCSWRST; UCB0BRW=    0; UCB0CTL1 &= ~UCSWRST; }

#define SD_CS_OFF                               P5DIR   &= ~BIT3
#define SD_CS_HIGH                              { P5OUT |=  BIT3; P5DIR |=  BIT3;  }
#define SD_CS_LOW                               { P5OUT &= ~BIT3; P5DIR |=  BIT3;  }
#define SD_ON                                   { SD_CS_OFF;      P5OUT |=  BIT6;  }
#define SD_OFF                                  { P5OUT &= ~BIT6; SD_CS_OFF; spi_xmit_byte(0); }
#define CHECK_SD_PRES                           (P5IN & BIT3)
#define CHECK_SD_ON                             (P5IN & BIT6)
