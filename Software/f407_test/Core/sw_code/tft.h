/* tft_driver.h */
#ifndef __TFT_H
#define __TFT_H

#include "main.h"

/* ================= 1. 硬件引脚映射区 ================= */
#define TFT_BL_PORT   GPIOD
#define TFT_BL_PIN    GPIO_PIN_8

#define TFT_CS_PORT   GPIOB
#define TFT_CS_PIN    GPIO_PIN_12

#define TFT_DC_PORT   GPIOD
#define TFT_DC_PIN    GPIO_PIN_9

#define TFT_RES_PORT  GPIOD
#define TFT_RES_PIN   GPIO_PIN_10

/* 底层引脚操作宏 */
#define TFT_CS_CLR()  HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_RESET)
#define TFT_CS_SET()  HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_SET)

#define TFT_DC_CMD()  HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_RESET)
#define TFT_DC_DATA() HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_SET)  

#define TFT_RES_CLR() HAL_GPIO_WritePin(TFT_RES_PORT, TFT_RES_PIN, GPIO_PIN_RESET)
#define TFT_RES_SET() HAL_GPIO_WritePin(TFT_RES_PORT, TFT_RES_PIN, GPIO_PIN_SET)

#define TFT_BL_OFF()  HAL_GPIO_WritePin(TFT_BL_PORT, TFT_BL_PIN, GPIO_PIN_RESET)
#define TFT_BL_ON()   HAL_GPIO_WritePin(TFT_BL_PORT, TFT_BL_PIN, GPIO_PIN_SET)

/* ================= 2. 常用颜色定义 (RGB565) ================= */
#define TFT_WHITE   0xFFFF
#define TFT_BLACK   0x0000
#define TFT_RED     0xF800
#define TFT_GREEN   0x07E0
#define TFT_BLUE    0x001F
#define TFT_YELLOW  0xFFE0

/* ================= 3. 全局外部引用 ================= */
extern SPI_HandleTypeDef hspi2;

/* ================= 4. 对外开放的应用层函数原型 ================= */
// 基础控制
void TFT_Init(void);
// 图形绘制
void TFT_Fill(uint16_t color);
void TFT_DrawPixel(uint8_t x, uint8_t y, uint16_t color);
// 文字显示 (需要配合字模数组)
void TFT_ShowChar(uint8_t x, uint8_t y, char ch, uint16_t fc, uint16_t bc);
void TFT_ShowString(uint8_t x, uint8_t y, const char *str, uint16_t fc, uint16_t bc);
void TFT_ShowChineseStr(uint8_t x, uint8_t y, const char *str, uint16_t fc, uint16_t bc);
#endif