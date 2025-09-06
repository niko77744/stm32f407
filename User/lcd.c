#define LOG_TAG "lcd"
#include "lcd.h"
#include "elog.h"
#include "./font/font.h"
#if 0

SRAM_HandleTypeDef g_sram_handle; /* SRAM���(���ڿ���LCD) */

/* LCD�Ļ�����ɫ�ͱ���ɫ */
uint32_t g_point_color = 0xF800; /* ������ɫ */
uint32_t g_back_color = 0xFFFF;  /* ����ɫ */

/* ����LCD��Ҫ���� */
_lcd_dev lcddev;

/**
 * @brief       LCDд����
 * @param       data: Ҫд�������
 * @retval      ��
 */
void lcd_wr_data(volatile uint16_t data)
{
    data = data; /* ʹ��-O2�Ż���ʱ��,����������ʱ */
    LCD->LCD_RAM = data;
}

/**
 * @brief       LCDд�Ĵ������/��ַ����
 * @param       regno: �Ĵ������/��ַ
 * @retval      ��
 */
void lcd_wr_regno(volatile uint16_t regno)
{
    regno = regno;        /* ʹ��-O2�Ż���ʱ��,����������ʱ */
    LCD->LCD_REG = regno; /* д��Ҫд�ļĴ������ */
}

/**
 * @brief       LCDд�Ĵ���
 * @param       regno:�Ĵ������/��ַ
 * @param       data:Ҫд�������
 * @retval      ��
 */
void lcd_write_reg(uint16_t regno, uint16_t data)
{
    LCD->LCD_REG = regno; /* д��Ҫд�ļĴ������ */
    LCD->LCD_RAM = data;  /* д������ */
}

/**
 * @brief       LCD��ʱ����,�����ڲ�����mdk -O1ʱ���Ż�ʱ��Ҫ���õĵط�
 * @param       t:��ʱ����ֵ
 * @retval      ��
 */
static void lcd_opt_delay(uint32_t i)
{
    while (i--)
        ; /* ʹ��AC6ʱ��ѭ�����ܱ��Ż�,��ʹ��while(1) __asm volatile(""); */
}

/**
 * @brief       LCD������
 * @param       ��
 * @retval      ��ȡ��������
 */
static uint16_t lcd_rd_data(void)
{
    volatile uint16_t ram; /* ��ֹ���Ż� */
    lcd_opt_delay(2);
    ram = LCD->LCD_RAM;
    return ram;
}

/**
 * @brief       ׼��дGRAM
 * @param       ��
 * @retval      ��
 */
void lcd_write_ram_prepare(void)
{
    LCD->LCD_REG = lcddev.wramcmd;
}

/**
 * @brief       ��ȡ��ĳ�����ɫֵ
 * @param       x,y:����
 * @retval      �˵����ɫ(32λ��ɫ,�������LTDC)
 */
uint32_t lcd_read_point(uint16_t x, uint16_t y)
{
    uint16_t r = 0, g = 0, b = 0;

    if (x >= lcddev.width || y >= lcddev.height)
    {
        return 0; /* �����˷�Χ,ֱ�ӷ��� */
    }

    lcd_set_cursor(x, y); /* �������� */

    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(0x2E00); /* 5510 ���Ͷ�GRAMָ�� */
    }
    else
    {
        lcd_wr_regno(0x2E); /* 9341/5310/1963/7789/7796/9806 �ȷ��Ͷ�GRAMָ�� */
    }

    r = lcd_rd_data(); /* �ٶ�(dummy read) */

    if (lcddev.id == 0x1963)
    {
        return r; /* 1963ֱ�Ӷ��Ϳ��� */
    }

    r = lcd_rd_data(); /* ʵ��������ɫ */

    if (lcddev.id == 0x7796) /* 7796 һ�ζ�ȡһ������ֵ */
    {
        return r;
    }

    /* 9341/5310/5510/7789/9806Ҫ��2�ζ��� */
    b = lcd_rd_data();
    g = r & 0xFF; /* ����9341/5310/5510/7789/9806,��һ�ζ�ȡ����RG��ֵ,R��ǰ,G�ں�,��ռ8λ */
    g <<= 8;

    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11)); /* ILI9341/NT35310/NT35510/ST7789/ILI9806��Ҫ��ʽת��һ�� */
}

/**
 * @brief       LCD������ʾ
 * @param       ��
 * @retval      ��
 */
void lcd_display_on(void)
{
    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(0x2900); /* ������ʾ */
    }
    else /* 9341/5310/1963/7789/7796/9806 �ȷ��Ϳ�����ʾָ�� */
    {
        lcd_wr_regno(0x29); /* ������ʾ */
    }
}

/**
 * @brief       LCD�ر���ʾ
 * @param       ��
 * @retval      ��
 */
void lcd_display_off(void)
{
    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(0x2800); /* �ر���ʾ */
    }
    else /* 9341/5310/1963/7789/7796/9806 �ȷ��͹ر���ʾָ�� */
    {
        lcd_wr_regno(0x28); /* �ر���ʾ */
    }
}

/**
 * @brief       ���ù��λ��(��RGB����Ч)
 * @param       x,y: ����
 * @retval      ��
 */
void lcd_set_cursor(uint16_t x, uint16_t y)
{
    if (lcddev.id == 0x1963)
    {
        if (lcddev.dir == 0) /* ����ģʽ, x������Ҫ�任 */
        {
            x = lcddev.width - 1 - x;
            lcd_wr_regno(lcddev.setxcmd);
            lcd_wr_data(0);
            lcd_wr_data(0);
            lcd_wr_data(x >> 8);
            lcd_wr_data(x & 0xFF);
        }
        else /* ����ģʽ */
        {
            lcd_wr_regno(lcddev.setxcmd);
            lcd_wr_data(x >> 8);
            lcd_wr_data(x & 0xFF);
            lcd_wr_data((lcddev.width - 1) >> 8);
            lcd_wr_data((lcddev.width - 1) & 0xFF);
        }

        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_data(y & 0xFF);
        lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_data((lcddev.height - 1) & 0xFF);
    }
    else if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(x >> 8);
        lcd_wr_regno(lcddev.setxcmd + 1);
        lcd_wr_data(x & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_regno(lcddev.setycmd + 1);
        lcd_wr_data(y & 0xFF);
    }
    else /* 9341/5310/7789/7796/9806 �� �������� */
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(x >> 8);
        lcd_wr_data(x & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_data(y & 0xFF);
    }
}

/**
 * @brief       ����LCD���Զ�ɨ�跽��(��RGB����Ч)
 *   @note
 *              9341/5310/5510/1963/7789/7796/9806��IC�Ѿ�ʵ�ʲ���
 *              ע��:�����������ܻ��ܵ��˺������õ�Ӱ��(������9341),
 *              ����,һ������ΪL2R_U2D����,�������Ϊ����ɨ�跽ʽ,���ܵ�����ʾ������.
 *
 * @param       dir:0~7,����8������(���嶨���lcd.h)
 * @retval      ��
 */
