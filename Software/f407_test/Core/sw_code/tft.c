/* tft_driver.c */
#include "tft.h"
#include "font.h"
#include <string.h>

// 注意：后续你需要自己建一个 font.h 文件并包含进来，里面存放字模数组
// #include "font.h" 

/* ================= 1. 私有底层函数声明 (不对外公开) ================= */
static void TFT_HardwareReset(void);
static void TFT_SPI_SendByte(uint8_t byte);
static void TFT_WriteCmd(uint8_t cmd);
static void TFT_WriteData(uint8_t data);
static void TFT_SetWindow(uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end);


/* ================= 2. 底层硬件与通信实现 ================= */
static void TFT_SPI_SendByte(uint8_t byte) {
    HAL_SPI_Transmit(&hspi2, &byte, 1, 100); 
}

static void TFT_WriteCmd(uint8_t cmd) {
    TFT_CS_CLR();        
    TFT_DC_CMD();        
    TFT_SPI_SendByte(cmd); 
    TFT_CS_SET();        
}

static void TFT_WriteData(uint8_t data) {
    TFT_CS_CLR();
    TFT_DC_DATA();       
    TFT_SPI_SendByte(data);
    TFT_CS_SET();
}

static void TFT_HardwareReset(void) {
    TFT_RES_CLR();
    HAL_Delay(50); 
    TFT_RES_SET();
    HAL_Delay(50); 
}

static void TFT_SetWindow(uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end) {
    // 物理屏幕坐标偏移量校准
    x_start += 2; 
    x_end   += 2;
    y_start += 1;
    y_end   += 1;

    TFT_WriteCmd(0x2A); 
    TFT_WriteData(0x00);
    TFT_WriteData(x_start);
    TFT_WriteData(0x00);
    TFT_WriteData(x_end);

    TFT_WriteCmd(0x2B); 
    TFT_WriteData(0x00);
    TFT_WriteData(y_start);
    TFT_WriteData(0x00);
    TFT_WriteData(y_end);

    TFT_WriteCmd(0x2C); // 准备接收显存数据
}


/* ================= 3. 核心初始化序列 ================= */
void TFT_Init(void) {
    TFT_HardwareReset(); 
    TFT_WriteCmd(0x11);  
    HAL_Delay(120);      

    // ST7735S 帧率控制
    TFT_WriteCmd(0xB1); TFT_WriteData(0x05); TFT_WriteData(0x3C); TFT_WriteData(0x3C); 
    TFT_WriteCmd(0xB2); TFT_WriteData(0x05); TFT_WriteData(0x3C); TFT_WriteData(0x3C); 
    TFT_WriteCmd(0xB3); TFT_WriteData(0x05); TFT_WriteData(0x3C); TFT_WriteData(0x3C); TFT_WriteData(0x05); TFT_WriteData(0x3C); TFT_WriteData(0x3C); 
    // 点反转控制
    TFT_WriteCmd(0xB4); TFT_WriteData(0x03); 
    // 电源控制序列
    TFT_WriteCmd(0xC0); TFT_WriteData(0x28); TFT_WriteData(0x08); TFT_WriteData(0x04); 
    TFT_WriteCmd(0xC1); TFT_WriteData(0xC0); 
    TFT_WriteCmd(0xC2); TFT_WriteData(0x0D); TFT_WriteData(0x00); 
    TFT_WriteCmd(0xC3); TFT_WriteData(0x8D); TFT_WriteData(0x2A); 
    TFT_WriteCmd(0xC4); TFT_WriteData(0x8D); TFT_WriteData(0xEE); 
    // VCOM 电压控制
    TFT_WriteCmd(0xC5); TFT_WriteData(0x1A); 
    // 方向与色彩模式设置
    TFT_WriteCmd(0x36); TFT_WriteData(0xC0); 
    // 像素格式设置 (16-bit/pixel)
    TFT_WriteCmd(0x3A); TFT_WriteData(0x05); 

    TFT_WriteCmd(0x29); 
    HAL_Delay(20);
    TFT_BL_ON();
}


/* ================= 4. 高层图形与文本应用 ================= */

/**
 * @brief  全屏填充
 */
void TFT_Fill(uint16_t color) {
    uint32_t i;
    uint8_t data[2] = {color >> 8, color & 0xFF}; 
    
    TFT_SetWindow(0, 0, 127, 159); 
    
    TFT_DC_DATA(); 
    TFT_CS_CLR();  
    for(i = 0; i < 20480; i++) {
        HAL_SPI_Transmit(&hspi2, data, 2, 100); 
    }
    TFT_CS_SET();  
}

/**
 * @brief  在指定坐标画一个像素点 (文字显示的基础)
 */
