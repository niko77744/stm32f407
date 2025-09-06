#define LOG_TAG "lcd"
#include "lcd.h"
#include "elog.h"
#include "./font/font.h"
#if 0

SRAM_HandleTypeDef g_sram_handle; /* SRAM句柄(用于控制LCD) */

/* LCD的画笔颜色和背景色 */
uint32_t g_point_color = 0xF800; /* 画笔颜色 */
uint32_t g_back_color = 0xFFFF;  /* 背景色 */

/* 管理LCD重要参数 */
_lcd_dev lcddev;

/**
 * @brief       LCD写数据
 * @param       data: 要写入的数据
 * @retval      无
 */
void lcd_wr_data(volatile uint16_t data)
{
    data = data; /* 使用-O2优化的时候,必须插入的延时 */
    LCD->LCD_RAM = data;
}

/**
 * @brief       LCD写寄存器编号/地址函数
 * @param       regno: 寄存器编号/地址
 * @retval      无
 */
void lcd_wr_regno(volatile uint16_t regno)
{
    regno = regno;        /* 使用-O2优化的时候,必须插入的延时 */
    LCD->LCD_REG = regno; /* 写入要写的寄存器序号 */
}

/**
 * @brief       LCD写寄存器
 * @param       regno:寄存器编号/地址
 * @param       data:要写入的数据
 * @retval      无
 */
void lcd_write_reg(uint16_t regno, uint16_t data)
{
    LCD->LCD_REG = regno; /* 写入要写的寄存器序号 */
    LCD->LCD_RAM = data;  /* 写入数据 */
}

/**
 * @brief       LCD延时函数,仅用于部分在mdk -O1时间优化时需要设置的地方
 * @param       t:延时的数值
 * @retval      无
 */
static void lcd_opt_delay(uint32_t i)
{
    while (i--)
        ; /* 使用AC6时空循环可能被优化,可使用while(1) __asm volatile(""); */
}

/**
 * @brief       LCD读数据
 * @param       无
 * @retval      读取到的数据
 */
static uint16_t lcd_rd_data(void)
{
    volatile uint16_t ram; /* 防止被优化 */
    lcd_opt_delay(2);
    ram = LCD->LCD_RAM;
    return ram;
}

/**
 * @brief       准备写GRAM
 * @param       无
 * @retval      无
 */
void lcd_write_ram_prepare(void)
{
    LCD->LCD_REG = lcddev.wramcmd;
}

/**
 * @brief       读取个某点的颜色值
 * @param       x,y:坐标
 * @retval      此点的颜色(32位颜色,方便兼容LTDC)
 */
uint32_t lcd_read_point(uint16_t x, uint16_t y)
{
    uint16_t r = 0, g = 0, b = 0;

    if (x >= lcddev.width || y >= lcddev.height)
    {
        return 0; /* 超过了范围,直接返回 */
    }

    lcd_set_cursor(x, y); /* 设置坐标 */

    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(0x2E00); /* 5510 发送读GRAM指令 */
    }
    else
    {
        lcd_wr_regno(0x2E); /* 9341/5310/1963/7789/7796/9806 等发送读GRAM指令 */
    }

    r = lcd_rd_data(); /* 假读(dummy read) */

    if (lcddev.id == 0x1963)
    {
        return r; /* 1963直接读就可以 */
    }

    r = lcd_rd_data(); /* 实际坐标颜色 */

    if (lcddev.id == 0x7796) /* 7796 一次读取一个像素值 */
    {
        return r;
    }

    /* 9341/5310/5510/7789/9806要分2次读出 */
    b = lcd_rd_data();
    g = r & 0xFF; /* 对于9341/5310/5510/7789/9806,第一次读取的是RG的值,R在前,G在后,各占8位 */
    g <<= 8;

    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11)); /* ILI9341/NT35310/NT35510/ST7789/ILI9806需要公式转换一下 */
}

/**
 * @brief       LCD开启显示
 * @param       无
 * @retval      无
 */
void lcd_display_on(void)
{
    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(0x2900); /* 开启显示 */
    }
    else /* 9341/5310/1963/7789/7796/9806 等发送开启显示指令 */
    {
        lcd_wr_regno(0x29); /* 开启显示 */
    }
}

/**
 * @brief       LCD关闭显示
 * @param       无
 * @retval      无
 */
void lcd_display_off(void)
{
    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(0x2800); /* 关闭显示 */
    }
    else /* 9341/5310/1963/7789/7796/9806 等发送关闭显示指令 */
    {
        lcd_wr_regno(0x28); /* 关闭显示 */
    }
}

/**
 * @brief       设置光标位置(对RGB屏无效)
 * @param       x,y: 坐标
 * @retval      无
 */
