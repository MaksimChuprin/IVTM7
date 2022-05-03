#include  "define.h"
#include  "modbus.h"

Int8U  hex2char(pU8 buffer, Int16U pointer)
{
  Int8U a, b;
     
  a = buffer[pointer++] - 48;
  if(a > 9) a -= 7;
           
  b = buffer[pointer] - 48;
  if(b > 9) b -= 7;
           
  return((a << 4) | b);
}

Int16U  hex2int(pU8 buffer, Int16U pointer)
{
  return((hex2char(buffer, pointer)<<8) | hex2char(buffer, pointer+2));
}

Int16U  char2hex(Int8U cnum)
{
  Int8U  a,b;
      
  a= (cnum & 0x0f);
  b= cnum>>4;
      
  if(a > 9) a += 55;
  else      a += 48;
          
  if(b > 9) b += 55;
  else      b += 48;
          
  return(a | (b<<8));
}

// check sum 
Int8U check_sum(pU8 buffer, Int16U len)
{
  Int8U    crc = 0;
     
  for(U16 i = 0; i < len; i++) crc += buffer[i];
  return crc;
}

void  hexbuffer_2_binbuffer(pInt8U hexBuffer, pInt8U binBuffer, Int16U nBytes, Int16U Pointer)
{
  Int16U i;
  
  for(i= 0; i < nBytes; i++, Pointer+= 2) binBuffer[i]= hex2char(hexBuffer, Pointer);
}

void  binbuffer_2_hexbuffer(pInt8U hexBuffer, Int8U __data20* binBuffer, Int16U nBytes, Int16U Pointer)
{
  Int16U        codedbyte;
  
  for(U16 i = 0; i < nBytes; i++) 
  {
    codedbyte =  char2hex(binBuffer[i]);
    hexBuffer[Pointer++]= codedbyte >> 8;
    hexBuffer[Pointer++]= codedbyte;
  }
}

U16 GetParameterValueForMODBUSHoldReg(U16 parameter_number, Int8U * buffer)
{
  uint32_t serial=0;
  for (int i=0; i<7; i++)
  {
    serial += (uint32_t) SerialNumberStr[i] - '0';
    serial *= 10L;
  }
  serial += (uint32_t) SerialNumberStr[7] - '0';

  uint8_t *data = (uint8_t *) &serial;
  if(parameter_number < 2)
  {    
    buffer[1] = data[parameter_number*2];    
    buffer[0] = data[parameter_number*2+1];    
  }
  else return 1;
  
  return 0;
}

U16 GetParameterValueForMODBUS(U16 parameter_number, Int8U * buffer)
{
  pU8  data_p;
  U16 count;
    
  if(parameter_number < 6)      
  {
    data_p = (pU8)&Tempr;
    buffer[0] = *((pU8)( (U16)data_p + parameter_number * 2 + 1));
    buffer[1] = *((pU8)( (U16)data_p + parameter_number * 2 + 0));    
  }
  else if (parameter_number == 6)    
  {
    buffer[0] = Errors & 0xFF;
    buffer[1] = Errors >> 8;
  }
  else if (parameter_number == 7)
  {
    count = WriteAdr / get_bin_size();
    buffer[0] = count & 0xFF;
    buffer[1] = count >> 8;
  }
  else return 1;
  
  return 0;
}

U16 SetParameterValueForMODBUS(U16 register_adr, U16 preset_value) {return 0;}

#ifdef MODBUS20
Int8U MODBUS_ReadFileRecord (Int8U * buffer, Int16U flash_rec_num, Int16U rec_length)
{    
    Int8U size = 0;
    if (( ConfWordCopy & DEVTYPE_MASK ) != M6_TYPE)
    {
      Int32U realAdr =  StartStatistic + flash_rec_num * get_bin_size();
      U8 bytenum = rec_length * 2;// * get_bin_size();
      
      if (((flash_rec_num*get_bin_size()+bytenum)<=WriteAdr) ||
        (Errors&STAT_OVR))      
          if((realAdr + bytenum) <  (FLASHENDADR)) 
          {
            binbuffer_2_binbuffer( (Int8U __data20 *)(realAdr), buffer, bytenum);
            size = bytenum;
          }
    }
    return size;
}
#endif