void lcd_scan_dir(uint8_t dir)
{
    uint16_t regval = 0;
    uint16_t dirreg = 0;
    uint16_t temp;

    /* ����ʱ����1963���ı�ɨ�跽������ʱ1963�ı䷽��(���������1963�����⴦��,����������IC��Ч) */
    if ((lcddev.dir == 1 && lcddev.id != 0x1963) || (lcddev.dir == 0 && lcddev.id == 0x1963))
    {
        switch (dir) /* ����ת�� */
        {
        case 0:
            dir = 6;
            break;

        case 1:
            dir = 7;
            break;

        case 2:
            dir = 4;
            break;

        case 3:
            dir = 5;
            break;

        case 4:
            dir = 1;
            break;

        case 5:
            dir = 0;
            break;

        case 6:
            dir = 3;
            break;

        case 7:
            dir = 2;
            break;
        }
    }

    /* ����ɨ�跽ʽ ���� 0x36/0x3600 �Ĵ��� bit 5,6,7 λ��ֵ */
    switch (dir)
    {
    case L2R_U2D: /* ������,���ϵ��� */
        regval |= (0 << 7) | (0 << 6) | (0 << 5);
        break;

    case L2R_D2U: /* ������,���µ��� */
        regval |= (1 << 7) | (0 << 6) | (0 << 5);
        break;

    case R2L_U2D: /* ���ҵ���,���ϵ��� */
        regval |= (0 << 7) | (1 << 6) | (0 << 5);
        break;

    case R2L_D2U: /* ���ҵ���,���µ��� */
        regval |= (1 << 7) | (1 << 6) | (0 << 5);
        break;

    case U2D_L2R: /* ���ϵ���,������ */
        regval |= (0 << 7) | (0 << 6) | (1 << 5);
        break;

    case U2D_R2L: /* ���ϵ���,���ҵ��� */
        regval |= (0 << 7) | (1 << 6) | (1 << 5);
        break;

    case D2U_L2R: /* ���µ���,������ */
        regval |= (1 << 7) | (0 << 6) | (1 << 5);
        break;

    case D2U_R2L: /* ���µ���,���ҵ��� */
        regval |= (1 << 7) | (1 << 6) | (1 << 5);
        break;
    }

    dirreg = 0x36; /* �Ծ��󲿷�����IC, ��0x36�Ĵ������� */

    if (lcddev.id == 0x5510)
    {
        dirreg = 0x3600; /* ����5510, ����������ic�ļĴ����в��� */
    }

    /* 9341 & 7789 & 7796 Ҫ����BGRλ */
    if (lcddev.id == 0x9341 || lcddev.id == 0x7789 || lcddev.id == 0x7796)
    {
        regval |= 0x08;
    }

    lcd_write_reg(dirreg, regval);

    if (lcddev.id != 0x1963) /* 1963�������괦�� */
    {
        if (regval & 0x20)
        {
            if (lcddev.width < lcddev.height) /* ����X,Y */
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
        else
        {
            if (lcddev.width > lcddev.height) /* ����X,Y */
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
    }

    /* ������ʾ����(����)��С */
    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setxcmd + 1);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setxcmd + 2);
        lcd_wr_data((lcddev.width - 1) >> 8);
        lcd_wr_regno(lcddev.setxcmd + 3);
        lcd_wr_data((lcddev.width - 1) & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setycmd + 1);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setycmd + 2);
        lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_regno(lcddev.setycmd + 3);
        lcd_wr_data((lcddev.height - 1) & 0xFF);
    }
    else
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(0);
        lcd_wr_data(0);
        lcd_wr_data((lcddev.width - 1) >> 8);
        lcd_wr_data((lcddev.width - 1) & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(0);
        lcd_wr_data(0);
        lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_data((lcddev.height - 1) & 0xFF);
    }
}

/**
 * @brief       ����
 * @param       x,y: ����
 * @param       color: �����ɫ(32λ��ɫ,�������LTDC)
 * @retval      ��
 */
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color)
{
    lcd_set_cursor(x, y);    /* ���ù��λ�� */
    lcd_write_ram_prepare(); /* ��ʼд��GRAM */
    LCD->LCD_RAM = color;
}

/**
 * @brief       SSD1963�����������ú���
 * @param       pwm: ����ȼ�,0~100.Խ��Խ��.
 * @retval      ��
 */
void lcd_ssd_backlight_set(uint8_t pwm)
{
    lcd_wr_regno(0xBE);      /* ����PWM��� */
    lcd_wr_data(0x05);       /* 1����PWMƵ�� */
    lcd_wr_data(pwm * 2.55); /* 2����PWMռ�ձ� */
    lcd_wr_data(0x01);       /* 3����C */
    lcd_wr_data(0xFF);       /* 4����D */
    lcd_wr_data(0x00);       /* 5����E */
    lcd_wr_data(0x00);       /* 6����F */
}

/**
 * @brief       ����LCD��ʾ����
 * @param       dir:0,����; 1,����
 * @retval      ��
 */
void lcd_display_dir(uint8_t dir)
{
    lcddev.dir = dir; /* ����/���� */

    if (dir == 0) /* ���� */
    {
        lcddev.width = 240;
        lcddev.height = 320;

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0x2C00;
            lcddev.setxcmd = 0x2A00;
            lcddev.setycmd = 0x2B00;
            lcddev.width = 480;
            lcddev.height = 800;
        }
        else if (lcddev.id == 0x1963)
        {
            lcddev.wramcmd = 0x2C; /* ����д��GRAM��ָ�� */
            lcddev.setxcmd = 0x2B; /* ����дX����ָ�� */
            lcddev.setycmd = 0x2A; /* ����дY����ָ�� */
            lcddev.width = 480;    /* ���ÿ��480 */
            lcddev.height = 800;   /* ���ø߶�800 */
        }
        else /* ����IC, ����: 9341/5310/7789/7796/9806��IC */
        {
            lcddev.wramcmd = 0x2C;
            lcddev.setxcmd = 0x2A;
            lcddev.setycmd = 0x2B;
        }

        if (lcddev.id == 0x5310 || lcddev.id == 0x7796) /* �����5310/7796 ���ʾ�� 320*480�ֱ��� */
        {
            lcddev.width = 320;
            lcddev.height = 480;
        }

        if (lcddev.id == 0X9806) /* �����9806 ���ʾ�� 480*800 �ֱ��� */
        {
            lcddev.width = 480;
            lcddev.height = 800;
        }
    }
    else /* ���� */
    {
        lcddev.width = 320;  /* Ĭ�Ͽ�� */
        lcddev.height = 240; /* Ĭ�ϸ߶� */

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0x2C00;
            lcddev.setxcmd = 0x2A00;
            lcddev.setycmd = 0x2B00;
            lcddev.width = 800;
            lcddev.height = 480;
        }
        else if (lcddev.id == 0x1963 || lcddev.id == 0x9806)
        {
            lcddev.wramcmd = 0x2C; /* ����д��GRAM��ָ�� */
            lcddev.setxcmd = 0x2A; /* ����дX����ָ�� */
            lcddev.setycmd = 0x2B; /* ����дY����ָ�� */
            lcddev.width = 800;    /* ���ÿ��800 */
            lcddev.height = 480;   /* ���ø߶�480 */
        }
        else /* ����IC, ����:9341/5310/7789/7796��IC */
        {
            lcddev.wramcmd = 0x2C;
            lcddev.setxcmd = 0x2A;
            lcddev.setycmd = 0x2B;
        }

        if (lcddev.id == 0x5310 || lcddev.id == 0x7796) /* �����5310/7796 ���ʾ�� 320*480�ֱ��� */
        {
            lcddev.width = 480;
            lcddev.height = 320;
        }
    }

    lcd_scan_dir(DFT_SCAN_DIR); /* Ĭ��ɨ�跽�� */
}

/**
 * @brief       ���ô���(��RGB����Ч), ���Զ����û������굽�������Ͻ�(sx,sy).
 * @param       sx,sy:������ʼ����(���Ͻ�)
 * @param       width,height:���ڿ�Ⱥ͸߶�,�������0!!
 *   @note      �����С:width*height.
 *
 * @retval      ��
 */
void lcd_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    uint16_t twidth, theight;
    twidth = sx + width - 1;
    theight = sy + height - 1;

    if (lcddev.id == 0x1963 && lcddev.dir != 1) /* 1963�������⴦�� */
    {
        sx = lcddev.width - width - sx;
        height = sy + height - 1;
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(sx >> 8);
        lcd_wr_data(sx & 0xFF);
        lcd_wr_data((sx + width - 1) >> 8);
        lcd_wr_data((sx + width - 1) & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(sy >> 8);
        lcd_wr_data(sy & 0xFF);
        lcd_wr_data(height >> 8);
        lcd_wr_data(height & 0xFF);
    }
    else if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(sx >> 8);
        lcd_wr_regno(lcddev.setxcmd + 1);
        lcd_wr_data(sx & 0xFF);
        lcd_wr_regno(lcddev.setxcmd + 2);
        lcd_wr_data(twidth >> 8);
        lcd_wr_regno(lcddev.setxcmd + 3);
        lcd_wr_data(twidth & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(sy >> 8);
        lcd_wr_regno(lcddev.setycmd + 1);
        lcd_wr_data(sy & 0xFF);
        lcd_wr_regno(lcddev.setycmd + 2);
        lcd_wr_data(theight >> 8);
        lcd_wr_regno(lcddev.setycmd + 3);
        lcd_wr_data(theight & 0xFF);
    }
    else /* 9341/5310/7789/1963/7796/9806���� �� ���ô��� */
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(sx >> 8);
        lcd_wr_data(sx & 0xFF);
        lcd_wr_data(twidth >> 8);
        lcd_wr_data(twidth & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(sy >> 8);
        lcd_wr_data(sy & 0xFF);
        lcd_wr_data(theight >> 8);
        lcd_wr_data(theight & 0xFF);
    }
}