void lcd_set_cursor(uint16_t x, uint16_t y)
{
    if (lcddev.id == 0x1963)
    {
        if (lcddev.dir == 0) /* 竖屏模式, x坐标需要变换 */
        {
            x = lcddev.width - 1 - x;
            lcd_wr_regno(lcddev.setxcmd);
            lcd_wr_data(0);
            lcd_wr_data(0);
            lcd_wr_data(x >> 8);
            lcd_wr_data(x & 0xFF);
        }
        else /* 横屏模式 */
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
    else /* 9341/5310/7789/7796/9806 等 设置坐标 */
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
 * @brief       设置LCD的自动扫描方向(对RGB屏无效)
 *   @note
 *              9341/5310/5510/1963/7789/7796/9806等IC已经实际测试
 *              注意:其他函数可能会受到此函数设置的影响(尤其是9341),
 *              所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
 *
 * @param       dir:0~7,代表8个方向(具体定义见lcd.h)
 * @retval      无
 */
void lcd_scan_dir(uint8_t dir)
{
    uint16_t regval = 0;
    uint16_t dirreg = 0;
    uint16_t temp;

    /* 横屏时，对1963不改变扫描方向！竖屏时1963改变方向(这里仅用于1963的特殊处理,对其他驱动IC无效) */
    if ((lcddev.dir == 1 && lcddev.id != 0x1963) || (lcddev.dir == 0 && lcddev.id == 0x1963))
    {
        switch (dir) /* 方向转换 */
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

    /* 根据扫描方式 设置 0x36/0x3600 寄存器 bit 5,6,7 位的值 */
    switch (dir)
    {
    case L2R_U2D: /* 从左到右,从上到下 */
        regval |= (0 << 7) | (0 << 6) | (0 << 5);
        break;

    case L2R_D2U: /* 从左到右,从下到上 */
        regval |= (1 << 7) | (0 << 6) | (0 << 5);
        break;

    case R2L_U2D: /* 从右到左,从上到下 */
        regval |= (0 << 7) | (1 << 6) | (0 << 5);
        break;

    case R2L_D2U: /* 从右到左,从下到上 */
        regval |= (1 << 7) | (1 << 6) | (0 << 5);
        break;

    case U2D_L2R: /* 从上到下,从左到右 */
        regval |= (0 << 7) | (0 << 6) | (1 << 5);
        break;

    case U2D_R2L: /* 从上到下,从右到左 */
        regval |= (0 << 7) | (1 << 6) | (1 << 5);
        break;

    case D2U_L2R: /* 从下到上,从左到右 */
        regval |= (1 << 7) | (0 << 6) | (1 << 5);
        break;

    case D2U_R2L: /* 从下到上,从右到左 */
        regval |= (1 << 7) | (1 << 6) | (1 << 5);
        break;
    }

    dirreg = 0x36; /* 对绝大部分驱动IC, 由0x36寄存器控制 */

    if (lcddev.id == 0x5510)
    {
        dirreg = 0x3600; /* 对于5510, 和其他驱动ic的寄存器有差异 */
    }

    /* 9341 & 7789 & 7796 要设置BGR位 */
    if (lcddev.id == 0x9341 || lcddev.id == 0x7789 || lcddev.id == 0x7796)
    {
        regval |= 0x08;
    }

    lcd_write_reg(dirreg, regval);

    if (lcddev.id != 0x1963) /* 1963不做坐标处理 */
    {
        if (regval & 0x20)
        {
            if (lcddev.width < lcddev.height) /* 交换X,Y */
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
        else
        {
            if (lcddev.width > lcddev.height) /* 交换X,Y */
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
    }

    /* 设置显示区域(开窗)大小 */
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
 * @brief       画点
 * @param       x,y: 坐标
 * @param       color: 点的颜色(32位颜色,方便兼容LTDC)
 * @retval      无
 */
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color)
{
    lcd_set_cursor(x, y);    /* 设置光标位置 */
    lcd_write_ram_prepare(); /* 开始写入GRAM */
    LCD->LCD_RAM = color;
}

/**
 * @brief       SSD1963背光亮度设置函数
 * @param       pwm: 背光等级,0~100.越大越亮.
 * @retval      无
 */
void lcd_ssd_backlight_set(uint8_t pwm)
{
    lcd_wr_regno(0xBE);      /* 配置PWM输出 */
    lcd_wr_data(0x05);       /* 1设置PWM频率 */
    lcd_wr_data(pwm * 2.55); /* 2设置PWM占空比 */
    lcd_wr_data(0x01);       /* 3设置C */
    lcd_wr_data(0xFF);       /* 4设置D */
    lcd_wr_data(0x00);       /* 5设置E */
    lcd_wr_data(0x00);       /* 6设置F */
}

/**
 * @brief       设置LCD显示方向
 * @param       dir:0,竖屏; 1,横屏
 * @retval      无
 */
void lcd_display_dir(uint8_t dir)
{
    lcddev.dir = dir; /* 竖屏/横屏 */

    if (dir == 0) /* 竖屏 */
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
            lcddev.wramcmd = 0x2C; /* 设置写入GRAM的指令 */
            lcddev.setxcmd = 0x2B; /* 设置写X坐标指令 */
            lcddev.setycmd = 0x2A; /* 设置写Y坐标指令 */
            lcddev.width = 480;    /* 设置宽度480 */
            lcddev.height = 800;   /* 设置高度800 */
        }
        else /* 其他IC, 包括: 9341/5310/7789/7796/9806等IC */
        {
            lcddev.wramcmd = 0x2C;
            lcddev.setxcmd = 0x2A;
            lcddev.setycmd = 0x2B;
        }

        if (lcddev.id == 0x5310 || lcddev.id == 0x7796) /* 如果是5310/7796 则表示是 320*480分辨率 */
        {
            lcddev.width = 320;
            lcddev.height = 480;
        }

        if (lcddev.id == 0X9806) /* 如果是9806 则表示是 480*800 分辨率 */
        {
            lcddev.width = 480;
            lcddev.height = 800;
        }
    }
    else /* 横屏 */
    {
        lcddev.width = 320;  /* 默认宽度 */
        lcddev.height = 240; /* 默认高度 */

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
            lcddev.wramcmd = 0x2C; /* 设置写入GRAM的指令 */
            lcddev.setxcmd = 0x2A; /* 设置写X坐标指令 */
            lcddev.setycmd = 0x2B; /* 设置写Y坐标指令 */
            lcddev.width = 800;    /* 设置宽度800 */
            lcddev.height = 480;   /* 设置高度480 */
        }
        else /* 其他IC, 包括:9341/5310/7789/7796等IC */
        {
            lcddev.wramcmd = 0x2C;
            lcddev.setxcmd = 0x2A;
            lcddev.setycmd = 0x2B;
        }

        if (lcddev.id == 0x5310 || lcddev.id == 0x7796) /* 如果是5310/7796 则表示是 320*480分辨率 */
        {
            lcddev.width = 480;
            lcddev.height = 320;
        }
    }

    lcd_scan_dir(DFT_SCAN_DIR); /* 默认扫描方向 */
}

/**
 * @brief       设置窗口(对RGB屏无效), 并自动设置画点坐标到窗口左上角(sx,sy).
 * @param       sx,sy:窗口起始坐标(左上角)
 * @param       width,height:窗口宽度和高度,必须大于0!!
 *   @note      窗体大小:width*height.
 *
 * @retval      无
 */
void lcd_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    uint16_t twidth, theight;
    twidth = sx + width - 1;
    theight = sy + height - 1;

    if (lcddev.id == 0x1963 && lcddev.dir != 1) /* 1963竖屏特殊处理 */
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
    else /* 9341/5310/7789/1963/7796/9806横屏 等 设置窗口 */
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
 * @brief       ILI9341寄存器初始化代码
 * @param       无
 * @retval      无
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
 * @brief       初始化LCD
 *   @note      该初始化函数可以初始化各种型号的LCD(详见本.c文件最前面的描述)
 *
 * @param       无
 * @retval      无
 */
void lcd_init(void)
{

    /* 尝试9341 ID的读取 */
    lcd_wr_regno(0xD3);
    lcddev.id = lcd_rd_data(); /* dummy read */
    lcddev.id = lcd_rd_data(); /* 读到0x00 */
    lcddev.id = lcd_rd_data(); /* 读取93 */
    lcddev.id <<= 8;
    lcddev.id |= lcd_rd_data(); /* 读取41 */

    if (lcddev.id != 0x9341) /* 不是 9341 , 尝试看看是不是 ST7789 */
    {
        lcd_wr_regno(0x04);
        lcddev.id = lcd_rd_data(); /* dummy read */
        lcddev.id = lcd_rd_data(); /* 读到0x85 */
        lcddev.id = lcd_rd_data(); /* 读取0x85 */
        lcddev.id <<= 8;
        lcddev.id |= lcd_rd_data(); /* 读取0x52 */

        if (lcddev.id == 0x8552) /* 将8552的ID转换成7789 */
        {
            lcddev.id = 0x7789;
        }

        if (lcddev.id != 0x7789) /* 也不是ST7789, 尝试是不是 NT35310 */
        {
            lcd_wr_regno(0xD4);
            lcddev.id = lcd_rd_data(); /* dummy read */
            lcddev.id = lcd_rd_data(); /* 读回0x01 */
            lcddev.id = lcd_rd_data(); /* 读回0x53 */
            lcddev.id <<= 8;
            lcddev.id |= lcd_rd_data(); /* 这里读回0x10 */

            if (lcddev.id != 0x5310) /* 也不是NT35310,尝试看看是不是ST7796 */
            {
                lcd_wr_regno(0XD3);
                lcddev.id = lcd_rd_data(); /* dummy read */
                lcddev.id = lcd_rd_data(); /* 读到0X00 */
                lcddev.id = lcd_rd_data(); /* 读取0X77 */
                lcddev.id <<= 8;
                lcddev.id |= lcd_rd_data(); /* 读取0X96 */

                if (lcddev.id != 0x7796) /* 也不是ST7796,尝试看看是不是NT35510 */
                {
                    /* 发送密钥（厂家提供） */
                    lcd_write_reg(0xF000, 0x0055);
                    lcd_write_reg(0xF001, 0x00AA);
                    lcd_write_reg(0xF002, 0x0052);
                    lcd_write_reg(0xF003, 0x0008);
                    lcd_write_reg(0xF004, 0x0001);

                    lcd_wr_regno(0xC500);      /* 读取ID低八位 */
                    lcddev.id = lcd_rd_data(); /* 读回0x80 */
                    lcddev.id <<= 8;

                    lcd_wr_regno(0xC501);       /* 读取ID高八位 */
                    lcddev.id |= lcd_rd_data(); /* 读回0x00 */

                    delay_ms(5); /* 等待5ms, 因为0XC501指令对1963来说就是软件复位指令, 等待5ms让1963复位完成再操作 */

                    if (lcddev.id != 0x5510) /* 也不是NT5510,尝试看看是不是ILI9806 */
                    {
                        lcd_wr_regno(0XD3);
                        lcddev.id = lcd_rd_data(); /* dummy read */
                        lcddev.id = lcd_rd_data(); /* 读回0X00 */
                        lcddev.id = lcd_rd_data(); /* 读回0X98 */
                        lcddev.id <<= 8;
                        lcddev.id |= lcd_rd_data(); /* 读回0X06 */

                        if (lcddev.id != 0x9806) /* 也不是ILI9806,尝试看看是不是SSD1963 */
                        {
                            lcd_wr_regno(0xA1);
                            lcddev.id = lcd_rd_data();
                            lcddev.id = lcd_rd_data(); /* 读回0x57 */
                            lcddev.id <<= 8;
                            lcddev.id |= lcd_rd_data(); /* 读回0x61 */

                            if (lcddev.id == 0x5761)
                                lcddev.id = 0x1963; /* SSD1963读回的ID是5761H,为方便区分,我们强制设置为1963 */
                        }
                    }
                }
            }
        }
    }

    /* 特别注意, 如果在main函数里面屏蔽串口1初始化, 则会卡死在printf
     * 里面(卡死在f_putc函数), 所以, 必须初始化串口1, 或者屏蔽掉下面
     * 这行 printf 语句 !!!!!!!
     */
    // printf("LCD ID:%x\r\n", lcddev.id); /* 打印LCD ID */

    if (lcddev.id == 0x9341)
    {
        lcd_ex_ili9341_reginit(); /* 执行ILI9341初始化 */
    }

    /* 由于不同屏幕的写时序不同，这里的时序可以根据自己的屏幕进行修改
      （若插上长排线对时序也会有影响，需要自己根据情况修改） */
    /* 初始化完成以后,提速 */

    if (lcddev.id == 0X9341) // 此驱动,设置写时序为最快
    {
        FSMC_Bank1E->BWTR[6] &= ~(0XF << 0); // 地址建立时间清零
        FSMC_Bank1E->BWTR[6] &= ~(0XF << 8); // 数据保存时间清零
        FSMC_Bank1E->BWTR[6] |= 3 << 0;      // 地址建立时间为3个HCLK =18ns
        FSMC_Bank1E->BWTR[6] |= 2 << 8;      // 数据保存时间为6ns*3个HCLK=18ns
    }

    lcd_display_dir(0); /* 默认为竖屏 */
    LCD_BL(1);          /* 点亮背光 */
    lcd_clear(WHITE);
}

/**
 * @brief       清屏函数
 * @param       color: 要清屏的颜色
 * @retval      无
 */
void lcd_clear(uint16_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;

    totalpoint *= lcddev.height;  /* 得到总点数 */
    lcd_set_cursor(0x00, 0x0000); /* 设置光标位置 */
    lcd_write_ram_prepare();      /* 开始写入GRAM */

    for (index = 0; index < totalpoint; index++)
    {
        LCD->LCD_RAM = color;
    }
}

/**
 * @brief       在指定区域内填充单个颜色
 * @param       (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex - sx + 1) * (ey - sy + 1)
 * @param       color:  要填充的颜色(32位颜色,方便兼容LTDC)
 * @retval      无
 */
void lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color)
{
    uint16_t i, j;
    uint16_t xlen = 0;
    xlen = ex - sx + 1;

    for (i = sy; i <= ey; i++)
    {
        lcd_set_cursor(sx, i);   /* 设置光标位置 */
        lcd_write_ram_prepare(); /* 开始写入GRAM */

        for (j = 0; j < xlen; j++)
        {
            LCD->LCD_RAM = color; /* 显示颜色 */
        }
    }
}

/**
 * @brief       在指定区域内填充指定颜色块
 * @param       (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex - sx + 1) * (ey - sy + 1)
 * @param       color: 要填充的颜色数组首地址
 * @retval      无
 */
void lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
    uint16_t height, width;
    uint16_t i, j;

    width = ex - sx + 1;  /* 得到填充的宽度 */
    height = ey - sy + 1; /* 高度 */

    for (i = 0; i < height; i++)
    {
        lcd_set_cursor(sx, sy + i); /* 设置光标位置 */
        lcd_write_ram_prepare();    /* 开始写入GRAM */

        for (j = 0; j < width; j++)
        {
            LCD->LCD_RAM = color[i * width + j]; /* 写入数据 */
        }
    }
}

