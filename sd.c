#include  "define.h"

static U8           SdBuffer[16];
extern U32 sd_card_sector_count;
extern volatile U16               SD_Clock;

void sd_spi_clear(void)
{
  SD_CS_HIGH;
  DELAY_USB_MS(1);
  SET_SD_SPI_300K;
  for(U8 i = 0; i < 10; i++)    spi_xmit_byte(0xff);  // пред-инициализация > 74 тактов с "1"     
  SD_CS_LOW;
}

// spi_xmit_byte
inline U8 spi_xmit_byte(U8 byte)
{ 
  U8 d;
  
  SHIFT_BYTE_SD(byte, d);
  return d;
}
// SPI_XMIT_COMMAND 
void spi_xmit_command(U8 command, U32 data, U8 crc)
{
  U8 i, full_command[6];			
	
  // get real adr for SD <= 2Gb
  if(SD_Flags.data_sector && !SD_Flags.HCSD) data *= 512;
	
  full_command[0] = command;	
  full_command[5] = crc;

  // load data to command
  for(i= 4; i > 0; i--)
  {
    full_command[i] = data & 0xff;
    data = data >> 8;
  }
  // send 
  for(i= 0; i < 6; i++) spi_xmit_byte(full_command[i]);			
} 

// SD_COMMAND_RESPONSE 
void sd_command_response(U8 response_type, pU8 response)
{ 		
  for(U8 i= 0; i < response_type; i++)  
      response[i] = spi_xmit_byte(DUMMY_DATA);
}

void sd_get_512_bytes (U8 *data)
{
  volatile uint16_t d=DUMMY_DATA;
  UCB0IE&=~3;
  DMACTL0 = (DMA1TSEL__USCIB0RX)|(DMACTL0&0xFF);                        
  DMA1CTL = DMADSTINCR0 | DMADSTINCR1 |DMADSTBYTE | DMASRCBYTE ;
  DMA1SZ = 512;                                
  DMA1DA = (void __data20*) &data[0];                           
  DMA1SA = (void __data20*)&UCB0RXBUF;                          
  DMA1CTL |= DMAEN;                                      

  DMACTL1 = DMA2TSEL__USCIB0TX;                        
  DMA2CTL = DMADSTBYTE | DMASRCBYTE ;
  DMA2SZ = 511;                                
  DMA2SA = (void __data20*) &d;                           
  DMA2DA = (void __data20*)&UCB0TXBUF;                        
  DMA2CTL |= DMAEN;                                   
  
  UCB0TXBUF = d;
  while (DMA2CTL&DMAEN) {};
}

void sd_send_512_bytes (U8 *data)
{
  UCB0IE&=~3;
  DMACTL0 = (DMA1TSEL__USCIB0TX)|(DMACTL0&0xFF);                        // UCB0TXIFG на канал 1
  DMA1CTL = DMASRCINCR0 | DMASRCINCR1 |DMADSTBYTE | DMASRCBYTE ;
  DMA1SZ = 511;                                // длина
  DMA1SA = (void __data20*) &data[1];                            // Источник
  DMA1DA = (void __data20*)&UCB0TXBUF;                        // Получатель
  DMA1CTL |= DMAEN;                                    // Начать
  UCB0TXBUF = data[0];
  while (DMA1CTL&DMAEN) {};
}

// SD_INITIALIZATION 
U8 sd_initialization(void)
{
  SD_Flags.HCSD        = 0;
  SD_Flags.data_sector = 0;
  SET_SD_SPI_300K;
      
  spi_xmit_command(GO_IDLE_STATE, STUFF_BITS, INITIAL_CRC);	        // CMD0    
  SD_Clock= SEC(0.2);
  do
  {
    sd_command_response(R1_RESPONSE, SdBuffer);
    if( !SD_Clock ) return ERROR;
  }
  while(SdBuffer[0] != IN_IDLE_STATE);
  spi_xmit_byte(DUMMY_DATA);  // 8 clock
  
  spi_xmit_command(SEND_IF_COND, INTERFACE_COND, INTERFACE_COND_CRC);	// CMD8
  SD_Clock= SEC(0.2);
  do
  {
    sd_command_response(R1_RESPONSE, SdBuffer);
    if( !SD_Clock ) return ERROR;
  }
  while((SdBuffer[0] != IN_IDLE_STATE) && (SdBuffer[0] != ILLEGAL_COMMAND_IDLE_STATE));
  
  // выбор карты 1 или 2-й версии
  if(SdBuffer[0] == ILLEGAL_COMMAND_IDLE_STATE) 
  {
    spi_xmit_byte(DUMMY_DATA);    // 8 clock
    return sd_version1_initialization();
  }
  else  
    return sd_version2_initialization();  
}