/**
 * @brief       ILI9341�Ĵ�����ʼ������
 * @param       ��
 * @retval      ��
 */
void lcd_ex_ili9341_reginit(void)
{
    lcd_wr_regno(0xCF);
    lcd_wr_data(0x00);
    lcd_wr_data(0xC1);
    lcd_wr_data(0X30);
    lcd_wr_regno(0xED);
    lcd_wr_data(0x64);
    lcd_wr_data(0x03);
    lcd_wr_data(0X12);
    lcd_wr_data(0X81);
    lcd_wr_regno(0xE8);
    lcd_wr_data(0x85);
    lcd_wr_data(0x10);
    lcd_wr_data(0x7A);
    lcd_wr_regno(0xCB);
    lcd_wr_data(0x39);
    lcd_wr_data(0x2C);
    lcd_wr_data(0x00);
    lcd_wr_data(0x34);
    lcd_wr_data(0x02);
    lcd_wr_regno(0xF7);
    lcd_wr_data(0x20);
    lcd_wr_regno(0xEA);
    lcd_wr_data(0x00);
    lcd_wr_data(0x00);
    lcd_wr_regno(0xC0); /* Power control */
    lcd_wr_data(0x1B);  /* VRH[5:0] */
    lcd_wr_regno(0xC1); /* Power control */
    lcd_wr_data(0x01);  /* SAP[2:0];BT[3:0] */
    lcd_wr_regno(0xC5); /* VCM control */
    lcd_wr_data(0x30);  /* 3F */
    lcd_wr_data(0x30);  /* 3C */
    lcd_wr_regno(0xC7); /* VCM control2 */
    lcd_wr_data(0XB7);
    lcd_wr_regno(0x36); /* Memory Access Control */
    lcd_wr_data(0x48);
    lcd_wr_regno(0x3A);
    lcd_wr_data(0x55);
    lcd_wr_regno(0xB1);
    lcd_wr_data(0x00);
    lcd_wr_data(0x1A);
    lcd_wr_regno(0xB6); /* Display Function Control */
    lcd_wr_data(0x0A);
    lcd_wr_data(0xA2);
    lcd_wr_regno(0xF2); /* 3Gamma Function Disable */
    lcd_wr_data(0x00);
    lcd_wr_regno(0x26); /* Gamma curve selected */
    lcd_wr_data(0x01);
    lcd_wr_regno(0xE0); /* Set Gamma */
    lcd_wr_data(0x0F);
    lcd_wr_data(0x2A);
    lcd_wr_data(0x28);
    lcd_wr_data(0x08);
    lcd_wr_data(0x0E);
    lcd_wr_data(0x08);
    lcd_wr_data(0x54);
    lcd_wr_data(0XA9);
    lcd_wr_data(0x43);
    lcd_wr_data(0x0A);
    lcd_wr_data(0x0F);
    lcd_wr_data(0x00);
    lcd_wr_data(0x00);
    lcd_wr_data(0x00);
    lcd_wr_data(0x00);
    lcd_wr_regno(0XE1); /* Set Gamma */
    lcd_wr_data(0x00);
    lcd_wr_data(0x15);
    lcd_wr_data(0x17);
    lcd_wr_data(0x07);
    lcd_wr_data(0x11);
    lcd_wr_data(0x06);
    lcd_wr_data(0x2B);
    lcd_wr_data(0x56);
    lcd_wr_data(0x3C);
    lcd_wr_data(0x05);
    lcd_wr_data(0x10);
    lcd_wr_data(0x0F);
    lcd_wr_data(0x3F);
    lcd_wr_data(0x3F);
    lcd_wr_data(0x0F);
    lcd_wr_regno(0x2B);
    lcd_wr_data(0x00);
    lcd_wr_data(0x00);
    lcd_wr_data(0x01);
    lcd_wr_data(0x3f);
    lcd_wr_regno(0x2A);
    lcd_wr_data(0x00);
    lcd_wr_data(0x00);
    lcd_wr_data(0x00);
    lcd_wr_data(0xef);
    lcd_wr_regno(0x11); /* Exit Sleep */
    delay_ms(120);
    lcd_wr_regno(0x29); /* display on */
}

/**
 * @brief       ��ʼ��LCD
 *   @note      �ó�ʼ���������Գ�ʼ�������ͺŵ�LCD(�����.c�ļ���ǰ�������)
 *
 * @param       ��
 * @retval      ��
 */
void lcd_init(void)
{

    /* ����9341 ID�Ķ�ȡ */
    lcd_wr_regno(0xD3);
    lcddev.id = lcd_rd_data(); /* dummy read */
    lcddev.id = lcd_rd_data(); /* ����0x00 */
    lcddev.id = lcd_rd_data(); /* ��ȡ93 */
    lcddev.id <<= 8;
    lcddev.id |= lcd_rd_data(); /* ��ȡ41 */

    if (lcddev.id != 0x9341) /* ���� 9341 , ���Կ����ǲ��� ST7789 */
    {
        lcd_wr_regno(0x04);
        lcddev.id = lcd_rd_data(); /* dummy read */
        lcddev.id = lcd_rd_data(); /* ����0x85 */
        lcddev.id = lcd_rd_data(); /* ��ȡ0x85 */
        lcddev.id <<= 8;
        lcddev.id |= lcd_rd_data(); /* ��ȡ0x52 */

        if (lcddev.id == 0x8552) /* ��8552��IDת����7789 */
        {
            lcddev.id = 0x7789;
        }

        if (lcddev.id != 0x7789) /* Ҳ����ST7789, �����ǲ��� NT35310 */
        {
            lcd_wr_regno(0xD4);
            lcddev.id = lcd_rd_data(); /* dummy read */
            lcddev.id = lcd_rd_data(); /* ����0x01 */
            lcddev.id = lcd_rd_data(); /* ����0x53 */
            lcddev.id <<= 8;
            lcddev.id |= lcd_rd_data(); /* �������0x10 */

            if (lcddev.id != 0x5310) /* Ҳ����NT35310,���Կ����ǲ���ST7796 */
            {
                lcd_wr_regno(0XD3);
                lcddev.id = lcd_rd_data(); /* dummy read */
                lcddev.id = lcd_rd_data(); /* ����0X00 */
                lcddev.id = lcd_rd_data(); /* ��ȡ0X77 */
                lcddev.id <<= 8;
                lcddev.id |= lcd_rd_data(); /* ��ȡ0X96 */

                if (lcddev.id != 0x7796) /* Ҳ����ST7796,���Կ����ǲ���NT35510 */
                {
                    /* ������Կ�������ṩ�� */
                    lcd_write_reg(0xF000, 0x0055);
                    lcd_write_reg(0xF001, 0x00AA);
                    lcd_write_reg(0xF002, 0x0052);
                    lcd_write_reg(0xF003, 0x0008);
                    lcd_write_reg(0xF004, 0x0001);

                    lcd_wr_regno(0xC500);      /* ��ȡID�Ͱ�λ */
                    lcddev.id = lcd_rd_data(); /* ����0x80 */
                    lcddev.id <<= 8;

                    lcd_wr_regno(0xC501);       /* ��ȡID�߰�λ */
                    lcddev.id |= lcd_rd_data(); /* ����0x00 */

                    delay_ms(5); /* �ȴ�5ms, ��Ϊ0XC501ָ���1963��˵���������λָ��, �ȴ�5ms��1963��λ����ٲ��� */

                    if (lcddev.id != 0x5510) /* Ҳ����NT5510,���Կ����ǲ���ILI9806 */
                    {
                        lcd_wr_regno(0XD3);
                        lcddev.id = lcd_rd_data(); /* dummy read */
                        lcddev.id = lcd_rd_data(); /* ����0X00 */
                        lcddev.id = lcd_rd_data(); /* ����0X98 */
                        lcddev.id <<= 8;
                        lcddev.id |= lcd_rd_data(); /* ����0X06 */

                        if (lcddev.id != 0x9806) /* Ҳ����ILI9806,���Կ����ǲ���SSD1963 */
                        {
                            lcd_wr_regno(0xA1);
                            lcddev.id = lcd_rd_data();
                            lcddev.id = lcd_rd_data(); /* ����0x57 */
                            lcddev.id <<= 8;
                            lcddev.id |= lcd_rd_data(); /* ����0x61 */

                            if (lcddev.id == 0x5761)
                                lcddev.id = 0x1963; /* SSD1963���ص�ID��5761H,Ϊ��������,����ǿ������Ϊ1963 */
                        }
                    }
                }
            }
        }
    }

    /* �ر�ע��, �����main�����������δ���1��ʼ��, ��Ῠ����printf
     * ����(������f_putc����), ����, �����ʼ������1, �������ε�����
     * ���� printf ��� !!!!!!!
     */
    // printf("LCD ID:%x\r\n", lcddev.id); /* ��ӡLCD ID */

    if (lcddev.id == 0x9341)
    {
        lcd_ex_ili9341_reginit(); /* ִ��ILI9341��ʼ�� */
    }

    /* ���ڲ�ͬ��Ļ��дʱ��ͬ�������ʱ����Ը����Լ�����Ļ�����޸�
      �������ϳ����߶�ʱ��Ҳ����Ӱ�죬��Ҫ�Լ���������޸ģ� */
    /* ��ʼ������Ժ�,���� */

    if (lcddev.id == 0X9341) // ������,����дʱ��Ϊ���
    {
        FSMC_Bank1E->BWTR[6] &= ~(0XF << 0); // ��ַ����ʱ������
        FSMC_Bank1E->BWTR[6] &= ~(0XF << 8); // ���ݱ���ʱ������
        FSMC_Bank1E->BWTR[6] |= 3 << 0;      // ��ַ����ʱ��Ϊ3��HCLK =18ns
        FSMC_Bank1E->BWTR[6] |= 2 << 8;      // ���ݱ���ʱ��Ϊ6ns*3��HCLK=18ns
    }

    lcd_display_dir(0); /* Ĭ��Ϊ���� */
    LCD_BL(1);          /* �������� */
    lcd_clear(WHITE);
}

