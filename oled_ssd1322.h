

#ifndef OLED_SSD1322_H
#define OLED_SSD1322_H

#include <stdint.h>
#include <stdbool.h>

/* STM HAL handle (dışarıda tanımlı, main.c'de) */
#include "stm32h7xx_hal.h"
extern SPI_HandleTypeDef hspi2;

/* Kontrol pinleri */
#define SSD1322_DC_Port     GPIOA
#define SSD1322_DC_Pin      GPIO_PIN_9

#define SSD1322_RST_Port    GPIOA
#define SSD1322_RST_Pin     GPIO_PIN_10

#define SSD1322_CS_Port     GPIOB
#define SSD1322_CS_Pin      GPIO_PIN_2

/* Optional debug pin (örneğin PA8) */
#define DEBUG_PIN_PORT GPIOA
#define DEBUG_PIN_PIN  GPIO_PIN_8

/* Display alanı (128x64) */
#define COLUMN_START 0x00
#define COLUMN_END   0x7F
#define ROW_START    0x00
#define ROW_END      0x3F




/* SPI retry */
#define SSD1322_SPI_RETRY_MAX 3

/* Font / drawing */
void SSD1322_DrawChar(int x, int y, char c);
void SSD1322_DrawStringCentered(const char *s);
void SSD1322_DrawStringAtOffset(const char *s, int y, int offset);

/* Scrolling line helper */
typedef struct {
    char text[64];
    int text_pixel_width;
    int offset;
    int direction; // 1 = sola, -1 = sağa
    int y;
} scrolling_line_t;

void ScrollLine_Init(scrolling_line_t *line, const char *fmt, int y);
void ScrollLine_Tick(scrolling_line_t *line);

/* Core API */
void SSD1322_Init(void);
void SSD1322_Clear(void);
void SSD1322_DisplayOnOff(bool on);
void SSD1322_RefreshFromFramebuffer(void);
void SSD1322_EntireDisplayOn(void);
void SSD1322_EntireDisplayOff(void);

void SSD1322_SendCommand(uint8_t cmd);
void SSD1322_SendCommandWithData(uint8_t cmd, const uint8_t *data, uint16_t len);
void SSD1322_ClearFramebuffer(void);
void SSD1322_SetColumn(uint8_t a, uint8_t b);
void SSD1322_SetRow(uint8_t a, uint8_t b);
void SSD1322_DrawGridTest(void);
void SSD1322_SetPixel(int x, int y, uint8_t gray);
void pixel_grid_test(void);


/* Image / logo */
void SSD1322_DisplayImage(const uint8_t *img);

/* Self-test (isteğe bağlı, remap vs denemesi) */
void SSD1322_SelfTestRemap(void);
void SSD1322_FillTestPattern(void);

/* Gerekirse logon */
extern const uint8_t NHD_Logo[];

#endif /* OLED_SSD1322_H */