// SD_VERSION1_INITIALIZATION 
U8 sd_version1_initialization(void)
{
  SD_Clock = SEC(5);//(1.1);
  for(;;)
  {
    if(Flags.key)               key_process();
    if(Flags.lcd_update)        lcd_process();   
    WDRCLR;
    spi_xmit_command(APP_CMD, STUFF_BITS, DUMMY_CRC);           // CMD55
    do
    {
      sd_command_response(R1_RESPONSE, SdBuffer);
      if( !SD_Clock ) return ERROR;
    }
    while((SdBuffer[0] != IN_IDLE_STATE) && (SdBuffer[0] != SUCCESS));
    spi_xmit_byte(DUMMY_DATA);   // 8 clock
   		
    //Transmit SEND OP COND command   								
    spi_xmit_command(SD_SEND_OP_COND, STUFF_BITS, DUMMY_CRC);	// ACMD41
    do
    {
      sd_command_response(R1_RESPONSE,SdBuffer);
      if( !SD_Clock ) return ERROR;
    }
    while((SdBuffer[0] != IN_IDLE_STATE) && (SdBuffer[0] != SUCCESS));    
    spi_xmit_byte(DUMMY_DATA);   // 8 clock
    
    // analize
    if(SdBuffer[0] == SUCCESS) break;
  }
  
  // Transmit SET_BLOCKLEN command 
  spi_xmit_command(SET_BLOCKLEN, BLOCK_LENGTH, DUMMY_CRC);    // CMD16
  do
  {
    sd_command_response(R1_RESPONSE,SdBuffer);
    if( !SD_Clock ) return ERROR;
  }
  while(SdBuffer[0] != SUCCESS);
  spi_xmit_byte(DUMMY_DATA);   // 8 clock  
  
  spi_xmit_command(SEND_CSD, STUFF_BITS, DUMMY_CRC);	// CMD9
  do
  {
    sd_command_response(R1_RESPONSE, SdBuffer);
      if( !SD_Clock ) return ERROR;
  }
  while(SdBuffer[0] != SUCCESS);  
  
  do
  {
    sd_command_response(R1_RESPONSE, SdBuffer);
    if( !SD_Clock ) return ERROR;
  }
  while(SdBuffer[0] != 0xFE);  
  sd_command_response(16,SdBuffer);
  spi_xmit_byte(DUMMY_DATA);   // 8 clock 
  
  if ((SdBuffer[0] >> 6) == 0)
  {
    U32 n = (SdBuffer[5] & 15) + ((SdBuffer[10] & 128) >> 7) + ((SdBuffer[9] & 3) << 1) + 2;
    U32 csize = (SdBuffer[8] >> 6) + ((U32)SdBuffer[7] << 2) + ((U32)(SdBuffer[6] & 3) << 10) + 1;
    sd_card_sector_count = csize << (n - 9);
  }
  else
  {
    sd_card_sector_count=(((U32)SdBuffer[7] & 0x3F) << 16) | ((U16)SdBuffer[8] << 8) | SdBuffer[9];
    sd_card_sector_count++;
    sd_card_sector_count<<=10;			
  } 
  
  return SUCCESS;
}