/**
 * @brief       ��������
 * @param       color: Ҫ��������ɫ
 * @retval      ��
 */
void lcd_clear(uint16_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;

    totalpoint *= lcddev.height;  /* �õ��ܵ��� */
    lcd_set_cursor(0x00, 0x0000); /* ���ù��λ�� */
    lcd_write_ram_prepare();      /* ��ʼд��GRAM */

    for (index = 0; index < totalpoint; index++)
    {
        LCD->LCD_RAM = color;
    }
}

/**
 * @brief       ��ָ����������䵥����ɫ
 * @param       (sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex - sx + 1) * (ey - sy + 1)
 * @param       color:  Ҫ������ɫ(32λ��ɫ,�������LTDC)
 * @retval      ��
 */
void lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color)
{
    uint16_t i, j;
    uint16_t xlen = 0;
    xlen = ex - sx + 1;

    for (i = sy; i <= ey; i++)
    {
        lcd_set_cursor(sx, i);   /* ���ù��λ�� */
        lcd_write_ram_prepare(); /* ��ʼд��GRAM */

        for (j = 0; j < xlen; j++)
        {
            LCD->LCD_RAM = color; /* ��ʾ��ɫ */
        }
    }
}

/**
 * @brief       ��ָ�����������ָ����ɫ��
 * @param       (sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex - sx + 1) * (ey - sy + 1)
 * @param       color: Ҫ������ɫ�����׵�ַ
 * @retval      ��
 */
void lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
    uint16_t height, width;
    uint16_t i, j;

    width = ex - sx + 1;  /* �õ����Ŀ�� */
    height = ey - sy + 1; /* �߶� */

    for (i = 0; i < height; i++)
    {
        lcd_set_cursor(sx, sy + i); /* ���ù��λ�� */
        lcd_write_ram_prepare();    /* ��ʼд��GRAM */

        for (j = 0; j < width; j++)
        {
            LCD->LCD_RAM = color[i * width + j]; /* д������ */
        }
    }
}

/**
 * @brief       ����
 * @param       x1,y1: �������
 * @param       x2,y2: �յ�����
 * @param       color: �ߵ���ɫ
 * @retval      ��
 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;
    delta_x = x2 - x1; /* ������������ */
    delta_y = y2 - y1;
    row = x1;
    col = y1;

    if (delta_x > 0)
    {
        incx = 1; /* ���õ������� */
    }
    else if (delta_x == 0)
    {
        incx = 0; /* ��ֱ�� */
    }
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)
    {
        incy = 1;
    }
    else if (delta_y == 0)
    {
        incy = 0; /* ˮƽ�� */
    }
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y)
    {
        distance = delta_x; /* ѡȡ�������������� */
    }
    else
    {
        distance = delta_y;
    }

    for (t = 0; t <= distance + 1; t++) /* ������� */
    {
        lcd_draw_point(row, col, color); /* ���� */
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
}

/**
 * @brief       ��ˮƽ��
 * @param       x,y   : �������
 * @param       len   : �߳���
 * @param       color : ���ε���ɫ
 * @retval      ��
 */
void lcd_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
    if ((len == 0) || (x > lcddev.width) || (y > lcddev.height))
    {
        return;
    }

    lcd_fill(x, y, x + len - 1, y, color);
}

/**
 * @brief       ������
 * @param       x1,y1: �������
 * @param       x2,y2: �յ�����
 * @param       color: ���ε���ɫ
 * @retval      ��
 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    lcd_draw_line(x1, y1, x2, y1, color);
    lcd_draw_line(x1, y1, x1, y2, color);
    lcd_draw_line(x1, y2, x2, y2, color);
    lcd_draw_line(x2, y1, x2, y2, color);
}

/**
 * @brief       ��Բ
 * @param       x0,y0 : Բ��������
 * @param       r     : �뾶
 * @param       color : Բ����ɫ
 * @retval      ��
 */
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a, b;
    int di;

    a = 0;
    b = r;
    di = 3 - (r << 1); /* �ж��¸���λ�õı�־ */

    while (a <= b)
    {
        lcd_draw_point(x0 + a, y0 - b, color); /* 5 */
        lcd_draw_point(x0 + b, y0 - a, color); /* 0 */
        lcd_draw_point(x0 + b, y0 + a, color); /* 4 */
        lcd_draw_point(x0 + a, y0 + b, color); /* 6 */
        lcd_draw_point(x0 - a, y0 + b, color); /* 1 */
        lcd_draw_point(x0 - b, y0 + a, color);
        lcd_draw_point(x0 - a, y0 - b, color); /* 2 */
        lcd_draw_point(x0 - b, y0 - a, color); /* 7 */
        a++;

        /* ʹ��Bresenham�㷨��Բ */
        if (di < 0)
        {
            di += 4 * a + 6;
        }
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

/**
 * @brief       ���ʵ��Բ
 * @param       x,y  : Բ��������
 * @param       r    : �뾶
 * @param       color: Բ����ɫ
 * @retval      ��
 */
void lcd_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    uint32_t i;
    uint32_t imax = ((uint32_t)r * 707) / 1000 + 1;
    uint32_t sqmax = (uint32_t)r * (uint32_t)r + (uint32_t)r / 2;
    uint32_t xr = r;

    lcd_draw_hline(x - r, y, 2 * r, color);

    for (i = 1; i <= imax; i++)
    {
        if ((i * i + xr * xr) > sqmax)
        {
            /* draw lines from outside */
            if (xr > imax)
            {
                lcd_draw_hline(x - i + 1, y + xr, 2 * (i - 1), color);
                lcd_draw_hline(x - i + 1, y - xr, 2 * (i - 1), color);
            }

            xr--;
        }

        /* draw lines from inside (center) */
        lcd_draw_hline(x - xr, y + i, 2 * xr, color);
        lcd_draw_hline(x - xr, y - i, 2 * xr, color);
    }
}

/**
 * @brief       ��ָ��λ����ʾһ���ַ�
 * @param       x,y  : ����
 * @param       chr  : Ҫ��ʾ���ַ�:" "--->"~"
 * @param       size : �����С 12/16/24/32
 * @param       mode : ���ӷ�ʽ(1); �ǵ��ӷ�ʽ(0);
 * @param       color : �ַ�����ɫ;
 * @retval      ��
 */
void lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t temp, t1, t;
    uint16_t y0 = y;
    uint8_t csize = 0;
    uint8_t *pfont = 0;

    csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); /* �õ�����һ���ַ���Ӧ������ռ���ֽ��� */
    chr = chr - ' ';                                        /* �õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩ */

    switch (size)
    {
    case 12:
        pfont = (uint8_t *)asc2_1206[chr]; /* ����1206���� */
        break;

    case 16:
        pfont = (uint8_t *)asc2_1608[chr]; /* ����1608���� */
        break;

    case 24:
        pfont = (uint8_t *)asc2_2412[chr]; /* ����2412���� */
        break;

        // case 32:
        //     pfont = (uint8_t *)asc2_3216[chr]; /* ����3216���� */
        //     break;

    default:
        return;
    }

    for (t = 0; t < csize; t++)
    {
        temp = pfont[t]; /* ��ȡ�ַ��ĵ������� */

        for (t1 = 0; t1 < 8; t1++) /* һ���ֽ�8���� */
        {
            if (temp & 0x80) /* ��Ч��,��Ҫ��ʾ */
            {
                lcd_draw_point(x, y, color); /* �������,Ҫ��ʾ����� */
            }
            else if (mode == 0) /* ��Ч��,����ʾ */
            {
                lcd_draw_point(x, y, g_back_color); /* ������ɫ,�൱������㲻��ʾ(ע�ⱳ��ɫ��ȫ�ֱ�������) */
            }

            temp <<= 1; /* ��λ, �Ա��ȡ��һ��λ��״̬ */
            y++;

            if (y >= lcddev.height)
                return; /* �������� */

            if ((y - y0) == size) /* ��ʾ��һ����? */
            {
                y = y0; /* y���긴λ */
                x++;    /* x������� */

                if (x >= lcddev.width)
                {
                    return; /* x���곬������ */
                }

                break;
            }
        }
    }
}

/**
 * @brief       ƽ������, m^n
 * @param       m: ����
 * @param       n: ָ��
 * @retval      m��n�η�
 */
static uint32_t lcd_pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;

    while (n--)
    {
        result *= m;
    }

    return result;
}

/**
 * @brief       ��ʾlen������
 * @param       x,y : ��ʼ����
 * @param       num : ��ֵ(0 ~ 2^32)
 * @param       len : ��ʾ���ֵ�λ��
 * @param       size: ѡ������ 12/16/24/32
 * @param       color : ���ֵ���ɫ;
 * @retval      ��
 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++) /* ������ʾλ��ѭ�� */
    {
        temp = (num / lcd_pow(10, len - t - 1)) % 10; /* ��ȡ��Ӧλ������ */

        if (enshow == 0 && t < (len - 1)) /* û��ʹ����ʾ,�һ���λҪ��ʾ */
        {
            if (temp == 0)
            {
                lcd_show_char(x + (size / 2) * t, y, ' ', size, 0, color); /* ��ʾ�ո�,ռλ */
                continue;                                                  /* �����¸�һλ */
            }
            else
            {
                enshow = 1; /* ʹ����ʾ */
            }
        }

        lcd_show_char(x + (size / 2) * t, y, temp + '0', size, 0, color); /* ��ʾ�ַ� */
    }
}

/**
 * @brief       ��չ��ʾlen������(��λ��0Ҳ��ʾ)
 * @param       x,y : ��ʼ����
 * @param       num : ��ֵ(0 ~ 2^32)
 * @param       len : ��ʾ���ֵ�λ��
 * @param       size: ѡ������ 12/16/24/32
 * @param       mode: ��ʾģʽ
 *              [7]:0,�����;1,���0.
 *              [6:1]:����
 *              [0]:0,�ǵ�����ʾ;1,������ʾ.
 * @param       color : ���ֵ���ɫ;
 * @retval      ��
 */
void lcd_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++) /* ������ʾλ��ѭ�� */
    {
        temp = (num / lcd_pow(10, len - t - 1)) % 10; /* ��ȡ��Ӧλ������ */

        if (enshow == 0 && t < (len - 1)) /* û��ʹ����ʾ,�һ���λҪ��ʾ */
        {
            if (temp == 0)
            {
                if (mode & 0x80) /* ��λ��Ҫ���0 */
                {
                    lcd_show_char(x + (size / 2) * t, y, '0', size, mode & 0x01, color); /* ��0ռλ */
                }
                else
                {
                    lcd_show_char(x + (size / 2) * t, y, ' ', size, mode & 0x01, color); /* �ÿո�ռλ */
                }

                continue;
            }
            else
            {
                enshow = 1; /* ʹ����ʾ */
            }
        }

        lcd_show_char(x + (size / 2) * t, y, temp + '0', size, mode & 0x01, color);
    }
}

/**
 * @brief       ��ʾ�ַ���
 * @param       x,y         : ��ʼ����
 * @param       width,height: �����С
 * @param       size        : ѡ������ 12/16/24/32
 * @param       p           : �ַ����׵�ַ
 * @param       color       : �ַ�������ɫ;
 * @retval      ��
 */
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color)
{
    uint8_t x0 = x;

    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' ')) /* �ж��ǲ��ǷǷ��ַ�! */
    {
        if (x >= width)
        {
            x = x0;
            y += size;
        }

        if (y >= height)
        {
            break; /* �˳� */
        }

        lcd_show_char(x, y, *p, size, 0, color);
        x += size / 2;
        p++;
    }
}
#else

// ��ʼ��LCD�Ļ�����ɫ�ͱ���ɫ
uint16_t BRUSH_COLOR = BLACK; // ������ɫ
uint16_t BACK_COLOR = WHITE;  // ����ɫ

// LCD��������
uint16_t lcd_id;        // LCD ID
uint8_t dir_flag;       // �������������ƣ�0��������1������
uint16_t lcd_width;     // LCD�Ŀ��
uint16_t lcd_height;    // LCD�ĸ߶�
uint16_t write_gramcmd; // дgramָ��
uint16_t setxcmd;       // ����x����ָ��
uint16_t setycmd;       // ����y����ָ��

/****************************************************************************
 * ��    ��: void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_Value)
 * ��    �ܣ�LCDд�Ĵ���
 * ��ڲ�����LCD_Reg: �Ĵ�����ַ
 *           LCD_RegValue: Ҫд�������
 * ���ز�������
 * ˵    ����
 ****************************************************************************/
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_Value)
{
    LCD_CMD = LCD_Reg;    // д��Ҫд�ļĴ������
    LCD_DATA = LCD_Value; // ��Ĵ���д�������
}

/****************************************************************************
 * ��    ��: uint16_t LCD_ReadReg(uint16_t LCD_Reg)
 * ��    �ܣ�LCD���Ĵ���
 * ��ڲ�����LCD_Reg:�Ĵ�����ַ
 * ���ز����������üĴ���������ֵ
 * ˵    ����
 ****************************************************************************/
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
    LCD_CMD = LCD_Reg; // д��Ҫ���ļĴ������
    delay_us(4);
    return LCD_DATA; // ���ض�����ֵ
}

// lcd��ʱ����
void lcdm_delay(uint8_t i)
{
    while (i--)
        ;
}

// ��ʼдGRAM
void LCD_WriteGRAM(void)
{
    LCD_CMD = write_gramcmd;
}
// ��ʼдGRAM
void LCD_WriteData(volatile uint16_t data)
{
    data = data; /* ʹ��-O2�Ż���ʱ��,����������ʱ */
    LCD_DATA = data;
}

// LCD������ʾ
void LCD_DisplayOn(void)
{
    LCD_CMD = 0x29; // 9341��1963����ʾ����һ��
}

// LCD�ر���ʾ
void LCD_DisplayOff(void)
{
    LCD_CMD = 0x28; // 9341��1963����ʾ����һ��
}

/****************************************************************************
* ��    ��: void LCD_Open_Window(uint16_t X0,uint16_t Y0,uint16_t width,uint16_t height)
* ��    �ܣ�������,�����û������굽�������Ͻ�(X0,Y0)
* ��ڲ�����X0,Y0:������ʼ����(���Ͻ�)
            width,height:���ڿ�Ⱥ͸߶�
* ���ز�������
* ˵    ���������С:width*height.�B
****************************************************************************/
void LCD_Open_Window(uint16_t X0, uint16_t Y0, uint16_t width, uint16_t height)
{
    width = X0 + width - 1; // ������½�����
    height = Y0 + height - 1;

    if (dir_flag == 0 && lcd_id == 0X1963) // 1963��������
    {
        X0 = lcd_width - width - X0;
        height = Y0 + height - 1;
        LCD_CMD = setxcmd;
        LCD_DATA = X0 >> 8;
        LCD_DATA = X0 & 0XFF;
        LCD_DATA = (X0 + width - 1) >> 8;
        LCD_DATA = (X0 + width - 1) & 0XFF;
        LCD_CMD = setycmd;
        LCD_DATA = Y0 >> 8;
        LCD_DATA = Y0 & 0XFF;
        LCD_DATA = height >> 8;
        LCD_DATA = height & 0XFF;
    }
    else
    {
        LCD_CMD = setxcmd;
        LCD_DATA = X0 >> 8;
        LCD_DATA = X0 & 0XFF;
        LCD_DATA = width >> 8;
        LCD_DATA = width & 0XFF;
        LCD_CMD = setycmd;
        LCD_DATA = Y0 >> 8;
        LCD_DATA = Y0 & 0XFF;
        LCD_DATA = height >> 8;
        LCD_DATA = height & 0XFF;
    }
}