void TFT_DrawPixel(uint8_t x, uint8_t y, uint16_t color) {
    if(x >= 128 || y >= 160) return; // 边界保护
    
    TFT_SetWindow(x, y, x, y); // 窗口设为1个像素大小
    
    TFT_DC_DATA();
    TFT_CS_CLR();
    
    uint8_t data[2] = {color >> 8, color & 0xFF};
    HAL_SPI_Transmit(&hspi2, data, 2, 100);
    
    TFT_CS_SET();
}

/**
 * @brief  显示单个 ASCII 字符 (目前为框架，需配合字库数组使用)
 * @param  x, y  起始坐标
 * @param  ch    要显示的字符 (如 'A')
 * @param  fc    前景色 (字体颜色)
 * @param  bc    背景色
 */
void TFT_ShowChar(uint8_t x, uint8_t y, char ch, uint16_t fc, uint16_t bc) {
    uint8_t i, j;
    uint8_t temp;
    
    // 边界保护：如果字符不在我们提取的范围内(空格到~)，直接忽略
    if (ch < ' ' || ch > '~') return;
    
    // 计算数组的索引偏移量
    ch = ch - ' '; 
    
    // 纵向取模渲染逻辑 (按列绘制)
    for (j = 0; j < 8; j++) { // 遍历 8 列
    
        // 1. 渲染该列的上半截 (y 到 y+7)
        temp = ascii_font[(uint8_t)ch][j]; 
        for (i = 0; i < 8; i++) {
            if (temp & 0x01) { // 纵向取模通常低位在上，所以判断 0x01
                TFT_DrawPixel(x + j, y + i, fc);
            } else {
                TFT_DrawPixel(x + j, y + i, bc);
            }
            temp >>= 1; // 数据右移，准备画下一个点
        }
        
        // 2. 渲染该列的下半截 (y+8 到 y+15)
        temp = ascii_font[(uint8_t)ch][j + 8]; 
        for (i = 0; i < 8; i++) {
            if (temp & 0x01) {
                TFT_DrawPixel(x + j, y + i + 8, fc);
            } else {
                TFT_DrawPixel(x + j, y + i + 8, bc);
            }
            temp >>= 1;
        }
    }
}
/**
 * @brief  显示字符串
 */
void TFT_ShowString(uint8_t x, uint8_t y, const char *str, uint16_t fc, uint16_t bc) {
    while(*str != '\0') {
        if(x > 128 - 8) { // 假设字宽为8，如果超出版面则换行
            x = 0;
            y += 16;      // 假设字高为16
        }
        if(y > 160 - 16) break; // 超出屏幕高度则停止
        
        TFT_ShowChar(x, y, *str, fc, bc);
        x += 8; // 移动到下一个字符位置
        str++;
    }
}



/**
 * @brief  专属纯中文字符串显示函数 (支持像写代码一样直接填中文)
 * @param  x, y  起始坐标
 * @param  str   必须传入纯中文的字符串 
 * @param  fc    前景色
 * @param  bc    背景色
 */
/**
 * @brief  专属纯中文字符串显示函数 (已适配：横向逐行取模、高位在前)
 */
void TFT_ShowChineseStr(uint8_t x, uint8_t y, const char *str, uint16_t fc, uint16_t bc) 
{
    uint8_t i, j, k;
    uint8_t temp;
    
    while (*str != '\0') 
    {
        // 截取当前汉字 (UTF-8 编码占用3个字节)
        char current_ch[4] = {str[0], str[1], str[2], '\0'};
        
        for (k = 0; k < CH_FONT_COUNT; k++) 
        {
            if (strncmp(current_ch, CH_Font[k].Index, 3) == 0) // 查字典成功
            {
                // === 全新渲染逻辑：横向逐行扫描 ===
                for (i = 0; i < 16; i++) // 汉字高度16，遍历16行
                { 
                    // 1. 画左半边的 8 个像素 (读取偶数索引的字节)
                    temp = CH_Font[k].Msk[i * 2]; 
                    for (j = 0; j < 8; j++) {
                        if (temp & 0x80) TFT_DrawPixel(x + j, y + i, fc); // 高位在前，判断 0x80
                        else TFT_DrawPixel(x + j, y + i, bc);
                        temp <<= 1; // 左移一位
                    }
                    
                    // 2. 画右半边的 8 个像素 (读取奇数索引的字节)
                    temp = CH_Font[k].Msk[i * 2 + 1]; 
                    for (j = 0; j < 8; j++) {
                        if (temp & 0x80) TFT_DrawPixel(x + j + 8, y + i, fc); // 注意 X 坐标加了 8
                        else TFT_DrawPixel(x + j + 8, y + i, bc);
                        temp <<= 1;
                    }
                }
                break; // 画完跳出字典循环
            }
        }
        
        str += 3; // 指针后移3个字节
        x += 16;  // 光标右移16像素
    }
}