//###################### SD_VERSION2_INITIALIZATION ##########################
U8 sd_version2_initialization(void)
{   
  // read R7 response
  sd_command_response(R7_RESPONSE, SdBuffer);
  spi_xmit_byte(DUMMY_DATA);  spi_xmit_byte(DUMMY_DATA);// 8 clock
  
  // check voltage support
  if(SdBuffer[2] != 0x01) return ERROR;
  // check pattern
  if(SdBuffer[3] != 0xaa) return ERROR;
    
  //SD_Clock= SEC(1.1);    
  SD_Clock= SEC(5);    
  for(;;)
  {		
    if(Flags.key)               key_process();
    if(Flags.lcd_update)        lcd_process();   
    WDRCLR;    
    spi_xmit_command(APP_CMD, STUFF_BITS, DUMMY_CRC);           // CMD55
    do
    {
      sd_command_response(R1_RESPONSE,SdBuffer);
      if( !SD_Clock ) 
        return ERROR;
    }
    while((SdBuffer[0] != IN_IDLE_STATE) && (SdBuffer[0] != SUCCESS)); 
    spi_xmit_byte(DUMMY_DATA);      // 8 clock
   								
    //spi_xmit_command(SD_SEND_OP_COND, VER2_OP_COND, DUMMY_CRC);	// ACMD41
    spi_xmit_command(SD_SEND_OP_COND, 0x40100000L, 0xCD);	// ACMD41
    do
    {
      sd_command_response(R1_RESPONSE,SdBuffer);
      if( !SD_Clock ) 
        return ERROR;
    }
    while((SdBuffer[0] != IN_IDLE_STATE) && (SdBuffer[0] != SUCCESS));
    spi_xmit_byte(DUMMY_DATA);      // 8 clock
    
    // analize
    if(SdBuffer[0] == SUCCESS) break;
  }
  
  spi_xmit_command(READ_OCR, STUFF_BITS, DUMMY_CRC);	// CMD58
  do
  {
    sd_command_response(R1_RESPONSE, SdBuffer);
    if( !SD_Clock ) return ERROR;
  }
  while(SdBuffer[0] != SUCCESS);  
  sd_command_response(R3_RESPONSE,SdBuffer);
  spi_xmit_byte(DUMMY_DATA);   // 8 clock
  
  //Check if card is High Capacity
  if(SdBuffer[0] & HIGH_CAPACITY)  SD_Flags.HCSD = 1;
    
  spi_xmit_command(SEND_CSD, STUFF_BITS, DUMMY_CRC);	// CMD9
  do
  {
    sd_command_response(R1_RESPONSE, SdBuffer);
      if( !SD_Clock ) return ERROR;
  }
  while(SdBuffer[0] != SUCCESS);  
  
  do
  {
    sd_command_response(R1_RESPONSE, SdBuffer);
    if( !SD_Clock ) return ERROR;
  }
  while(SdBuffer[0] != 0xFE);  
  sd_command_response(16,SdBuffer);
  spi_xmit_byte(DUMMY_DATA);   // 8 clock 
  
  if ((SdBuffer[0] >> 6) == 0)
  {
    U32 n = (SdBuffer[5] & 15) + ((SdBuffer[10] & 128) >> 7) + ((SdBuffer[9] & 3) << 1) + 2;
    U32 csize = (SdBuffer[8] >> 6) + ((U32)SdBuffer[7] << 2) + ((U32)(SdBuffer[6] & 3) << 10) + 1;
    sd_card_sector_count = csize << (n - 9);
  }
  else
  {
    sd_card_sector_count=(((U32)SdBuffer[7] & 0x3F) << 16) | ((U16)SdBuffer[8] << 8) | SdBuffer[9];
    sd_card_sector_count++;
    sd_card_sector_count<<=10;			
  } 
  return SUCCESS;
}

U8 sd_write_buf(U32 sector, const U8 *buf)
{
  U8            response;
  
  SET_SD_SPI_1M;
  
  P3OUT|=BIT3;
  
  SD_Flags.data_sector = 1;		//Data manipulation function

  //Transmit WRITE BLOCK command
  spi_xmit_command(WRITE_BLOCK, sector, DUMMY_CRC);   //CMD24
  SD_Clock= SEC(0.2);
  do
  {
    sd_command_response(R1_RESPONSE, SdBuffer);
    if( !SD_Clock ) 
      return ERROR;
  } while(SdBuffer[0] != SUCCESS);
  spi_xmit_byte(DUMMY_DATA);  // 8 clock
	
  // start token
  spi_xmit_byte(START_BLOCK);	

  sd_send_512_bytes ((U8*)buf);
  //for(uint16_t i= 0; i < 512; i++)           spi_xmit_byte(buf[i]);                    // добить сегмент 512 байт - "FF"

  //Transmit CRC
  spi_xmit_byte(DUMMY_DATA);
  spi_xmit_byte(DUMMY_DATA);

  //After data write, card with send a response stating if the card was accepted
  SD_Clock= SEC(0.2);
  do
  {
    response  =  spi_xmit_byte(DUMMY_DATA);	
    response &=  0x0F;		//Mask response to test for errors

    //If there where errors writing data, branch to error function
    if((response == CRC_ERROR) || (response == WRITE_ERROR) || !SD_Clock) 
      return ERROR;
  } while(response != DATA_ACCEPTED);

  //Card will respond with the DATA OUT line pulled low if the card is still busy
  //erasing. Continue checking DATA OUT line until line is released high.
  SD_Clock = SEC(4);
  do
  {        
    response = spi_xmit_byte(DUMMY_DATA);
    if( !SD_Clock ) 
      return ERROR;
    WDRCLR;
  } while(response != DUMMY_DATA);
  spi_xmit_byte(DUMMY_DATA);  // 8 clock
  P3OUT&=~BIT3;
  return SUCCESS;
}