/****************************************************************************
 * ��    ��: void Set_Scan_Direction(uint8_t direction)    ������#������
 * ��    �ܣ�����LCD��ɨ�跽��
 * ��ڲ�����direction��ɨ�跽��
 * ���ز�������
 * ˵    ����
 ****************************************************************************/
void Set_Scan_Direction(uint8_t direction)
{
    uint16_t skhda = 0;
    uint16_t diomf = 0;
    // 9341������1963����ʱ��Ҫת����
    if ((dir_flag == 1 && lcd_id == 0X9341) || (dir_flag == 0 && lcd_id == 0X1963))
    {
        switch (direction) // ����ת��
        {
        case 0:
            direction = 6;
            break;
        case 1:
            direction = 7;
            break;
        case 2:
            direction = 4;
            break;
        case 3:
            direction = 5;
            break;
        case 4:
            direction = 1;
            break;
        case 5:
            direction = 0;
            break;
        case 6:
            direction = 3;
            break;
        case 7:
            direction = 2;
            break;
        }
    }

    switch (direction)
    {
    case L2R_U2D: // ������,���ϵ���
        skhda |= (0 << 7) | (0 << 6) | (0 << 5);
        break;
    case L2R_D2U: // ������,���µ���
        skhda |= (1 << 7) | (0 << 6) | (0 << 5);
        break;
    case R2L_U2D: // ���ҵ���,���ϵ���
        skhda |= (0 << 7) | (1 << 6) | (0 << 5);
        break;
    case R2L_D2U: // ���ҵ���,���µ���
        skhda |= (1 << 7) | (1 << 6) | (0 << 5);
        break;
    case U2D_L2R: // ���ϵ���,������
        skhda |= (0 << 7) | (0 << 6) | (1 << 5);
        break;
    case U2D_R2L: // ���ϵ���,���ҵ���
        skhda |= (0 << 7) | (1 << 6) | (1 << 5);
        break;
    case D2U_L2R: // ���µ���,������
        skhda |= (1 << 7) | (0 << 6) | (1 << 5);
        break;
    case D2U_R2L: // ���µ���,���ҵ���
        skhda |= (1 << 7) | (1 << 6) | (1 << 5);
        break;
    }
    diomf = 0X36;
    if (lcd_id == 0X9341)
        skhda |= 0X08;
    LCD_WriteReg(diomf, skhda);
    LCD_Open_Window(0, 0, lcd_width, lcd_height); // ������ɨ�跽��󣬿���ʾ����Ϊȫ������
}

/****************************************************************************
* ��    ��: void Set_Display_Mode(uint8_t mode)
* ��    �ܣ�����LCD��ʾ����
* ��ڲ�����mode: 0,����
                  1,����
* ���ز�������
* ˵    �����B
****************************************************************************/
void Set_Display_Mode(uint8_t mode)
{
    if (mode == 0) // ����
    {
        dir_flag = 0;

        if (lcd_id == 0X9341)
        {
            write_gramcmd = 0X2C;
            setxcmd = 0X2A;
            setycmd = 0X2B;
            lcd_width = 240;
            lcd_height = 320;
        }
        else if (lcd_id == 0X1963)
        {
            write_gramcmd = 0X2C; // GRAM��ָ��
            setxcmd = 0X2B;       // дX����ָ��
            setycmd = 0X2A;       // дY����ָ��
            lcd_width = 480;      // ���ÿ��480
            lcd_height = 800;     // ���ø߶�800
        }
    }
    else // ����
    {
        dir_flag = 1;

        if (lcd_id == 0X9341)
        {
            write_gramcmd = 0X2C;
            setxcmd = 0X2A;
            setycmd = 0X2B;
            lcd_width = 320;
            lcd_height = 240;
        }
        else if (lcd_id == 0X1963)
        {
            write_gramcmd = 0X2C; // GRAM��ָ��
            setxcmd = 0X2A;       // дX����ָ��
            setycmd = 0X2B;       // дY����ָ��
            lcd_width = 800;      // ���ÿ��800
            lcd_height = 480;     // ���ø߶�480
        }
    }
    Set_Scan_Direction(L2R_U2D); // ����ɨ�跽��   ������,���µ���
}

/****************************************************************************
* ��    ��: void LCD_SetCursor(uint16_t Xaddr, uint16_t Yaddr)       ��#��%��#��
* ��    �ܣ����ù��λ��
* ��ڲ�����x��x����
            y��y����
* ���ز�������
* ˵    ����
****************************************************************************/
void LCD_SetCursor(uint16_t Xaddr, uint16_t Yaddr)
{
    if (lcd_id == 0X9341)
    {
        LCD_CMD = setxcmd;
        LCD_DATA = (Xaddr >> 8);
        LCD_DATA = (Xaddr & 0XFF);
        LCD_CMD = setycmd;
        LCD_DATA = (Yaddr >> 8);
        LCD_DATA = (Yaddr & 0XFF);
    }
    else if (lcd_id == 0X1963)
    {
        if (dir_flag == 0)
        {
            Xaddr = lcd_width - 1 - Xaddr;
            LCD_CMD = setxcmd;
            LCD_DATA = 0;
            LCD_DATA = 0;
            LCD_DATA = Xaddr >> 8;
            LCD_DATA = Xaddr & 0XFF;
        }
        else
        {
            LCD_CMD = setxcmd;
            LCD_DATA = Xaddr >> 8;
            LCD_DATA = Xaddr & 0XFF;
            LCD_DATA = (lcd_width - 1) >> 8;
            LCD_DATA = (lcd_width - 1) & 0XFF;
        }
        LCD_CMD = setycmd;
        LCD_DATA = Yaddr >> 8;
        LCD_DATA = Yaddr & 0XFF;
        LCD_DATA = (lcd_height - 1) >> 8;
        LCD_DATA = (lcd_height - 1) & 0XFF;
    }
}

/****************************************************************************
* ��    ��: uint16_t LCD_GetPoint(uint16_t x,uint16_t y)
* ��    �ܣ���ȡĳ�����ɫֵ
* ��ڲ�����x��x����
            y��y����
* ���ز������˵����ɫ
* ˵    ����
****************************************************************************/
uint16_t LCD_GetPoint(uint16_t x, uint16_t y)
{
    __IO uint16_t r = 0, g = 0, b = 0;

    LCD_SetCursor(x, y);

    LCD_CMD = 0X2E; // 9341��1963��GRAMָ��һ��
    r = LCD_DATA;

    if (lcd_id == 0X1963)
        return r; // 1963ֱ�Ӷ���������16λ��ɫֵ

    else // ������������9341
    {
        lcdm_delay(2);
        b = LCD_DATA; // 9341Ҫ��2��
        g = r & 0XFF; // 9341��һ�ζ�ȡ����RG��ֵ,R��ǰ,G�ں�,��ռ8λ
        g <<= 8;
        return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11)); // 9341�蹫ʽת��
    }
}

/****************************************************************************
* ��    ��: void LCD_DrawPoint(uint16_t x,uint16_t y)
* ��    �ܣ����㣨�ڸõ�д�뻭�ʵ���ɫ��
* ��ڲ�����x��x����
            y��y����
* ���ز�������
* ˵    ���BRUSH_COLOR:�˵����ɫֵ
****************************************************************************/
void LCD_DrawPoint(uint16_t x, uint16_t y)
{
    LCD_SetCursor(x, y); // ���ù��λ��
    LCD_WriteGRAM();     // ��ʼд��GRAM
    LCD_DATA = BRUSH_COLOR;
}