/**
 * @brief       画线
 * @param       x1,y1: 起点坐标
 * @param       x2,y2: 终点坐标
 * @param       color: 线的颜色
 * @retval      无
 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;
    delta_x = x2 - x1; /* 计算坐标增量 */
    delta_y = y2 - y1;
    row = x1;
    col = y1;

    if (delta_x > 0)
    {
        incx = 1; /* 设置单步方向 */
    }
    else if (delta_x == 0)
    {
        incx = 0; /* 垂直线 */
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
        incy = 0; /* 水平线 */
    }
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y)
    {
        distance = delta_x; /* 选取基本增量坐标轴 */
    }
    else
    {
        distance = delta_y;
    }

    for (t = 0; t <= distance + 1; t++) /* 画线输出 */
    {
        lcd_draw_point(row, col, color); /* 画点 */
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
 * @brief       画水平线
 * @param       x,y   : 起点坐标
 * @param       len   : 线长度
 * @param       color : 矩形的颜色
 * @retval      无
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
 * @brief       画矩形
 * @param       x1,y1: 起点坐标
 * @param       x2,y2: 终点坐标
 * @param       color: 矩形的颜色
 * @retval      无
 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    lcd_draw_line(x1, y1, x2, y1, color);
    lcd_draw_line(x1, y1, x1, y2, color);
    lcd_draw_line(x1, y2, x2, y2, color);
    lcd_draw_line(x2, y1, x2, y2, color);
}

