#ifndef __DEVICE_STRUCT_
#define __DEVICE_STRUCT_
typedef struct 
{ 
   char    up;
   char    *strings_p;
}  list_sruct;                 //menu

typedef struct 
{ 
   U8    HCSD:        1;
   U8    Version:     1;
   U8    data_sector: 1;
   U8    sd_inserted: 1;   
   U8    sd_InsertedAndValid: 1;
   U8    sd_NeedToInitialize: 1;
}  sd_flags_struct;           // SD и её параметры

typedef struct 
{ 
   Int16U    timeout:       1;
   Int16U    systick:       1; 
   Int16U    measure_ready: 1;  
   Int16U    DefMemWrite:   1;   
   
   Int16U    uart_receive:  1;   
   Int16U    respeeduart:   1;
   Int16U    lcd_update:    1;        
   Int16U    key:           1;
   
   Int16U    fromsleep:     1;
   Int16U    writeinfo:     1;
   
   Int16U    TransmitOn:    1;
   Int16U    TransmitDone:  1;
   Int16U    ReceiveDone:   1;
   
   Int16U    LPS22HB:       1;      
   Int16U    PressGPa:      1;
} Flags_t;
#endif