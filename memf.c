#include  "define.h"

#pragma data_alignment = 2
static U8 FlashBuffer[512];

void  binbuffer_2_binbuffer(Int8U __data20 * pBufferSource, Int8U __data20 * pBufferDestination, Int16U nBytes)
{
  Int16U i;
  for(i= 0; i < nBytes; i++) pBufferDestination[i] = pBufferSource[i];
}

Int16U calcCHK16(pInt8U buffer, Int16U len)
{
  Int16U  i, crc16;
  
  for(crc16 = 0, i = 0; i < len; i++) 
  { 
    crc16+= *buffer; 
    buffer++; 
  }
  
  return crc16;
}

Int16U writeFlash(Int32U writeAdr, pInt8U buffer, Int16U num)
{
  if( FCTL3 & LOCKA )   FCTL3 = FWKEY + LOCKA;                                  // unlock write
  else                  FCTL3 = FWKEY;
  
  for(U16 i = 0; i < num; i++, writeAdr++) 
  {
    if((writeAdr >= 0xfe00) && (writeAdr < 0x10000))           return 0; // защита сегмента векторов прерываний
    if((writeAdr >= 0x9000) && (writeAdr < StartDescriptor))   return 0; // защита кода
    
    if( ((writeAdr % SEG_SIZE) == 0) || (writeAdr == SEG_A)
         || (writeAdr == SEG_B) || (writeAdr == SEG_C) || (writeAdr == SEG_D) )
    {
      FCTL1= FWKEY + ERASE;
      *((Int8U __data20 *)writeAdr)= 0xff;
    }
    if(buffer[i] != 0xff)
    {
      FCTL1 = FWKEY + WRT;
      *((Int8U __data20 *)writeAdr) = buffer[i];
      FCTL1 = FWKEY;
    }
    if(*((Int8U __data20 *)writeAdr) != buffer[i]) 
    { 
      FCTL3 = FWKEY + LOCK + LOCKA; 
      return 0; 
    }
  }
  
  FCTL3 = FWKEY + LOCK + LOCKA;  // lock write
  return num; 
}

Int16U WriteFlashString(Int32U adr, pInt8U bufpoint, Int16U len)
{
  Int32U        startAdr, crc;
  Int16U        writeNum, shift;
  ldiv_t        n;

  
  if((adr >= SEG_D) && (adr <= (SEG_D + DataSizeSegD - len)))
  {  
    writeNum=  DataSizeSegD;
    startAdr=  SEG_D;
  }
  else if((adr >= SEG_C) && (adr <= (SEG_C + DataSizeSegC - len)))
  {
    writeNum= DataSizeSegC;
    startAdr=  SEG_C;
  }
  else if((adr >= SEG_B) && (adr <= (SEG_B + 128 - len)) && Flags.DefMemWrite)
  {
    writeNum=  128;
    startAdr=  SEG_B;
  }
  else if((adr >= SEG_A) && (adr <= (SEG_A + 128 - len)) && Flags.DefMemWrite)
  {
    writeNum=  128;
    startAdr=  SEG_A;
  }
  else 
  {
    n = ldiv(adr, 512);
    startAdr = n.quot * 512;
    writeNum = 512;
    len = ((len + n.rem) > 512) ? (512 - n.rem) : len;
    Flags.DefMemWrite = 1;      // не считать CRC 
  }
  
  binbuffer_2_binbuffer((Int8U __data20 *)startAdr, FlashBuffer, writeNum);
  shift = adr - startAdr;
  binbuffer_2_binbuffer((Int8U __data20 *)bufpoint, &FlashBuffer[shift], len);
  
  if( !Flags.DefMemWrite ) 
  { 
    crc = calcCHK16(FlashBuffer, writeNum); 
    FlashBuffer[writeNum] = crc; FlashBuffer[writeNum + 1] = crc >> 8; 
    writeNum += 2;
  }
  
  Flags.DefMemWrite = 0;
  return (writeFlash(startAdr, FlashBuffer, writeNum) == writeNum) ? len : 0;
}

void CheckInfoFlash(void)
{ 
  if(Errors & INFOMEM_ERROR) return;
  
  if(DataSizeSegD && (DataSizeSegD < 127))
  {
    if(calcCHK16((pInt8U)SEG_D, DataSizeSegD) != CheckSumD)
    {
      WriteFlashString(SEG_D, (pInt8U)SEG_B, DataSizeSegD); 
      if(calcCHK16((pInt8U)SEG_D, DataSizeSegD) != CheckSumD) Errors |= INFOMEM_ERROR;
    }
  }
  
  if(DataSizeSegC && (DataSizeSegC < 127))
  {
    if(calcCHK16((pInt8U)SEG_C, DataSizeSegC) != CheckSumC)
    {
      WriteFlashString(SEG_C, (pInt8U)SEG_A, DataSizeSegC); 
      if(calcCHK16((pInt8U)SEG_C, DataSizeSegC) != CheckSumC) Errors |= INFOMEM_ERROR;
    }
  }      
}