/**
 * @brief       画圆
 * @param       x0,y0 : 圆中心坐标
 * @param       r     : 半径
 * @param       color : 圆的颜色
 * @retval      无
 */
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a, b;
    int di;

    a = 0;
    b = r;
    di = 3 - (r << 1); /* 判断下个点位置的标志 */

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

        /* 使用Bresenham算法画圆 */
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
 * @brief       填充实心圆
 * @param       x,y  : 圆中心坐标
 * @param       r    : 半径
 * @param       color: 圆的颜色
 * @retval      无
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
 * @brief       在指定位置显示一个字符
 * @param       x,y  : 坐标
 * @param       chr  : 要显示的字符:" "--->"~"
 * @param       size : 字体大小 12/16/24/32
 * @param       mode : 叠加方式(1); 非叠加方式(0);
 * @param       color : 字符的颜色;
 * @retval      无
 */
void lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t temp, t1, t;
    uint16_t y0 = y;
    uint8_t csize = 0;
    uint8_t *pfont = 0;

    csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); /* 得到字体一个字符对应点阵集所占的字节数 */
    chr = chr - ' ';                                        /* 得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库） */

    switch (size)
    {
    case 12:
        pfont = (uint8_t *)asc2_1206[chr]; /* 调用1206字体 */
        break;

    case 16:
        pfont = (uint8_t *)asc2_1608[chr]; /* 调用1608字体 */
        break;

    case 24:
        pfont = (uint8_t *)asc2_2412[chr]; /* 调用2412字体 */
        break;

        // case 32:
        //     pfont = (uint8_t *)asc2_3216[chr]; /* 调用3216字体 */
        //     break;

    default:
        return;
    }

    for (t = 0; t < csize; t++)
    {
        temp = pfont[t]; /* 获取字符的点阵数据 */

        for (t1 = 0; t1 < 8; t1++) /* 一个字节8个点 */
        {
            if (temp & 0x80) /* 有效点,需要显示 */
            {
                lcd_draw_point(x, y, color); /* 画点出来,要显示这个点 */
            }
            else if (mode == 0) /* 无效点,不显示 */
            {
                lcd_draw_point(x, y, g_back_color); /* 画背景色,相当于这个点不显示(注意背景色由全局变量控制) */
            }

            temp <<= 1; /* 移位, 以便获取下一个位的状态 */
            y++;

            if (y >= lcddev.height)
                return; /* 超区域了 */

            if ((y - y0) == size) /* 显示完一列了? */
            {
                y = y0; /* y坐标复位 */
                x++;    /* x坐标递增 */

                if (x >= lcddev.width)
                {
                    return; /* x坐标超区域了 */
                }

                break;
            }
        }
    }
}

