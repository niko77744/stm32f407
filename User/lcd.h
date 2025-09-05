#ifndef __LCD_H__
#define __LCD_H__

#include "main.h"

// LCD��������
extern uint16_t lcd_id;        // LCD ID
extern uint8_t dir_flag;       // ���������������ƣ�0��������1��������
extern uint16_t lcd_width;     // LCD ���
extern uint16_t lcd_height;    // LCD �߶�
extern uint16_t write_gramcmd; // дgramָ��
extern uint16_t setxcmd;       // ����x����ָ��
extern uint16_t setycmd;       // ����y����ָ��

// LCD�Ļ�����ɫ�ͱ���ɫ
extern uint16_t BRUSH_COLOR; // Ĭ�Ϻ�ɫ
extern uint16_t BACK_COLOR;  // ������ɫ.Ĭ��Ϊ��ɫ

//////////////////////////////////////////////////////////////////////////////////
//-----------------LCD����˿ڶ���----------------

// A12��Ϊ��������������  ����ʱSTM32�ڲ�������һλ����
#define CMD_BASE ((uint32_t)(0x6C000000 | 0x00001FFE))
#define DATA_BASE ((uint32_t)(0x6C000000 | 0x00002000))

#define LCD_CMD (*(uint16_t *)CMD_BASE)
#define LCD_DATA (*(uint16_t *)DATA_BASE)

// ɨ�跽����
#define L2R_U2D 0 // ������,���ϵ���
#define L2R_D2U 1 // ������,���µ���
#define R2L_U2D 2 // ���ҵ���,���ϵ���
#define R2L_D2U 3 // ���ҵ���,���µ���

#define U2D_L2R 4 // ���ϵ���,������
#define U2D_R2L 5 // ���ϵ���,���ҵ���
#define D2U_L2R 6 // ���µ���,������
#define D2U_R2L 7 // ���µ���,���ҵ���

// ��ɫֵ����
#define WHITE 0xFFFF
#define BLACK 0x0000
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define BROWN 0XBC40
#define BRRED 0XFC07
#define GRAY 0X8430
#define MAGENTA 0xF81F
#define CYAN 0x7FFF
#define YELLOW 0xFFE0

void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_Value);
uint16_t LCD_ReadReg(uint16_t LCD_Reg);
void LCD_WriteGRAM(void);

void LCD_Init(void);                                              // ��ʼ��
void LCD_DisplayOn(void);                                         // ����ʾ
void LCD_DisplayOff(void);                                        // ����ʾ
void LCD_Clear(uint16_t Color);                                   // ����
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);                 // ���ù��
void LCD_DrawPoint(uint16_t x, uint16_t y);                       // ����
void LCD_Color_DrawPoint(uint16_t x, uint16_t y, uint16_t color); // ��ɫ����
uint16_t LCD_GetPoint(uint16_t x, uint16_t y);                    // ����

void LCD_Open_Window(uint16_t X0, uint16_t Y0, uint16_t width, uint16_t height);
void Set_Scan_Direction(uint8_t direction);
void Set_Display_Mode(uint8_t mode);

void LCD_Fill_onecolor(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color);                                                        // ��䵥����ɫ
void LCD_Draw_Picture(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);                                                        // ���ָ����ɫ
void LCD_DisplayChar(uint16_t x, uint16_t y, uint8_t word, uint8_t size);                                                                          // ��ʾһ���ַ�
void LCD_DisplayString(uint16_t x, uint16_t y, uint8_t size, uint8_t *p);                                                                          // ��ʾһ��12/16/24�����ַ���
void LCD_DisplayString_color(uint16_t x, uint16_t y, uint8_t size, uint8_t *p, uint16_t brushcolor, uint16_t backcolor);                           // ��ʾһ��12/16/24�����Զ�����ɫ���ַ���
void LCD_DisplayNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode);                                                // ��ʾ ����
void LCD_DisplayNum_color(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode, uint16_t brushcolor, uint16_t backcolor); // ��ʾ�Զ�����ɫ����

// ����ASCII����������ĸ
// ƫ����32���ո�֮���ַ�
// ���ֿ������п�֪��ÿ�����嵥���ַ���ռ���ֽ���

#endif /* __LCD_H__ */