/****************************************************************************
* ��    ��: void LCD_Color_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
* ��    �ܣ������õ����괦����Ӧ��ɫ���ڸõ�д���Զ�����ɫ��
* ��ڲ�����x��x����
            y��y����
            color �˵����ɫֵ
* ���ز�������
* ˵    ����color:д��˵����ɫֵ   GUI���øú���
****************************************************************************/
void LCD_Color_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_DrawPoint(x, y);
    LCD_CMD = write_gramcmd;
    LCD_DATA = color;
}

/****************************************************************************
 * ��    ��: void Ssd1963_Set_BackLight(uint8_t BL_value)
 * ��    �ܣ�SSD1963 ���ñ���
 * ��ڲ�����BL_value���������ȴ�С  ȡֵ:0-255  ����255����
 * ���ز�������
 * ˵    ����
 ****************************************************************************/
void Ssd1963_Set_BackLight(uint8_t BL_value)
{
    LCD_CMD = 0xBE;
    LCD_DATA = 0x05;
    LCD_DATA = BL_value;
    LCD_DATA = 0x01;
    LCD_DATA = 0xFF;
    LCD_DATA = 0x00;
    LCD_DATA = 0x00;
}

/****************************************************************************
 * ��    ��: void LCD_Clear(uint16_t color)
 * ��    �ܣ���������
 * ��ڲ�����color: Ҫ���������ɫ
 * ���ز�������
 * ˵    �����B
 ****************************************************************************/
void LCD_Clear(uint16_t color)
{
    uint32_t i = 0;
    uint32_t pointnum = 0;

    pointnum = lcd_width * lcd_height; // �õ�LCD�ܵ���
    LCD_SetCursor(0x00, 0x00);         // ���ù��λ��
    LCD_WriteGRAM();                   // ��ʼд��GRAM
    for (i = 0; i < pointnum; i++)
    {
        LCD_DATA = color;
    }
}

uint16_t ILI9341_Read_id(void)
{
    uint16_t id;

    LCD_CMD = 0xD3; // 9341��ID����
    id = LCD_DATA;
    id = LCD_DATA; // 0x00
    id = LCD_DATA; // 0x93
    id <<= 8;
    id |= LCD_DATA; // 0x41

    return id;
}

uint16_t SSD1963_Read_id(void)
{
    uint16_t id;

    LCD_CMD = (0xA1); // 1963��ID����
    id = LCD_DATA;
    id = LCD_DATA; // 0x57
    id <<= 8;
    id |= LCD_DATA; // 0x61

    return id;
}

// ��ʼ��lcd
void LCD_Init(void)
{
    lcd_id = ILI9341_Read_id(); // �ȶ�����������Ļ�ǲ���9341����

    if (lcd_id != 0x9341) // �������9341���������ǲ���1963����
    {
        lcd_id = SSD1963_Read_id();
        if (lcd_id == 0x5761)
            lcd_id = 0x1963; // SSD1963ʵ�ʶ�����ID��0x5761,Ϊ��ֱ�ۣ��������Ϊ1963
    }

    if (lcd_id == 0X9341) // ������,����дʱ��Ϊ���
    {
        FSMC_Bank1E->BWTR[6] &= ~(0XF << 0); // ��ַ����ʱ������
        FSMC_Bank1E->BWTR[6] &= ~(0XF << 8); // ���ݱ���ʱ������
        FSMC_Bank1E->BWTR[6] |= 3 << 0;      // ��ַ����ʱ��Ϊ3��HCLK =18ns
        FSMC_Bank1E->BWTR[6] |= 2 << 8;      // ���ݱ���ʱ��Ϊ6ns*3��HCLK=18ns
    }

    if (lcd_id == 0X9341) // 9341��ʼ��
    {
        LCD_CMD = 0xCF;
        LCD_DATA = 0x00;
        LCD_DATA = 0xC1;
        LCD_DATA = 0X30;
        LCD_CMD = 0xED;
        LCD_DATA = 0x64;
        LCD_DATA = 0x03;
        LCD_DATA = 0X12;
        LCD_DATA = 0X81;
        LCD_CMD = 0xE8;
        LCD_DATA = 0x85;
        LCD_DATA = 0x10;
        LCD_DATA = 0x7A;
        LCD_CMD = 0xCB;
        LCD_DATA = 0x39;
        LCD_DATA = 0x2C;
        LCD_DATA = 0x00;
        LCD_DATA = 0x34;
        LCD_DATA = 0x02;
        LCD_CMD = 0xF7;
        LCD_DATA = 0x20;
        LCD_CMD = 0xEA;
        LCD_DATA = 0x00;
        LCD_DATA = 0x00;
        LCD_CMD = 0xC0;
        LCD_DATA = 0x1B;
        LCD_CMD = 0xC1;
        LCD_DATA = 0x01;
        LCD_CMD = 0xC5;
        LCD_DATA = 0x30;
        LCD_DATA = 0x30;
        LCD_CMD = 0xC7;
        LCD_DATA = 0XB7;
        LCD_CMD = 0x36;
        LCD_DATA = 0x48;
        LCD_CMD = 0x3A;
        LCD_DATA = 0x55;
        LCD_CMD = 0xB1;
        LCD_DATA = 0x00;
        LCD_DATA = 0x1A;
        LCD_CMD = 0xB6;
        LCD_DATA = 0x0A;
        LCD_DATA = 0xA2;
        LCD_CMD = 0xF2;
        LCD_DATA = 0x00;
        LCD_CMD = 0x26;
        LCD_DATA = 0x01;
        LCD_CMD = 0xE0;
        LCD_DATA = 0x0F;
        LCD_DATA = 0x2A;
        LCD_DATA = 0x28;
        LCD_DATA = 0x08;
        LCD_DATA = 0x0E;
        LCD_DATA = 0x08;
        LCD_DATA = 0x54;
        LCD_DATA = 0XA9;
        LCD_DATA = 0x43;
        LCD_DATA = 0x0A;
        LCD_DATA = 0x0F;
        LCD_DATA = 0x00;
        LCD_DATA = 0x00;
        LCD_DATA = 0x00;
        LCD_DATA = 0x00;
        LCD_CMD = 0XE1;
        LCD_DATA = 0x00;
        LCD_DATA = 0x15;
        LCD_DATA = 0x17;
        LCD_DATA = 0x07;
        LCD_DATA = 0x11;
        LCD_DATA = 0x06;
        LCD_DATA = 0x2B;
        LCD_DATA = 0x56;
        LCD_DATA = 0x3C;
        LCD_DATA = 0x05;
        LCD_DATA = 0x10;
        LCD_DATA = 0x0F;
        LCD_DATA = 0x3F;
        LCD_DATA = 0x3F;
        LCD_DATA = 0x0F;
        LCD_CMD = 0x2B;
        LCD_DATA = 0x00;
        LCD_DATA = 0x00;
        LCD_DATA = 0x01;
        LCD_DATA = 0x3f;
        LCD_CMD = 0x2A;
        LCD_DATA = 0x00;
        LCD_DATA = 0x00;
        LCD_DATA = 0x00;
        LCD_DATA = 0xef;
        LCD_CMD = 0x11;
        delay_ms(120);
        LCD_CMD = 0x29;

        // LCD_BACK = 1; // ��������
        HAL_GPIO_WritePin(LCD_BACK_GPIO_Port, LCD_BACK_Pin, GPIO_PIN_SET);
    }
    Set_Display_Mode(0); // ��ʼ��Ϊ����
    LCD_Clear(WHITE);    // ������ɫ
}

/****************************************************************************
* ��    ��: void LCD_Fill_onecolor(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)  ��*��@��#��
* ��    �ܣ���ָ����������䵥����ɫ
* ��ڲ�����(sx,sy),(ex,ey):�����ζԽ�����
            color:Ҫ������ɫ
* ���ز�������
* ˵    ���������СΪ:(ex-sx+1)*(ey-sy+1)  �B
****************************************************************************/
void LCD_Fill_onecolor(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color)
{
    uint16_t i, j;
    uint16_t nlen = 0;

    nlen = ex - sx + 1;
    for (i = sy; i <= ey; i++)
    {
        LCD_SetCursor(sx, i); // ���ù��λ��
        LCD_WriteGRAM();      // ��ʼд��GRAM
        for (j = 0; j < nlen; j++)
            LCD_DATA = color; // ���ù��λ��
    }
}

