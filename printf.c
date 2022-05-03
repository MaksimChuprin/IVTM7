#include "define.h"

// LCD definition
#define   r_CHAR      16 
#define   o_CHAR      17
#define   MINUS_C     18
#define   LOW_LINE_C  19
#define   SPACE_C     20
#define   POINT_C     21
#define   n_CHAR      22
#define   S_CHAR      23
#define   t_CHAR      24
#define   U_CHAR      25
#define   Y_CHAR      26
#define   L_CHAR      27
#define   l_CHAR      28
#define   P_CHAR      29
#define   u_CHAR      30
#define   H_CHAR      31
#define   N_CHAR      32
#define   h_CHAR      33
#define   RP_CHAR     34
#define   c_CHAR      35
#define   b_CHAR      36
#define   R_CHAR      37
#define   d_CHAR      38

const char code_gen[][4] = { 
0x11,0x11,0x11,0x00,   // 0
0x10,0x01,0x00,0x00,   // 1
0x11,0x10,0x01,0x01,   // 2
0x11,0x11,0x00,0x01,   // 3
0x10,0x01,0x10,0x01,   // 4
0x01,0x11,0x10,0x01,   // 5
0x01,0x11,0x11,0x01,   // 6
0x11,0x01,0x00,0x00,   // 7
0x11,0x11,0x11,0x01,   // 8
0x11,0x11,0x10,0x01,   // 9
0x11,0x01,0x11,0x01,   // A
0x00,0x11,0x11,0x01,   // b
0x01,0x10,0x11,0x00,   // C
0x10,0x11,0x01,0x01,   // d
0x01,0x10,0x11,0x01,   // E
0x01,0x00,0x11,0x01,   // F
0x00,0x00,0x01,0x01,   // r
0x00,0x11,0x01,0x01,   // o
0x00,0x00,0x00,0x01,   // -
0x00,0x10,0x00,0x00,   // _
0x00,0x00,0x00,0x00,   // ' '
0x00,0x00,0x00,0x10,   // .
0x00,0x01,0x01,0x01,   // n
0x01,0x11,0x10,0x01,   // S
0x00,0x10,0x11,0x01,   // t
0x10,0x11,0x11,0x00,   // U
0x10,0x11,0x10,0x01,   // Y
0x00,0x10,0x11,0x00,   // L
0x00,0x00,0x11,0x00,   // l
0x11,0x00,0x11,0x01,   // P
0x00,0x11,0x01,0x00,   // u
0x10,0x01,0x11,0x01,   // H
0x11,0x01,0x11,0x00,   // N
0x00,0x01,0x11,0x01,   // h
0x11,0x01,0x11,0x00,   // Ï
0x00,0x10,0x01,0x01,   // c
0x00,0x11,0x11,0x01,   // b
0x01,0x00,0x11,0x00,   // R
0x10,0x11,0x01,0x01,   // d
};
                                      
static char  Lcd_pointer;
                                      
void outchar(char p, char c)
{  
  if(c=='.') 
  { 
    if( (LCD_SIZE == 4) && (p == 3) );
    else if( (LCD_SIZE == 5) && (p == 4) );
    else *(LCDMEM + p*4 + 3)= *(LCDMEM + p*4 + 3) | 0x10; 
  }
  else for(U8 i = 0; i < 4; i++) *(LCDMEM + p*4 + i) = code_gen[c][i];                  
}

void lcd_clear(void)
{  
  for(U8 i = 0; i < 26; i++) *(LCDMEM + i) = 0x00;
  Lcd_pointer = 0;
}

int putchar(int c)
{  
  if((c < 58) && (c > 47))  outchar(Lcd_pointer++,(c-48));
  else
  switch(c) 
  {
    case ' ': outchar(Lcd_pointer++,SPACE_C);
              break;
    case '.': outchar((Lcd_pointer - 1),'.');
              break;
    case '-': outchar(Lcd_pointer++,MINUS_C);
              break;
    case '_': outchar(Lcd_pointer++,LOW_LINE_C);
              break;
    case 'a': 
    case 'A': outchar(Lcd_pointer++,0x0A);
              break;
              
    case 'B': outchar(Lcd_pointer++,0x0B);
              break;
              
    case 'b': outchar(Lcd_pointer++, b_CHAR);
              break;
      
    case 'c': outchar(Lcd_pointer++, c_CHAR);
              break;
                
    case 'C': outchar(Lcd_pointer++,0x0C);
              break;
    case 'D': outchar(Lcd_pointer++,0x0D);
              break;
              
    case 'd': outchar(Lcd_pointer++, d_CHAR);
              break;
                
    case 'e':
    case 'E': outchar(Lcd_pointer++,0x0E);
              break;
    case 'f':
    case 'F': outchar(Lcd_pointer++,0x0F);
              break;
              
    case 'O': outchar(Lcd_pointer++,0x00);
              break;
              
    case 'o': outchar(Lcd_pointer++,o_CHAR);
              break;
              
    case 'R': outchar(Lcd_pointer++,R_CHAR);
              break;
    case 'r': outchar(Lcd_pointer++,r_CHAR);
              break;
              
    case 'N': outchar(Lcd_pointer++,N_CHAR);
              break;         
              
    case 'n': outchar(Lcd_pointer++,n_CHAR);
              break;
    case 'T':
    case 't': outchar(Lcd_pointer++,t_CHAR);
              break;                     
    case 'U': outchar(Lcd_pointer++,U_CHAR);                  
              break;
    case 'u': outchar(Lcd_pointer++,u_CHAR);
              break;
    case 's':
    case 'S': outchar(Lcd_pointer++,0x05);
              break;
    case 'Y': outchar(Lcd_pointer++,Y_CHAR);
              break;
    case 'L': outchar(Lcd_pointer++,L_CHAR);
              break;
    case 'i':
    case 'I':
    case 'l': outchar(Lcd_pointer++,l_CHAR);
              break;
    case 'p':
    case 'P': outchar(Lcd_pointer++,P_CHAR);
              break;
    case 'H': outchar(Lcd_pointer++,H_CHAR);
              break;
    case 'h': outchar(Lcd_pointer++,h_CHAR);
              break;
    case 'Ï': outchar(Lcd_pointer++,RP_CHAR);
              break;
  }
  if(Lcd_pointer == LCD_SIZE) Lcd_pointer = 0;
  return c;
}


/*
static void put_one_char(char c, void *dummy)
{
  putchar (c);
  (void)dummy;  
}


int printf(const char *format, ...)                    // Our main entry 
{
  va_list ap;
  int nr_of_chars;

  Lcd_pointer= 0;
  va_start(ap, format);      // Variable argument begin 
  nr_of_chars = _formatted_write(format, put_one_char, (void *) 0, ap);
  va_end(ap);                // Variable argument end 
  return nr_of_chars;        // According to ANSI 
}
*/