/**
 * @brief       平方函数, m^n
 * @param       m: 底数
 * @param       n: 指数
 * @retval      m的n次方
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
 * @brief       显示len个数字
 * @param       x,y : 起始坐标
 * @param       num : 数值(0 ~ 2^32)
 * @param       len : 显示数字的位数
 * @param       size: 选择字体 12/16/24/32
 * @param       color : 数字的颜色;
 * @retval      无
 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++) /* 按总显示位数循环 */
    {
        temp = (num / lcd_pow(10, len - t - 1)) % 10; /* 获取对应位的数字 */

        if (enshow == 0 && t < (len - 1)) /* 没有使能显示,且还有位要显示 */
        {
            if (temp == 0)
            {
                lcd_show_char(x + (size / 2) * t, y, ' ', size, 0, color); /* 显示空格,占位 */
                continue;                                                  /* 继续下个一位 */
            }
            else
            {
                enshow = 1; /* 使能显示 */
            }
        }

        lcd_show_char(x + (size / 2) * t, y, temp + '0', size, 0, color); /* 显示字符 */
    }
}

/**
 * @brief       扩展显示len个数字(高位是0也显示)
 * @param       x,y : 起始坐标
 * @param       num : 数值(0 ~ 2^32)
 * @param       len : 显示数字的位数
 * @param       size: 选择字体 12/16/24/32
 * @param       mode: 显示模式
 *              [7]:0,不填充;1,填充0.
 *              [6:1]:保留
 *              [0]:0,非叠加显示;1,叠加显示.
 * @param       color : 数字的颜色;
 * @retval      无
 */
void lcd_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++) /* 按总显示位数循环 */
    {
        temp = (num / lcd_pow(10, len - t - 1)) % 10; /* 获取对应位的数字 */

        if (enshow == 0 && t < (len - 1)) /* 没有使能显示,且还有位要显示 */
        {
            if (temp == 0)
            {
                if (mode & 0x80) /* 高位需要填充0 */
                {
                    lcd_show_char(x + (size / 2) * t, y, '0', size, mode & 0x01, color); /* 用0占位 */
                }
                else
                {
                    lcd_show_char(x + (size / 2) * t, y, ' ', size, mode & 0x01, color); /* 用空格占位 */
                }

                continue;
            }
            else
            {
                enshow = 1; /* 使能显示 */
            }
        }

        lcd_show_char(x + (size / 2) * t, y, temp + '0', size, mode & 0x01, color);
    }
}

/**
 * @brief       显示字符串
 * @param       x,y         : 起始坐标
 * @param       width,height: 区域大小
 * @param       size        : 选择字体 12/16/24/32
 * @param       p           : 字符串首地址
 * @param       color       : 字符串的颜色;
 * @retval      无
 */
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color)
{
    uint8_t x0 = x;

    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' ')) /* 判断是不是非法字符! */
    {
        if (x >= width)
        {
            x = x0;
            y += size;
        }

        if (y >= height)
        {
            break; /* 退出 */
        }

        lcd_show_char(x, y, *p, size, 0, color);
        x += size / 2;
        p++;
    }
}
#else

// 初始化LCD的画笔颜色和背景色
uint16_t BRUSH_COLOR = BLACK; // 画笔颜色
uint16_t BACK_COLOR = WHITE;  // 背景色

// LCD驱动参数
uint16_t lcd_id;        // LCD ID
uint8_t dir_flag;       // 横屏与竖屏控制：0，竖屏；1，横屏
uint16_t lcd_width;     // LCD的宽度
uint16_t lcd_height;    // LCD的高度
uint16_t write_gramcmd; // 写gram指令
uint16_t setxcmd;       // 设置x坐标指令
uint16_t setycmd;       // 设置y坐标指令

/****************************************************************************
 * 名    称: void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_Value)
 * 功    能：LCD写寄存器
 * 入口参数：LCD_Reg: 寄存器地址
 *           LCD_RegValue: 要写入的数据
 * 返回参数：无
 * 说    明：
 ****************************************************************************/
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_Value)
{
    LCD_CMD = LCD_Reg;    // 写入要写的寄存器序号
    LCD_DATA = LCD_Value; // 向寄存器写入的数据
}

/****************************************************************************
 * 名    称: uint16_t LCD_ReadReg(uint16_t LCD_Reg)
 * 功    能：LCD读寄存器
 * 入口参数：LCD_Reg:寄存器地址
 * 返回参数：读到该寄存器序号里的值
 * 说    明：
 ****************************************************************************/
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
    LCD_CMD = LCD_Reg; // 写入要读的寄存器序号
    delay_us(4);
    return LCD_DATA; // 返回读到的值
}

// lcd延时函数
void lcdm_delay(uint8_t i)
{
    while (i--)
        ;
}

