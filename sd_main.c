#include  "define.h"
#include "fatfs/ff.h"
#include "diskio.h"

FIL f;
FATFS fs;
#pragma data_alignment = 2
static char fatfs_buffer[512];

void convertUnixTimeToDate(U32 t, DateTime *date);
unsigned char USBMSC_checkMSCInsertionRemoval (void);

unsigned char open_stat_file (unsigned char create_new)
{
  char filename[13]="12345678.xls";
  
  //Имя текущего файла
  for (uint8_t i=0; i<8; i++) filename[i]=SerialNumberStr[i];
  
  //Пытаемся смонтировать том
  if (f_mount (&fs, "", 1)!=FR_OK)
  {    
    //Если не получилось - форматируем
    if (f_mkfs("", FM_FAT32, 0, (char *)fatfs_buffer, sizeof (fatfs_buffer))!=FR_OK) 
    { 
      //Не получилось отформатировать
      printf ("ErrSd");
      DELAY_USB_MS (1000);
      return 1; 
    }
    else
    {
    //Если получилось отформатировать, то пытаемся смонтировать еще раз
      if (f_mount (&fs, "", 1)!=FR_OK) 
      {
        printf ("ErrSd");
        DELAY_USB_MS (1000);
        return 1;
      }
    }
    create_new = 1;
  }
  //Проверяем метку тома. Должна совпадать с серийным номером
  f_getlabel("", fatfs_buffer, 0);
  for (uint8_t i=0; i<8; i++)
    if (fatfs_buffer[i]!=SerialNumberStr[i])
    {
      f_setlabel(SerialNumberStr);
      break;
    }

  //Если не нужно создавать новый файл, то пытаемся открыть существующий
  if (create_new == 0)
  {
    if (f_open (&f, filename, FA_OPEN_EXISTING | FA_WRITE | FA_READ)==FR_OK) return 0;
  }
  
  //Иначе переименовываем существующий файл
  if (f_open (&f, filename, FA_READ)==FR_OK)
  {
    char serial[10];
    f_gets (serial, 10, &f);
    f_close (&f);
    DateTime dt;
    U32       t;
    _DINT(); t = Time; _EINT();
    convertUnixTimeToDate (t, &dt);    
    serial[8]=0;
    sprintf (fatfs_buffer, "%s %02d-%02d-%04d %02d-%02d-%02d.xls", serial, dt.day, dt.month, dt.year, dt.hours, dt.minutes, dt.seconds);  
    f_rename ((const TCHAR *) filename, (const TCHAR *) fatfs_buffer);
  }

  //Создаем новый
  if (f_open (&f, filename, FA_CREATE_ALWAYS | FA_WRITE | FA_READ)!=FR_OK) 
  { 
    printf ("ErrSd");
    DELAY_USB_MS (1000);
    return 1; 
  }
  if (f_sync (&f)!=FR_OK) 
  {  
    printf ("ErrSd");
    DELAY_USB_MS (1000);
    return 1;
  }
  return 0;
}

float u32_to_f (U32 data)
{
  U32 *p=&data;
  F32 *f=(F32 *)p;
  return *f;
}

void  sd_process(void)
{   
  uint16_t end;  
  
  if( Errors & (PW_ERROR + INFOMEM_ERROR) )  return;
      
  SD_ON;
  DELAY_USB_MS(3);
  
  if(CHECK_SD_PRES)  
  { 
    SD_Flags.sd_inserted = 1;  
    Errors              |= SD_IN; 
  }
  else               
  { 
    if (SD_Flags.sd_InsertedAndValid) { printf("OutSd"); DELAY_USB_MS (1000); }
    SD_Flags.sd_inserted =  SD_Flags.sd_InsertedAndValid = 0; 
    Errors              &= ~(SD_ERROR + SD_IN); 
  } 
  
  USBMSC_checkMSCInsertionRemoval ();
  
  if( !SD_Flags.sd_inserted )   { SD_OFF; return; }
  
  if( Errors & SD_ERROR )       { SD_OFF; return; }
    
  if( SD_Flags.sd_InsertedAndValid )   
  { 
    if   (usb_on_off == 0)      { SD_OFF; return; }

    if( SD_Flags.sd_NeedToInitialize )
    {
        SD_Flags.sd_NeedToInitialize = 0;
        // очистка SPI интерфейса карты        
        sd_spi_clear();          
        // инициализируем карту 
        if( sd_initialization() == ERROR )  goto sd_err;
        // очистка SPI интерфейса карты 
        sd_spi_clear();  
    }
    return; 
  }
            
  // инициализация карты, тома, фаты...
  
  // очистка SPI интерфейса карты
  SD_CS_HIGH;
  DELAY_USB_MS(12);
  sd_spi_clear();  
  
  // инициализируем карту   
  if( sd_initialization() == ERROR )  goto sd_err;
  // очистка SPI интерфейса карты
  sd_spi_clear();  
    
  printf ("BusSD");
  
  //Пытаемся смотировать том и создать новый файл
  if ( open_stat_file (1) ) goto sd_err;
  
  if (IS_DEV_TNS)       end=sprintf (fatfs_buffer, "%s\r\nTime\tTemp\tHumidy\tPressure\tTempB", SerialNumberStr);
  else                  end=sprintf (fatfs_buffer, "%s\r\nTime\tTemp\tHumidy\tPressure",        SerialNumberStr);
  end=(end>0)?end:0;
  for (Int8U i=end; i<TEXT_SIZE-2; i++) fatfs_buffer[i]=' ';  
  fatfs_buffer[TEXT_SIZE-2]='\r'; 
  fatfs_buffer[TEXT_SIZE-1]='\n';  
  UINT bw;
  if (f_write (&f, fatfs_buffer, TEXT_SIZE, &bw)!=FR_OK) { goto sd_err; }
  if (f_sync (&f)!=FR_OK)                                { goto sd_err; }
  
  //Если до этого момента были записи во флэш
  if (((WriteAdr&0x7FFFFFFF)!=0)&&((WriteAdr&0x80000000)==0))
  {
    U32         adr = 0;
    U8          text[TEXT_SIZE];
    
    //Копируем из флэш в файл
    while (adr<WriteAdr)
    {
      WDRCLR;
      printf ("r%4d", adr/get_bin_size());
      read_record (adr/get_bin_size() + 1, text);
      if (f_write (&f, text, TEXT_SIZE, &bw)!=FR_OK) { goto sd_err; }
      adr+=get_bin_size();
    }
    if (f_sync (&f)!=FR_OK) { goto sd_err; }
    WriteAdr /= get_bin_size();
    WriteAdr |= 0x80000000L;
  }
  else WriteAdr=0x80000000L;
  
  printf (" InSd");
  DELAY_USB_MS (1000);
      
  SD_Flags.sd_InsertedAndValid = 1;
  Time2Write                   = 30;   
  return;

sd_err:  
  SD_Flags.sd_InsertedAndValid = 0;
  Errors                      |= SD_ERROR;  
  SD_OFF;
}