U8 sd_read_buf(U32 sector, pU8 pBuffer)
{    
  SD_Flags.data_sector =    1;
  SET_SD_SPI_1M;

  //Transmit READ SINGLE BLOCK command
  spi_xmit_command(READ_SINGLE_BLOCK, sector, DUMMY_CRC);
  SD_Clock = SEC(1);
  do
  {
    sd_command_response(R1_RESPONSE, SdBuffer);
    if( !SD_Clock ) 
      return ERROR;
  } while(SdBuffer[0] != SUCCESS);

  // wait START_BLOCK tokeb\n
  SD_Clock= SEC(1);    
  do
  {    
    sd_command_response(R1_RESPONSE, SdBuffer);
    if( !SD_Clock ) 
      return ERROR;
    WDRCLR;  
  } while(SdBuffer[0] != START_BLOCK);
  
  sd_get_512_bytes (pBuffer);
  /*for(U16 i = 0; i < 512; i++)  
  {
    pBuffer[i] = spi_xmit_byte(DUMMY_DATA); // загружаем в буффер только данные 0 сектора
  }*/
  spi_xmit_byte(DUMMY_DATA);
  spi_xmit_byte(DUMMY_DATA);
  spi_xmit_byte(DUMMY_DATA);
  return SUCCESS;
}

U8 sd_multiple_write (const U8 *buf, U32 sector, U32 count)
{   
  U8            SdBuffer[36];
  U8            response;
  
  SET_SD_SPI_1M;
  SD_Clock = SEC(10);
  spi_xmit_command(APP_CMD, STUFF_BITS, DUMMY_CRC);
  do
  {
    sd_command_response(R1_RESPONSE,SdBuffer);
    if( !SD_Clock ) 
      return ERROR;
  }
  while((SdBuffer[0] != IN_IDLE_STATE) && (SdBuffer[0] != SUCCESS));
  spi_xmit_byte(DUMMY_DATA);      // 8 clock

  spi_xmit_command(0x40+23, count, DUMMY_CRC);
  do
  {
    sd_command_response(R1_RESPONSE, SdBuffer);
    if( !SD_Clock ) return ERROR;
  }
  while(SdBuffer[0] != SUCCESS);  
  sd_command_response(R3_RESPONSE,SdBuffer);
  spi_xmit_byte(DUMMY_DATA);   // 8 clock
  
  spi_xmit_command(WRITE_MULTIPLE_BLOCK, sector, DUMMY_CRC);
  do
  {
    sd_command_response(R1_RESPONSE, SdBuffer);
    if( !SD_Clock ) return ERROR;
  }
  while(SdBuffer[0] != SUCCESS);  
  sd_command_response(R3_RESPONSE,SdBuffer);
  spi_xmit_byte(DUMMY_DATA);   // 8 clock
  
  for (U32 i=0; i<count; i++)
  {
    WDRCLR;
    do
    {
      response = spi_xmit_byte(DUMMY_DATA);
      if( !SD_Clock ) 
        return ERROR;
    } while(response != DUMMY_DATA);
    
    
    spi_xmit_byte(0xFC);
    for (U16 j=0; j<512; j++)      
      spi_xmit_byte (buf[i*512+j]);
    spi_xmit_byte(DUMMY_DATA);
    spi_xmit_byte(DUMMY_DATA);
    do
    {
      response  =  spi_xmit_byte(DUMMY_DATA);	
      response &=  0x0F;
      if((response == CRC_ERROR) || (response == WRITE_ERROR) || !SD_Clock) 
        return ERROR;
    } while(response != DATA_ACCEPTED);
  }
  
  do
  {
    response = spi_xmit_byte(DUMMY_DATA);
    if( !SD_Clock ) 
      return ERROR;
  } while(response != DUMMY_DATA);
  
  spi_xmit_byte(0xFD);
  
  spi_xmit_byte(DUMMY_DATA);
  
   do
  {
    response = spi_xmit_byte(DUMMY_DATA);
    if( !SD_Clock ) 
      return ERROR;
  } while(response != DUMMY_DATA);
  
  return SUCCESS;
}