// 开始写GRAM
void LCD_WriteGRAM(void)
{
    LCD_CMD = write_gramcmd;
}
// 开始写GRAM
void LCD_WriteData(volatile uint16_t data)
{
    data = data; /* 使用-O2优化的时候,必须插入的延时 */
    LCD_DATA = data;
}

// LCD开启显示
void LCD_DisplayOn(void)
{
    LCD_CMD = 0x29; // 9341与1963开显示命令一样
}

// LCD关闭显示
void LCD_DisplayOff(void)
{
    LCD_CMD = 0x28; // 9341与1963关显示命令一样
}

/****************************************************************************
* 名    称: void LCD_Open_Window(uint16_t X0,uint16_t Y0,uint16_t width,uint16_t height)
* 功    能：开窗口,并设置画点坐标到窗口左上角(X0,Y0)
* 入口参数：X0,Y0:窗口起始坐标(左上角)
            width,height:窗口宽度和高度
* 返回参数：无
* 说    明：窗体大小:width*height.
****************************************************************************/
void LCD_Open_Window(uint16_t X0, uint16_t Y0, uint16_t width, uint16_t height)
{
    width = X0 + width - 1; // 算出右下角坐标
    height = Y0 + height - 1;

    if (dir_flag == 0 && lcd_id == 0X1963) // 1963竖屏处理
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
 * 名    称: void Set_Scan_Direction(uint8_t direction)    启￥明#欣￥欣
 * 功    能：设置LCD的扫描方向
 * 入口参数：direction：扫描方向
 * 返回参数：无
 * 说    明：
 ****************************************************************************/
void Set_Scan_Direction(uint8_t direction)
{
    uint16_t skhda = 0;
    uint16_t diomf = 0;
    // 9341横屏和1963竖屏时需要转化下
    if ((dir_flag == 1 && lcd_id == 0X9341) || (dir_flag == 0 && lcd_id == 0X1963))
    {
        switch (direction) // 方向转换
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
    case L2R_U2D: // 从左到右,从上到下
        skhda |= (0 << 7) | (0 << 6) | (0 << 5);
        break;
    case L2R_D2U: // 从左到右,从下到上
        skhda |= (1 << 7) | (0 << 6) | (0 << 5);
        break;
    case R2L_U2D: // 从右到左,从上到下
        skhda |= (0 << 7) | (1 << 6) | (0 << 5);
        break;
    case R2L_D2U: // 从右到左,从下到上
        skhda |= (1 << 7) | (1 << 6) | (0 << 5);
        break;
    case U2D_L2R: // 从上到下,从左到右
        skhda |= (0 << 7) | (0 << 6) | (1 << 5);
        break;
    case U2D_R2L: // 从上到下,从右到左
        skhda |= (0 << 7) | (1 << 6) | (1 << 5);
        break;
    case D2U_L2R: // 从下到上,从左到右
        skhda |= (1 << 7) | (0 << 6) | (1 << 5);
        break;
    case D2U_R2L: // 从下到上,从右到左
        skhda |= (1 << 7) | (1 << 6) | (1 << 5);
        break;
    }
    diomf = 0X36;
    if (lcd_id == 0X9341)
        skhda |= 0X08;
    LCD_WriteReg(diomf, skhda);
    LCD_Open_Window(0, 0, lcd_width, lcd_height); // 设置完扫描方向后，开显示区域为全屏窗口
}

/****************************************************************************
* 名    称: void Set_Display_Mode(uint8_t mode)
* 功    能：设置LCD显示方向
* 入口参数：mode: 0,竖屏
                  1,横屏
* 返回参数：无
* 说    明：
****************************************************************************/
void Set_Display_Mode(uint8_t mode)
{
    if (mode == 0) // 竖屏
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
            write_gramcmd = 0X2C; // GRAM的指令
            setxcmd = 0X2B;       // 写X坐标指令
            setycmd = 0X2A;       // 写Y坐标指令
            lcd_width = 480;      // 设置宽度480
            lcd_height = 800;     // 设置高度800
        }
    }
    else // 横屏
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
            write_gramcmd = 0X2C; // GRAM的指令
            setxcmd = 0X2A;       // 写X坐标指令
            setycmd = 0X2B;       // 写Y坐标指令
            lcd_width = 800;      // 设置宽度800
            lcd_height = 480;     // 设置高度480
        }
    }
    Set_Scan_Direction(L2R_U2D); // 设置扫描方向   从左到右,从下到上
}

/****************************************************************************
* 名    称: void LCD_SetCursor(uint16_t Xaddr, uint16_t Yaddr)       启#明%欣#欣
* 功    能：设置光标位置
* 入口参数：x：x坐标
            y：y坐标
* 返回参数：无
* 说    明：
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
* 名    称: uint16_t LCD_GetPoint(uint16_t x,uint16_t y)
* 功    能：读取某点的颜色值
* 入口参数：x：x坐标
            y：y坐标
* 返回参数：此点的颜色
* 说    明：
****************************************************************************/
uint16_t LCD_GetPoint(uint16_t x, uint16_t y)
{
    __IO uint16_t r = 0, g = 0, b = 0;

    LCD_SetCursor(x, y);

    LCD_CMD = 0X2E; // 9341与1963读GRAM指令一样
    r = LCD_DATA;

    if (lcd_id == 0X1963)
        return r; // 1963直接读出来就是16位颜色值

    else // 其他驱动就是9341
    {
        lcdm_delay(2);
        b = LCD_DATA; // 9341要读2次
        g = r & 0XFF; // 9341第一次读取的是RG的值,R在前,G在后,各占8位
        g <<= 8;
        return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11)); // 9341需公式转换
    }
}

/****************************************************************************
* 名    称: void LCD_DrawPoint(uint16_t x,uint16_t y)
* 功    能：画点（在该点写入画笔的颜色）
* 入口参数：x：x坐标
            y：y坐标
* 返回参数：无
* 说    明RUSH_COLOR:此点的颜色值
****************************************************************************/
void LCD_DrawPoint(uint16_t x, uint16_t y)
{
    LCD_SetCursor(x, y); // 设置光标位置
    LCD_WriteGRAM();     // 开始写入GRAM
    LCD_DATA = BRUSH_COLOR;
}

