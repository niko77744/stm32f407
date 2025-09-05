#ifndef __XTP_2046_H__
#define __XTP_2046_H__

#include "main.h"

extern uint8_t CMD_RDX;
extern uint8_t CMD_RDY;

// ������оƬ��������
#define PEN PFin(11)  // T_PEN
#define DOUT PBin(2)  // T_MISO
#define TDIN PBout(1) // T_MOSI
#define TCLK PAout(5) // T_CLK
#define TCS PBout(0)  // T_CS

// ����������������
extern uint16_t Xdown;
extern uint16_t Ydown; // �����������¾ͷ��صĵ�����ֵ
extern uint16_t Xup;
extern uint16_t Yup; // ������������֮��̧�𷵻صĵ�����ֵ

extern float xFactor; // ������У׼����(����������ҪУ׼)
extern float yFactor;
extern short xOffset;
extern short yOffset;

// ����������
/*********������SPIͨ�����ȡ������ADֵ*********************/
void SPI_Write_Byte(uint8_t num);                  // �����оƬд��һ������
uint16_t SPI_Read_AD(uint8_t CMD);                 // ��ȡADת��ֵ
uint16_t RTouch_Read_XorY(uint8_t xy);             // ���˲��������ȡ(X/Y)
uint8_t RTouch_Read_XY(uint16_t *x, uint16_t *y);  // ˫�����ȡ(X+Y)
uint8_t RTouch_Read_XY2(uint16_t *x, uint16_t *y); // ���ζ�ȡ��˫���������ȡ

/*********��������ʼ������*********************/
void XPT2046_Init(void); // ��ʼ��

/*********������ɨ�谴������*********************/
void XPT2046_Scan(uint8_t tp); // tp:0,��Ļ����;1,��������

#endif /* __XTP_2046_H__ */