U16  EKSIS_ASCII(pU8 buffer)
{  
  U8       num;
  U16      adr, d, c, command;
  U32      realAdr;
          
  // проверка адреса
  adr       =   hex2int(buffer,   1);
  if( (adr != NetAdr) && (adr != SERVICE_ADR) ) return 0;
   
  adr       =   hex2int(buffer,   7);
  num       =   hex2char(buffer, 11);
  command   =   buffer[5] * 256 + buffer[6];
       
  buffer[0] =  '!';
  c         =   7;
  
  switch(command) 
  {
    case 'WI': if(num > MAX_DATA_LEN) goto fail;
               hexbuffer_2_binbuffer(buffer, &buffer[7], num, 13);
               Flags.respeeduart = 1;
               if(WriteFlashString(adr, &buffer[7], num))  goto final;              
               goto fail;
               
    case 'WR': if((num > MAX_DATA_LEN) || ((adr + num) > RAMSIZE) ) goto fail;
               hexbuffer_2_binbuffer(buffer, (pInt8U)(RamBeginAddr + adr), num, 13);
               if( Errors & RESET_STAT )
               {
                Errors     &= ~(RESET_STAT + STAT_OVR);
                WriteAdr    =  0;
                Time2Write  =  60;
               }
               goto final;
               
    case 'WD': if(num > MAX_DATA_LEN) goto fail; 
    
               if( StartDescriptor >= 0x10000 )               realAdr = 0;
               else
               {
                  if((adr + StartDescriptor) >= 0xfe00)       realAdr = 512;
                  else                                        realAdr = 0;
               }
               realAdr += StartDescriptor + adr;
                              
               if((realAdr + num) <=  (FLASHENDADR))  
               {
                 hexbuffer_2_binbuffer(buffer, &buffer[7], num, 13);
                 if(WriteFlashString(adr, &buffer[7], num))  goto final;              
                 else                                        goto fail;
               }
               else goto fail;               
                                             
    case 'RI': if(num > MAX_DATA_LEN) goto fail;
               binbuffer_2_hexbuffer(buffer,(Int8U __data20 *)(adr), num, 7);
               c = 7 + num * 2;
               goto final;
               
    case 'RR': if(num > MAX_DATA_LEN) goto fail;                              
               binbuffer_2_hexbuffer(buffer,(Int8U __data20 *)(RamBeginAddr + adr), num, 7);
               c = 7 + num * 2;
               goto final;
               
    case 'RF': if(num > MAX_DATA_LEN) goto fail;   
                    
               if( StartStatistic >= 0x10000 )               realAdr = 0;
               else
               {
                  if((adr + StartStatistic) >= 0xfe00)       realAdr = 512;
                  else                                       realAdr = 0;
               }
               realAdr += StartStatistic + adr + PageAdr;
               
               if((realAdr + num) <=  (FLASHENDADR))  binbuffer_2_hexbuffer(buffer, (Int8U __data20 *)(realAdr), num, 7);
               else goto fail;
               
               c = 7 + num * 2;
               goto final;
               
    case 'RD': if(num > MAX_DATA_LEN) goto fail;
    
               if( StartDescriptor >= 0x10000 )              realAdr = 0;
               else
               {
                if((adr + StartDescriptor) >= 0xfe00)        realAdr = 512;
                else                                         realAdr = 0;
               }
               realAdr += StartDescriptor + adr;
                  
               if((realAdr + num) <=  (FLASHENDADR))  binbuffer_2_hexbuffer(buffer, (Int8U __data20 *)(realAdr), num, 7);
               else goto fail;
                  
               c = 7 + num * 2;
               goto final;               
                                                             
    case 'IR': for(U8 i = 0; i < 8; i++) buffer[c++] = SerialNumberStr[i];
               for(U8 i = 0; ID[i]; i++) buffer[c++] = ID[i];
               goto final; 
                              
    case 'DW': Flags.DefMemWrite = 1;
               goto final;               
                                                    
fail:
     default:  buffer[0]    =   '?'; 
final: 
               d            =   char2hex(check_sum(buffer, c)); 
               buffer[c++]  =   d >> 8; 
               buffer[c++]  =   d; 
               buffer[c++]  =   0x0d; 
  }
  
  return c;
}

U8 checkEKSIS(pU8 buffer)
{
  if( (buffer[0] != '$') || (RxCounter < 10) )          return 0;               // не ЭКСИС-протокол
  
  for(U16 i = 1; i < RxCounter; i++)
  {
    if( buffer[i] == 0x0d )                             return ( i > 8 ? i : 0);
    if( (buffer[i] < 0x21) || (buffer[i] > 0x7A) )      return 0;               // не ЭКСИС-протокол
  }
  return 0;
}

/* ############################################# */
void  slave_uart_process(void)
{
  Flags.uart_receive = 0; 
        
  RxCounter = checkEKSIS(UartBuffer);
    
  if( RxCounter )
  {      
    if ( check_sum(UartBuffer, RxCounter - 2) == hex2char(UartBuffer, RxCounter - 2) )          TxByte2send =  EKSIS_ASCII(UartBuffer );
    else                                                                                        TxByte2send =  MODBUS_RTU (UartBuffer, NetAdr);        
  }
  else                                                                                          TxByte2send =  MODBUS_RTU (UartBuffer, NetAdr);
  
  RxCounter = 0;
    
  if( !TxByte2send )
  {
    memset(UartBuffer, 0, 12);
    RXI_ENABLE;
  }
  else 
  { 
    // запуск прибора при запросе
    if( ISSLEEP_MODE )
    {                  
      KeyCode   = SW2;
      key_process(); 
    }
    
    RS_TX_EN;
    SET_U0_2SB;
    TX_START;
  }
  return;
}  

/* ###################################################################### */