/****************************************************************************
* 名    称: void LCD_Color_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
* 功    能：在设置的坐标处画相应颜色（在该点写入自定义颜色）
* 入口参数：x：x坐标
            y：y坐标
            color 此点的颜色值
* 返回参数：无
* 说    明：color:写入此点的颜色值   GUI调用该函数
****************************************************************************/
void LCD_Color_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_DrawPoint(x, y);
    LCD_CMD = write_gramcmd;
    LCD_DATA = color;
}

/****************************************************************************
 * 名    称: void Ssd1963_Set_BackLight(uint8_t BL_value)
 * 功    能：SSD1963 设置背光
 * 入口参数：BL_value：背光亮度大小  取值:0-255  设置255最亮
 * 返回参数：无
 * 说    明：
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
 * 名    称: void LCD_Clear(uint16_t color)
 * 功    能：清屏函数
 * 入口参数：color: 要清屏的填充色
 * 返回参数：无
 * 说    明：
 ****************************************************************************/
void LCD_Clear(uint16_t color)
{
    uint32_t i = 0;
    uint32_t pointnum = 0;

    pointnum = lcd_width * lcd_height; // 得到LCD总点数
    LCD_SetCursor(0x00, 0x00);         // 设置光标位置
    LCD_WriteGRAM();                   // 开始写入GRAM
    for (i = 0; i < pointnum; i++)
    {
        LCD_DATA = color;
    }
}

uint16_t ILI9341_Read_id(void)
{
    uint16_t id;

    LCD_CMD = 0xD3; // 9341读ID命令
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

    LCD_CMD = (0xA1); // 1963读ID命令
    id = LCD_DATA;
    id = LCD_DATA; // 0x57
    id <<= 8;
    id |= LCD_DATA; // 0x61

    return id;
}

// 初始化lcd
void LCD_Init(void)
{
    lcd_id = ILI9341_Read_id(); // 先读看看所接屏幕是不是9341驱动

    if (lcd_id != 0x9341) // 如果不是9341，读看看是不是1963驱动
    {
        lcd_id = SSD1963_Read_id();
        if (lcd_id == 0x5761)
            lcd_id = 0x1963; // SSD1963实际读出的ID是0x5761,为了直观，这边设置为1963
    }

    if (lcd_id == 0X9341) // 此驱动,设置写时序为最快
    {
        FSMC_Bank1E->BWTR[6] &= ~(0XF << 0); // 地址建立时间清零
        FSMC_Bank1E->BWTR[6] &= ~(0XF << 8); // 数据保存时间清零
        FSMC_Bank1E->BWTR[6] |= 3 << 0;      // 地址建立时间为3个HCLK =18ns
        FSMC_Bank1E->BWTR[6] |= 2 << 8;      // 数据保存时间为6ns*3个HCLK=18ns
    }

    if (lcd_id == 0X9341) // 9341初始化
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

        // LCD_BACK = 1; // 点亮背光
        HAL_GPIO_WritePin(LCD_BACK_GPIO_Port, LCD_BACK_Pin, GPIO_PIN_SET);
    }
    Set_Display_Mode(0); // 初始化为竖屏
    LCD_Clear(WHITE);    // 清屏白色
}

/****************************************************************************
* 名    称: void LCD_Fill_onecolor(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)  启*明@欣#欣
* 功    能：在指定区域内填充单个颜色
* 入口参数：(sx,sy),(ex,ey):填充矩形对角坐标
            color:要填充的颜色
* 返回参数：无
* 说    明：区域大小为:(ex-sx+1)*(ey-sy+1)  
****************************************************************************/
void LCD_Fill_onecolor(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color)
{
    uint16_t i, j;
    uint16_t nlen = 0;

    nlen = ex - sx + 1;
    for (i = sy; i <= ey; i++)
    {
        LCD_SetCursor(sx, i); // 设置光标位置
        LCD_WriteGRAM();      // 开始写入GRAM
        for (j = 0; j < nlen; j++)
            LCD_DATA = color; // 设置光标位置
    }
}

/****************************************************************************
* 名    称: void LCD_Draw_Picture(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
* 功    能：在指定区域内画入图片
* 入口参数：(sx,sy),(ex,ey):填充矩形对角坐标
            color:要填充的图片像素颜色数组
* 返回参数：无
* 说    明：区域大小为:(ex-sx+1)*(ey-sy+1)  
****************************************************************************/
void LCD_Draw_Picture(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
    uint16_t height, width;
    uint16_t i, j;
    width = ex - sx + 1;  // 得到图片的宽度
    height = ey - sy + 1; // 得到图片的高度
    for (i = 0; i < height; i++)
    {
        LCD_SetCursor(sx, sy + i); // 设置光标位置
        LCD_WriteGRAM();           // 开始写入GRAM
        for (j = 0; j < width; j++)
            LCD_DATA = color[i * height + j]; // 写入颜色值
    }
}

