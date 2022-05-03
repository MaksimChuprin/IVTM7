#include  "define.h"

static void bt_exchange(U8 *buffer)
{
  Int32U          addr, realAdr;
  Int8U           bytenum, command, i, k, check_sum;
  
  addr=           buffer[0] + ((Int32U)buffer[1] << 8) + ((Int32U)buffer[2] << 16) + ((Int32U)buffer[3] << 24);
  command=        buffer[4];
  bytenum=        buffer[5];
  
  if(bytenum > BT_MAX_DATA_LEN) goto fail;
  
  switch(command)
  {
      case WR_BT:      
      case WI_BT:          
                    for (i = 0, check_sum = 0xff; i < (6 + bytenum); i++) check_sum += buffer[i];
                    check_sum = (check_sum != buffer[6 + bytenum]) ? 1 : 0;
                    break;
      
      case RR_BT:
      case RF_BT:      
      case RI_BT:
      case IR_BT:
      case RD_BT:      
                    for (i = 0, check_sum = 0xff; i < 6; i++) check_sum += buffer[i];
                    check_sum = (check_sum != buffer[6]) ? 1 : 0;
                    break;
                          
  default:          check_sum = 1;
  }
  
  if(check_sum) goto fail;
  
  switch(command)
  {                        
    case WR_BT:   if( (addr + bytenum) > RAMSIZE ) goto fail;
                  memcpy((void *)(RamBeginAddr + addr), &buffer[6], bytenum);
                  if( Errors & RESET_STAT )
                  {
                    Errors     &= ~(RESET_STAT + STAT_OVR);
                    WriteAdr    =  0;
                    Time2Write  =  60;
                  }
                  
                  buffer[0] = ASK_BTEXCH;
                  for (buffer[1] = 0, i = 0, buffer[2] = 0xff; i < 2; i++) buffer[2] += buffer[i];
                  break;
                  
    case WI_BT:   if(!WriteFlashString(addr, &buffer[6], bytenum))  goto fail;
                  
                  buffer[0] = ASK_BTEXCH; 
                  for (buffer[1] = 0, i = 0, buffer[2] = 0xff; i < 2; i++) buffer[2] += buffer[i];
                  break;                  
       
    case RR_BT:   if((addr + bytenum) <=  RAMSIZE) memcpy( &buffer[2], (void *)(RamBeginAddr + addr), bytenum);
                  else goto fail;
                  
                  buffer[0] = ASK_BTEXCH; 
                  for (buffer[1] = bytenum, i = 0, buffer[2 + bytenum] = 0xff; i < 2 + bytenum; i++) buffer[2 + bytenum] += buffer[i];
                  break; 
                         
    case RF_BT:   if( StartStatistic >= 0x10000 )               realAdr = 0;
                  else
                  {
                      if((addr + StartStatistic) >= 0xfe00)     realAdr = 512;
                      else                                      realAdr = 0;
                  }
                  realAdr += StartStatistic + addr;
                  
                  if((realAdr + bytenum) <=  (FLASHENDADR)) binbuffer_2_binbuffer( (Int8U __data20 *)(realAdr), (Int8U __data20 *)&buffer[2], bytenum);
                  else goto fail;
   
                  buffer[0] = ASK_BTEXCH;
                  for (buffer[1] = bytenum, i = 0, buffer[2 + bytenum] = 0xff; i < 2 + bytenum; i++) buffer[2 + bytenum] += buffer[i];                  
                  break;
                  
    case RI_BT:   if((addr + bytenum) <=  INFOENDADR) memcpy( &buffer[2], (void *)(addr), bytenum); 
                  else goto fail;
                  
                  buffer[0] = ASK_BTEXCH;                                    
                  for (buffer[1] = bytenum, i = 0, buffer[2 + bytenum] = 0xff; i < 2 + bytenum; i++) buffer[2 + bytenum] += buffer[i];                  
                  break;
                  
    case RD_BT:   if( StartDescriptor >= 0x10000 )               realAdr = 0;
                  else
                  {
                    if((addr + StartDescriptor) >= 0xfe00)       realAdr = 512;
                    else                                         realAdr = 0;
                  }
                  realAdr += StartDescriptor + addr;
                  
                  if((realAdr + bytenum) <=  (FLASHENDADR))  binbuffer_2_binbuffer( (Int8U __data20 *)(realAdr), (Int8U __data20 *)&buffer[2], bytenum);
                  else goto fail;
                  
                  buffer[0] = ASK_BTEXCH;                                    
                  for (buffer[1] = bytenum, i = 0, buffer[2 + bytenum] = 0xff; i < 2 + bytenum; i++) buffer[2 + bytenum] += buffer[i];                   
                  break;    
                  
    case IR_BT:   for (i = 0, k = 2; ID[i]; i++) buffer[k++] = ID[i];
                  buffer[k++] = ' ';
                                    
                  for (i = 0; i< 8; i++) buffer[k++] = SerialNumberStr[i];
                  buffer[k++] = ' ';
                  
                  k += sprintf( &buffer[k], "VID=%04x PID=%04x", USBD_VID, USBD_PID) + 1;  
                  
                  buffer[0] = ASK_BTEXCH;                                    
                  for (buffer[1] = k - 2, i = 0, buffer[k] = 0xff; i < k; i++) buffer[k] += buffer[i];                  
                  break;                  
                                 
    case DW_BT:   Flags.DefMemWrite= 1;
    
                  buffer[0] = ASK_BTEXCH;                                    
                  for (buffer[1] = 0, i = 0, buffer[2] = 0xff; i < 2; i++) buffer[2] += buffer[i];
                  break;                                    
                  
fail:             
    default:      buffer[0] = NAK_BTEXCH; buffer[1] = 0;
  }  
}