/****************************************************************************
* ��    ��: void LCD_Draw_Picture(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
* ��    �ܣ���ָ�������ڻ���ͼƬ
* ��ڲ�����(sx,sy),(ex,ey):�����ζԽ�����
            color:Ҫ����ͼƬ������ɫ����
* ���ز�������
* ˵    ���������СΪ:(ex-sx+1)*(ey-sy+1)  �B
****************************************************************************/
void LCD_Draw_Picture(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
    uint16_t height, width;
    uint16_t i, j;
    width = ex - sx + 1;  // �õ�ͼƬ�Ŀ��
    height = ey - sy + 1; // �õ�ͼƬ�ĸ߶�
    for (i = 0; i < height; i++)
    {
        LCD_SetCursor(sx, sy + i); // ���ù��λ��
        LCD_WriteGRAM();           // ��ʼд��GRAM
        for (j = 0; j < width; j++)
            LCD_DATA = color[i * height + j]; // д����ɫֵ
    }
}

/****************************************************************************
* ��    ��: void LCD_DisplayChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size)
* ��    �ܣ���ָ��λ����ʾһ���ַ�
* ��ڲ�����x,y:��ʼ����
            word:Ҫ��ʾ���ַ�:abcdefg1234567890...
            size:�����С 12/16/24
* ���ز�������
* ˵    ��������ģȡģ����Ϊ�ȴ����ң��ٴ��ϵ���  ��λ��ǰ  �B
****************************************************************************/
void LCD_DisplayChar(uint16_t x, uint16_t y, uint8_t word, uint8_t size)
{
    uint8_t bytenum, bytedata, a, b;

    uint16_t xmid = x; // �洢��ʼXֵ(λ��)

    if (size == 12)
        bytenum = 12; // ���ֿ������п�֪��ÿ�����嵥���ַ���ռ���ֽ���
    else if (size == 16)
        bytenum = 16;
    else if (size == 24)
        bytenum = 48;
    else
        return; // ���������˳�

    word = word - ' '; // �ֿ������ǰ�ASCII������
    // cfont.h���ֿ��Ǵӿո�ʼ�� �ո���ǵ�һ��Ԫ�� �����ַ���ASCII���ȥ�ո��͵õ��������е�ƫ��ֵ(λ��)
    for (b = 0; b < bytenum; b++)
    {
        if (size == 12)
            bytedata = asc2_1206[word][b]; // ����1206����
        else if (size == 16)
            bytedata = asc2_1608[word][b]; // ����1608����
        else if (size == 24)
            bytedata = asc2_2412[word][b]; // ����2412����

        for (a = 0; a < 8; a++)
        {
            if (bytedata & 0x01)
                LCD_Color_DrawPoint(x, y, BRUSH_COLOR); // ������ģ�ǵ�λ��ǰ �����ȴӵ�λ�ж�  Ϊ1ʱ��ʾ������ɫ
            else
                LCD_Color_DrawPoint(x, y, BACK_COLOR); // 0ʱ��ʾ������ɫ
            bytedata >>= 1;                            // ��λ�ж��� ��������λ�ж�
            x++;                                       // ��ʾ��һλ ����һλ��ʾ
            if ((x - xmid) == size / 2)                // x���򳬳������С �磺16���� ʵ���� 08*16�ĵ���  ����� size/2
            {
                x = xmid; // �ӳ�ʼXλ����д��һ��
                y++;      // ��һ��д�� ����һ����д
                break;    // ����for(a=0;a<8;a++)ѭ��
            }
        }
    }
}

/****************************************************************************
 * ��    ��: void LCD_DisplayString(uint16_t x,uint16_t y,uint8_t size,uint8_t *p)
 * ��    �ܣ���ʾ�ַ���
 * ��ڲ�����x,y:�������
 *           size:�����С
 *           *p:�ַ�����ʼ��ַ
 * ���ز�������
 * ˵    ����  �B
 ****************************************************************************/
void LCD_DisplayString(uint16_t x, uint16_t y, uint8_t size, uint8_t *p)
{
    while ((*p >= ' ') && (*p <= '~')) // ֻ��ʾ�� ������~��֮����ַ�
    {
        LCD_DisplayChar(x, y, *p, size);
        x += size / 2;
        if (x >= lcd_width)
            break;
        p++;
    }
}

/****************************************************************************
 * ��    ��: void LCD_DisplayString(uint16_t x,uint16_t y,uint8_t size,uint8_t *p)
 * ��    �ܣ���ʾ�Զ����ַ���
 * ��ڲ�����x,y:�������
 *           width,height:�����С
 *           size:�����С
 *           *p:�ַ�����ʼ��ַ
 *           brushcolor���Զ��廭����ɫ
 *           backcolor�� �Զ��屳����ɫ
 * ���ز�������
 * ˵    ����  �B
 ****************************************************************************/
void LCD_DisplayString_color(uint16_t x, uint16_t y, uint8_t size, uint8_t *p, uint16_t brushcolor, uint16_t backcolor)
{
    uint16_t bh_color, bk_color;

    bh_color = BRUSH_COLOR; // �ݴ滭����ɫ
    bk_color = BACK_COLOR;  // �ݴ汳����ɫ

    BRUSH_COLOR = brushcolor;
    BACK_COLOR = backcolor;

    LCD_DisplayString(x, y, size, p);

    BRUSH_COLOR = bh_color; // ���ı�ϵͳ��ɫ
    BACK_COLOR = bk_color;
}

// a^n����������ֵ:a^n�η�
uint32_t Counter_Power(uint8_t a, uint8_t n)
{
    uint32_t mid = 1;
    while (n--)
        mid *= a;
    return mid;
}

/****************************************************************************
* ��    ��: void LCD_DisplayNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
* ��    �ܣ���ָ��λ����ʾһ������
* ��ڲ�����x,y:�������
            value:��ֵ;
            len:����(������ʾ��λ��)
            size:�����С
            mode: 0����λΪ0����ʾ
                  1����λΪ0����len���Ȳ���ʾ����0
* ���ز�������
* ˵    ����  �B
****************************************************************************/
void LCD_DisplayNum(uint16_t x, uint16_t y, uint32_t value, uint8_t len, uint8_t size, uint8_t mode)
{
    uint8_t t, numtemp;
    uint8_t value_num; // ��ֵ�ܹ���λ��
    uint32_t value_mid;

    value_mid = value; // ����λ��ʱ��Ӱ��Ҫ��ʾ����ֵ��С
    for (value_num = 0; value_mid > 0; value_num++)
    {
        value_mid /= 10;
    } // ִ����for����֪��Ҫ��ʾ����ֵΪ��λ��

    if (value_num > len) // ��ֵλ����������λ��������ʾ���򲻹� ��ʾ����
    {
        LCD_DisplayString(x, y, size, "ERROR");
        return; // �˳�����
    }
    else
    {
        for (t = 0; t < len; t++)
        {
            if (t < (len - value_num))
            {
                if (mode)
                    LCD_DisplayChar(x + (size / 2) * t, y, '0', size);
                else
                    LCD_DisplayChar(x + (size / 2) * t, y, ' ', size);
            }
            else
            {
                numtemp = (value / Counter_Power(10, len - t - 1)) % 10; // ȡ����λ��ֵ
                LCD_DisplayChar(x + (size / 2) * t, y, numtemp + '0', size);
            }
        }
    }
}

/****************************************************************************
* ��    ��: void LCD_DisplayNum_color(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
* ��    �ܣ���ָ��λ����ʾһ���Զ�����ɫ������  ��#��*��&��
* ��ڲ�����x,y:�������
            num:��ֵ;
            len:����(��Ҫ��ʾ��λ��)
            size:�����С
            mode: 0����λΪ0����ʾ
                  1����λΪ0��ʾ0
            brushcolor���Զ��廭����ɫ
            backcolor�� �Զ��屳����ɫ
* ���ز�������
* ˵    ����  �B
****************************************************************************/
void LCD_DisplayNum_color(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode, uint16_t brushcolor, uint16_t backcolor)
{
    uint16_t bh_color, bk_color;

    bh_color = BRUSH_COLOR; // �ݴ滭����ɫ
    bk_color = BACK_COLOR;  // �ݴ汳����ɫ

    BRUSH_COLOR = brushcolor;
    BACK_COLOR = backcolor;

    LCD_DisplayNum(x, y, num, len, size, mode);

    BRUSH_COLOR = bh_color; // ���ı�ϵͳ��ɫ
    BACK_COLOR = bk_color;
}
#endif
