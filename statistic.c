#include  "define.h"
#include "fatfs/ff.h"
#include "diskio.h"

extern FIL f;
extern FATFS fs;

const U32       zeroLongMark = 0;

void usb_notify ();
void convertUnixTimeToDate(U32 t, DateTime *date);

#if defined(__7M2_PRA_MSD_USER)
unsigned char open_stat_file (unsigned char create_new);
#endif

void writeStatFlash ()
{
  U8            h, bytenum = 0;
  S8            p;
  U8            wbuff[32];
  S16           int10Val;
  U16           writeShift, unint10Val;
           
  if( StartStatistic >= 0x10000 )               writeShift = 0;
  else
  {
    if((WriteAdr + StartStatistic) >= 0xfe00)   writeShift = 512;
    else                                        writeShift = 0;
  }
  
  U32       t;
  _DINT(); t = Time; _EINT();
  memcpy(&wbuff[bytenum], (pInt8U)&t, 4); 
  bytenum += 4;
             
  if( Errors & TS_ERROR )       int10Val    = 0xffff;
  else                          int10Val    = round(Tempr * 10);
  memcpy(&wbuff[bytenum], (pInt8U)&int10Val, 2);
  bytenum += 2;
  
  if( IS_DEV_IVG )
  {
    if( Errors & HS_ERROR )     int10Val = 0xffff;
    else                        int10Val = round(Humidy * 10);
      
    memcpy(&wbuff[bytenum], (pInt8U)&int10Val, 2);
    bytenum += 2;
  }
  else
  {
    if( Errors & HS_ERROR )       h = 0xff;
    else                          h = round(Humidy);
    if( Errors & PRESS_ERROR )    p = 0xff;
    else                          
    {
      p = RelPress;
      if( p == (-1) )    p = 0; 
    }
    
    unint10Val = h | ((U16)p << 8);
    memcpy(&wbuff[bytenum], (pInt8U)&unint10Val, 2);
    bytenum += 2;
      
    if( IS_DEV_TNS )
    {
      if( Errors & TS2_ERROR )  int10Val    = 0xffff;
      else                      int10Val    = round(TemprBlack * 10);
      memcpy(&wbuff[bytenum], (pInt8U)&int10Val, 2);
      bytenum += 8;
    }
  } 
  
  // Инфоблок
  if( InfoTime <= t )
    {
      InfoTime    = t + HALF_ADAY;
      memcpy(&wbuff[bytenum],  (pInt8U)&zeroLongMark, 4);
      bytenum += 4;
      memcpy(&wbuff[bytenum], (pInt8U)&PowerShow, 2);
      bytenum += 2;
      memcpy(&wbuff[bytenum], (pInt8U)&Errors, 2);
      bytenum += 2;
      if( IS_DEV_TNS ) { bytenum += 8; }
  }
    
  // запись
  WriteAdr += WriteFlashString(WriteAdr + StartStatistic + writeShift, wbuff, bytenum);    
           
  if((WriteAdr + StartStatistic + writeShift) >= FLASHENDADR)
  {
      WriteAdr    = 0;
      Errors     |= STAT_OVR;
  }
}

void  statistic_process(void)
{
  U8 errsd   = 0;
  Time2Write = WritePeriod; 
  
  // проверка на режимы где статистика не пишется
  if( ISNOSTAT_MODE || (Errors & TIME_ERROR) )
  {
    return;
  }
    
  switch( ConfWordCopy & DEVTYPE_MASK )
  {       
    /* ######################  4  ######################### */ 
    case M4_TYPE:
    case M4L_TYPE: 
    /* ######################  4  ######################### */
  
    /* ######################  M1, 2, 3, 7  ######################### */    
    case M1_TYPE:
    case M2_TYPE:
    case M3_TYPE:      
    case M7_TYPE:  
    case K7_TYPE:
    case K1P_TYPE:
                    writeStatFlash ();
                    if (usb_on_off) usb_notify ();
                    break;
    /* ######################  M1, 2, 3, 7  ######################### */
    
    /* ######################  M6  ######################### */
    case M6_TYPE:      
#if defined(__7M2_PRA_MSD_USER)    
    if( SD_Flags.sd_InsertedAndValid )
    {      
      // запитываем и инциализируем карту
      if (usb_on_off==0)
      {
        SD_ON;
        SD_CS_HIGH;
        DELAY_USB_MS(15);
        
        sd_spi_clear();        
        errsd  = sd_initialization();        
        sd_spi_clear();
        
      }
      
      // проверить файл статистики 
      if ( (usb_on_off == 1) && (usb_write_data == 1) )
      {
        usb_write_data = 0;
        if ( open_stat_file (0) ) errsd = 1;
      }
      
      // карта норм - пишем статистику в файл
      if( !errsd )
      {        
        UINT bw;
        char fatfs_buffer[TEXT_SIZE];

        uint16_t end=sprintf_current_data (fatfs_buffer);
        end = (end>0)?end:0;
        
        for (Int8U i=end; i<TEXT_SIZE-2; i++) fatfs_buffer[i]=' ';
	fatfs_buffer[TEXT_SIZE-2]='\r';
        fatfs_buffer[TEXT_SIZE-1]='\n';
        f_lseek (&f, f_size (&f));
        int res=f_write (&f, fatfs_buffer, TEXT_SIZE, &bw);
        if (res!=FR_OK) 
        {
          errsd=1;
        }
        f_sync (&f);                
        
        // Инфоблок
        if( InfoTime <= Time )
        {
          InfoTime    = Time + HALF_ADAY;
          int end=sprintf (fatfs_buffer, "\t\t\t\t\tBattery:%d\tError:0x%04lX", PowerShow, Errors);
          end=(end>0)?end:0;
          for (Int8U i=end; i<TEXT_SIZE-2; i++) fatfs_buffer[i]=' ';
          fatfs_buffer[TEXT_SIZE-2]='\r';
          fatfs_buffer[TEXT_SIZE-1]='\n';
          f_lseek (&f, f_size (&f));
          int res=f_write (&f, fatfs_buffer, TEXT_SIZE, &bw);
          if (res!=FR_OK) 
          {
            errsd=1;
          }
        }                
      }
        
      if( !errsd )
      {
        WriteAdr++;
        WriteAdr|=0x80000000; // метка записи на карту - адрес не используется
      }            
      else
      {
        SD_Flags.sd_InsertedAndValid = 0;    // переинициализироват карту в sd_process()    
      }
    
      if (usb_on_off==0)
      {
        SD_CS_HIGH;
        DELAY_USB_MS(0.5);
        SD_OFF;
      }
    }
    else  // !SD_Flags.sd_InsertedAndValid
    {
      if (WriteAdr & 0x80000000) WriteAdr = 0;  // метка записи на флеш + инициация адреса
      writeStatFlash ();                        // заись на флеш при отсутствии карты или при неисправной карте
    }
    
    if (usb_on_off) usb_notify ();
#endif    
    break;
    /* ######################  M6  ######################### */
    
    default:        ;
  }  
}