void bt_process(void)
{
  U8            p, SerialNumber[9], type;
        
  Flags.uart_receive = 0;
  
  switch( BtStat )
  {
    case 0:     p = (strstr(UartBuffer, "AT-AB") == NULL); 
                if( p )
                {
                  BT_OFF;
                  Errors |= BT_ERROR;
                  return;
                }
                Errors &= ~BT_ERROR;                
                SerialNumber[8] = 0;
                type = (ConfWordCopy & PRES) ? '7':'6'; 
                memset(SerialNumber, 0, 9);
                memcpy(SerialNumber, SerialNumberStr, 8);
                TxByte2send = sprintf(UartBuffer, "AT+AB DefaultLocalName EKSIS-00%c-%s\r\n\0", type, SerialNumber);  // Devname
                TX_START;
                BtStat   = 1;
                break;               
                
    case 1:     p = (strstr(UartBuffer, "Ok") == NULL) && (strstr(UartBuffer, "OK") == NULL); 
                if( p )
                {
                  BT_OFF;
                  Errors |= BT_ERROR;                  
                  return;
                }                         
                TxByte2send = sprintf(UartBuffer, "AT+AB Config Var30=010100\r\n\0");  // COD
                TX_START;
                BtStat   = 2;
                break;
                
    case 2:     p = (strstr(UartBuffer, "Ok") == NULL) && (strstr(UartBuffer, "OK") == NULL); 
                if( p )
                {
                  BT_OFF;
                  Errors |= BT_ERROR;                  
                  return;
                }                                
                TxByte2send = sprintf(UartBuffer, "AT+AB Config Var40=1\r\n\0"); // DefaultSecurity
                TX_START;
                BtStat   = 3;
                break;

    case 3:     p = (strstr(UartBuffer, "Ok") == NULL) && (strstr(UartBuffer, "OK") == NULL); 
                if( p )
                {
                  BT_OFF;
                  Errors |= BT_ERROR;                  
                  return;
                }
                TxByte2send = sprintf(UartBuffer, "AT+AB Config Var41=4\r\n\0"); // DefaultAuth 
                TX_START;
                BtStat   = 4;
                break;                
                
    case 4:     p = (strstr(UartBuffer, "Ok") == NULL) && (strstr(UartBuffer, "OK") == NULL); 
                if( p )
                {
                  BT_OFF;
                  Errors |= BT_ERROR;                  
                  return;
                }                                
                TxByte2send = sprintf(UartBuffer, "AT+AB Config Var6=0000\r\n\0");  // PIN
                TX_START;
                BtStat   = 5;
                break;                                
                
    case 5:     p = (strstr(UartBuffer, "Ok") == NULL) && (strstr(UartBuffer, "OK") == NULL); 
                if( p )
                {
                  BT_OFF;
                  Errors |= BT_ERROR;                  
                  return;
                }                
                RXI_ENABLE;
                BtStat    = 6;
                BlueClock = BtActivePeriod;
                break;

    case 6:     if( UartBuffer[0] != '!' )  { RXI_ENABLE; break; }      
                bt_exchange(&UartBuffer[2]);
                memcpy( (void *)&UartBuffer[1], (void *)&UartBuffer[2], UartBuffer[3] + 3);
                TxByte2send = UartBuffer[2] + 4;
                TX_START;
                BlueClock = BtActivePeriod;
                break;
  }
}

/* ###################################################################### */


