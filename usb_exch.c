#include  "define.h"
#include "diskio.h"
#include "fatfs/ff.h"

#define ASK_USBANS                buffer[0]
#define BYTENUM_USB               buffer[1]

#define WR_USB                    0x00
#define WF_USB                    0x01
#define WI_USB                    0x02
#define WP_USB                    0x03
#define WD_USB                    0x04

#define RR_USB                    0x80
#define RF_USB                    0x81
#define RI_USB                    0x82
#define RP_USB                    0x83
#define RD_USB                    0x84

#define IR_USB                    0x8F
#define BS_USB                    0x55
#define DW_USB                    0x56


void    debug_send_data(Int8U len, pInt8U pBuffer) {}
void    debug_send_string(const Int8U *  pBuffer)  {}

#if defined(__7M2_PRA_MSD_USER)
extern FIL f;
#endif

void read_record (uint32_t record_num, uint8_t *text);

void usb_exchange(pInt8U buffer)
{
  Int32U          addr, realAdr;
  Int8U           bytenum, command, i, k, check_sum;
  
  addr=           buffer[0] + ((Int32U)buffer[1] << 8) + ((Int32U)buffer[2] << 16) + ((Int32U)buffer[3] << 24);
  command=        buffer[4];
  bytenum=        buffer[5];
  
  if(bytenum > USB_MAX_DATA_LEN) goto fail;
  
  switch(command)
  {
      case WR_USB:      
      case WI_USB:          
      case WD_USB:
                    for (i = 0, check_sum = 0xff; i < (6 + bytenum); i++) check_sum += buffer[i];
                    check_sum = (check_sum != buffer[6 + bytenum]) ? 1 : 0;
                    break;
      
      case RR_USB:
      case RF_USB:      
      case RI_USB:
      case IR_USB:      
      case DW_USB:         
      case RD_USB:
                    for (i = 0, check_sum = 0xff; i < 6; i++) check_sum += buffer[i];
                    check_sum = (check_sum != buffer[6]) ? 1 : 0;
                    break;
                    
      case BS_USB:  check_sum = 0;
                    break;
      
  default:          check_sum = 1;
  }
  
  if(check_sum) goto fail;
  
  switch(command)
  {                        
    // WR
    case 0x00:    if( (bytenum > USB_WDATA_LEN) || ((addr + bytenum) > RAMSIZE) ) goto fail;
                  binbuffer_2_binbuffer((Int8U __data20 *)&buffer[6], (Int8U __data20 *)(RamBeginAddr + addr), bytenum);
                  if( Errors & RESET_STAT )
                  {
                    Errors         &=  ~(RESET_STAT + STAT_OVR + SD_ERROR);
                    WriteAdr        =  0;
                    Time2Write      =  60;
                  }
                  
                  ASK_USBANS = ASK_USBEXCH;                                    
                  for (BYTENUM_USB = 0, i = 0, buffer[2] = 0xff; i < 2; i++) buffer[2] += buffer[i];
                  break;
    // WI
    case 0x02:    if(bytenum > USB_WDATA_LEN)  goto fail; 
                  if(!WriteFlashString(addr, &buffer[6], bytenum))  goto fail;
                  
                  Flags.respeeduart = 1;
                  ASK_USBANS = ASK_USBEXCH;                                    
                  for (BYTENUM_USB = 0, i = 0, buffer[2] = 0xff; i < 2; i++) buffer[2] += buffer[i];
                  break;                  
                  
    // WD
    case 0x04:    if(bytenum > USB_WDATA_LEN)  goto fail; 
    
                  if( StartDescriptor >= 0x10000 )               realAdr = 0;
                  else
                  {
                    if((addr + StartDescriptor) >= 0xfe00)       realAdr = 512;
                    else                                         realAdr = 0;
                  }
                  realAdr += StartDescriptor + addr;
                  
                  if((realAdr + bytenum) <=  (FLASHENDADR))  
                  {
                      if(!WriteFlashString(realAdr, &buffer[6], bytenum))  goto fail;
                  }
                  else goto fail;
                                                      
                  Flags.respeeduart = 1;
                  ASK_USBANS = ASK_USBEXCH;                                    
                  for (BYTENUM_USB = 0, i = 0, buffer[2] = 0xff; i < 2; i++) buffer[2] += buffer[i];
                  break;                  
    // RR             
    case 0x80:    if(bytenum > USB_RDATA_LEN)  goto fail;
                  if((addr + bytenum) <=  RAMSIZE) binbuffer_2_binbuffer( (Int8U __data20 *)(RamBeginAddr + addr), (Int8U __data20 *)&buffer[2], bytenum);
                  else goto fail;
                  
                  ASK_USBANS = ASK_USBEXCH;                                    
                  for (BYTENUM_USB = bytenum, i = 0, buffer[2 + bytenum] = 0xff; i < 2 + bytenum; i++) buffer[2 + bytenum] += buffer[i];
                  break; 
                  
    // RF             
    case 0x81:    if(bytenum > USB_RDATA_LEN)  goto fail; 
                  switch( ConfWordCopy & DEVTYPE_MASK )
                  {                
                case M1_TYPE:
                case M2_TYPE:
                case M3_TYPE:
                case M4_TYPE:
                case M4L_TYPE:
                case M7_TYPE:
                case K7_TYPE:
                case K1P_TYPE:
                    if( Time2Write < 10 )                         Time2Write = 10;
                    
                    if( StartStatistic >= 0x10000 )               realAdr = 0;
                    else
                    {
                      if((addr + StartStatistic) >= 0xfe00)       realAdr = 512;
                      else                                        realAdr = 0;
                    }
                    realAdr += StartStatistic + addr;
                    
                    if((realAdr + bytenum) <=  (FLASHENDADR))  binbuffer_2_binbuffer( (Int8U __data20 *)(realAdr), (Int8U __data20 *)&buffer[2], bytenum);
                    else goto fail;
                  
                    ASK_USBANS = ASK_USBEXCH;                                    
                    for (BYTENUM_USB = bytenum, i = 0, buffer[2 + bytenum] = 0xff; i < 2 + bytenum; i++) buffer[2 + bytenum] += buffer[i];                  
                    break;
                    
                case M6_TYPE:
#if defined(__7M2_PRA_MSD_USER)
                    if( Time2Write < 30 ) Time2Write = 30;  // отложить запись
                    
                    if( !SD_Flags.sd_InsertedAndValid ) 
                    {
                      if( StartStatistic >= 0x10000 )               realAdr = 0;
                      else
                      {
                        if((addr + StartStatistic) >= 0xfe00)       realAdr = 512;
                        else                                        realAdr = 0;
                      }
                      realAdr += StartStatistic + addr;
                    
                      if((realAdr + bytenum) <=  (FLASHENDADR))  binbuffer_2_binbuffer( (Int8U __data20 *)(realAdr), (Int8U __data20 *)&buffer[2], bytenum);
                      else goto fail;                     
                    }
                    else
                    {
                      f_lseek (&f, addr * TEXT_SIZE);
                      f_gets (buffer+2, TEXT_SIZE, &f);
                    }
                    bytenum    = TEXT_SIZE;
                    ASK_USBANS = ASK_USBEXCH;                                    
                    for (BYTENUM_USB = bytenum, i = 0, buffer[2 + bytenum] = 0xff; i < 2 + bytenum; i++) buffer[2 + bytenum] += buffer[i];                  
#endif
                    break;
                    
                  default: goto fail;
                  }
                  break;
                                    
    // RI
    case 0x82:    if(bytenum > USB_RDATA_LEN)  goto fail;
                  if((addr + bytenum) <= INFOENDADR) binbuffer_2_binbuffer( (Int8U __data20 *)(addr), (Int8U __data20 *)&buffer[2], bytenum);
                  else goto fail;
                  
                  ASK_USBANS = ASK_USBEXCH;                                    
                  for (BYTENUM_USB = bytenum, i = 0, buffer[2 + bytenum] = 0xff; i < 2 + bytenum; i++) buffer[2 + bytenum] += buffer[i];                  
                  break; 
                  
    // RD
    case 0x84:    if(bytenum > USB_RDATA_LEN)  goto fail;
                  if( StartDescriptor >= 0x10000 )               realAdr = 0;
                  else
                  {
                    if((addr + StartDescriptor) >= 0xfe00)       realAdr = 512;
                    else                                         realAdr = 0;
                  }
                  realAdr += StartDescriptor + addr;
                  
                  if((realAdr + bytenum) <=  (FLASHENDADR))  binbuffer_2_binbuffer( (Int8U __data20 *)(realAdr), (Int8U __data20 *)&buffer[2], bytenum);
                  else goto fail;
                  
                  ASK_USBANS = ASK_USBEXCH;                                    
                  for (BYTENUM_USB = bytenum, i = 0, buffer[2 + bytenum] = 0xff; i < 2 + bytenum; i++) buffer[2 + bytenum] += buffer[i];                  
                  break;
                  
    // IR
    case 0x8F:    for (i = 0, k = 2; ID[i]; i++) buffer[k++] = ID[i];
                  buffer[k++] = ' ';
                                    
                  for (i = 0; i < 8; i++) buffer[k++] = SerialNumberStr[i];
                  buffer[k++] = ' ';
                                                                        
                  ASK_USBANS = ASK_USBEXCH;                                    
                  for (BYTENUM_USB = k - 2, i = 0, buffer[k] = 0xff; i < k; i++) buffer[k] += buffer[i];
                  break;
    // BS
    case 0x55:         
                  __delay_cycles(100000);
                  BOOT_RESET;
                  break;
    // DW                  
    case 0x56:    Flags.DefMemWrite = 1;
                  ASK_USBANS = ASK_USBEXCH;
                  for (BYTENUM_USB = 0, i = 0, buffer[2] = 0xff; i < 2; i++) buffer[2] += buffer[i];
                  break;
      
fail:             
    default:      ASK_USBANS = NAK_USBEXCH;
  }
  SleepClock = SleepPeriod;
}