/****************************************************************************
* 名    称: void LCD_DisplayChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size)
* 功    能：在指定位置显示一个字符
* 入口参数：x,y:起始坐标
            word:要显示的字符:abcdefg1234567890...
            size:字体大小 12/16/24
* 返回参数：无
* 说    明：该字模取模方向为先从左到右，再从上到下  低位在前  
****************************************************************************/
void LCD_DisplayChar(uint16_t x, uint16_t y, uint8_t word, uint8_t size)
{
    uint8_t bytenum, bytedata, a, b;

    uint16_t xmid = x; // 存储初始X值(位置)

    if (size == 12)
        bytenum = 12; // 从字库数组中可知道每种字体单个字符所占的字节数
    else if (size == 16)
        bytenum = 16;
    else if (size == 24)
        bytenum = 48;
    else
        return; // 其他字体退出

    word = word - ' '; // 字库数组是按ASCII表排列
    // cfont.h中字库是从空格开始的 空格就是第一个元素 其他字符的ASCII码减去空格后就得到在数组中的偏移值(位置)
    for (b = 0; b < bytenum; b++)
    {
        if (size == 12)
            bytedata = asc2_1206[word][b]; // 调用1206字体
        else if (size == 16)
            bytedata = asc2_1608[word][b]; // 调用1608字体
        else if (size == 24)
            bytedata = asc2_2412[word][b]; // 调用2412字体

        for (a = 0; a < 8; a++)
        {
            if (bytedata & 0x01)
                LCD_Color_DrawPoint(x, y, BRUSH_COLOR); // 由于子模是低位在前 所以先从低位判断  为1时显示画笔颜色
            else
                LCD_Color_DrawPoint(x, y, BACK_COLOR); // 0时显示背景颜色
            bytedata >>= 1;                            // 低位判断完 依次往高位判断
            x++;                                       // 显示完一位 往下一位显示
            if ((x - xmid) == size / 2)                // x方向超出字体大小 如：16字体 实际是 08*16的点阵  故这边 size/2
            {
                x = xmid; // 从初始X位置在写下一行
                y++;      // 上一行写完 从下一行再写
                break;    // 跳出for(a=0;a<8;a++)循环
            }
        }
    }
}

/****************************************************************************
 * 名    称: void LCD_DisplayString(uint16_t x,uint16_t y,uint8_t size,uint8_t *p)
 * 功    能：显示字符串
 * 入口参数：x,y:起点坐标
 *           size:字体大小
 *           *p:字符串起始地址
 * 返回参数：无
 * 说    明：  
 ****************************************************************************/
void LCD_DisplayString(uint16_t x, uint16_t y, uint8_t size, uint8_t *p)
{
    while ((*p >= ' ') && (*p <= '~')) // 只显示“ ”到“~”之间的字符
    {
        LCD_DisplayChar(x, y, *p, size);
        x += size / 2;
        if (x >= lcd_width)
            break;
        p++;
    }
}

/****************************************************************************
 * 名    称: void LCD_DisplayString(uint16_t x,uint16_t y,uint8_t size,uint8_t *p)
 * 功    能：显示自定义字符串
 * 入口参数：x,y:起点坐标
 *           width,height:区域大小
 *           size:字体大小
 *           *p:字符串起始地址
 *           brushcolor：自定义画笔颜色
 *           backcolor： 自定义背景颜色
 * 返回参数：无
 * 说    明：  
 ****************************************************************************/
void LCD_DisplayString_color(uint16_t x, uint16_t y, uint8_t size, uint8_t *p, uint16_t brushcolor, uint16_t backcolor)
{
    uint16_t bh_color, bk_color;

    bh_color = BRUSH_COLOR; // 暂存画笔颜色
    bk_color = BACK_COLOR;  // 暂存背景颜色

    BRUSH_COLOR = brushcolor;
    BACK_COLOR = backcolor;

    LCD_DisplayString(x, y, size, p);

    BRUSH_COLOR = bh_color; // 不改变系统颜色
    BACK_COLOR = bk_color;
}

// a^n函数，返回值:a^n次方
uint32_t Counter_Power(uint8_t a, uint8_t n)
{
    uint32_t mid = 1;
    while (n--)
        mid *= a;
    return mid;
}

/****************************************************************************
* 名    称: void LCD_DisplayNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
* 功    能：在指定位置显示一串数字
* 入口参数：x,y:起点坐标
            value:数值;
            len:长度(设置显示的位数)
            size:字体大小
            mode: 0：高位为0不显示
                  1：高位为0根据len长度补显示几个0
* 返回参数：无
* 说    明：  
****************************************************************************/
void LCD_DisplayNum(uint16_t x, uint16_t y, uint32_t value, uint8_t len, uint8_t size, uint8_t mode)
{
    uint8_t t, numtemp;
    uint8_t value_num; // 数值总共几位数
    uint32_t value_mid;

    value_mid = value; // 计算位数时不影响要显示的数值大小
    for (value_num = 0; value_mid > 0; value_num++)
    {
        value_mid /= 10;
    } // 执行完for，就知道要显示的数值为几位数

    if (value_num > len) // 数值位数大于设置位数，即显示区域不够 显示错误
    {
        LCD_DisplayString(x, y, size, "ERROR");
        return; // 退出函数
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
                numtemp = (value / Counter_Power(10, len - t - 1)) % 10; // 取出各位数值
                LCD_DisplayChar(x + (size / 2) * t, y, numtemp + '0', size);
            }
        }
    }
}

/****************************************************************************
* 名    称: void LCD_DisplayNum_color(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
* 功    能：在指定位置显示一串自定义颜色的数字  启#明*欣&欣
* 入口参数：x,y:起点坐标
            num:数值;
            len:长度(即要显示的位数)
            size:字体大小
            mode: 0：高位为0不显示
                  1：高位为0显示0
            brushcolor：自定义画笔颜色
            backcolor： 自定义背景颜色
* 返回参数：无
* 说    明：  
****************************************************************************/
void LCD_DisplayNum_color(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode, uint16_t brushcolor, uint16_t backcolor)
{
    uint16_t bh_color, bk_color;

    bh_color = BRUSH_COLOR; // 暂存画笔颜色
    bk_color = BACK_COLOR;  // 暂存背景颜色

    BRUSH_COLOR = brushcolor;
    BACK_COLOR = backcolor;

    LCD_DisplayNum(x, y, num, len, size, mode);

    BRUSH_COLOR = bh_color; // 不改变系统颜色
    BACK_COLOR = bk_color;
}
#endif
