/* oled_ssd1322.c */

#include <string.h>
#include <stdio.h>
#include "oled_ssd1322.h"
#include "font6x8.h"

/* Eğer logonuz büyükse, extern olarak alın */
extern const uint8_t NHD_Logo[];

/* Inline kontrol helper'ları */
static inline void CS_LOW (void) { HAL_GPIO_WritePin(SSD1322_CS_Port,  SSD1322_CS_Pin,  GPIO_PIN_RESET); }
static inline void CS_HIGH(void) { HAL_GPIO_WritePin(SSD1322_CS_Port,  SSD1322_CS_Pin,  GPIO_PIN_SET);   }
static inline void DC_CMD (void) { HAL_GPIO_WritePin(SSD1322_DC_Port,  SSD1322_DC_Pin,  GPIO_PIN_RESET); }
static inline void DC_DAT (void) { HAL_GPIO_WritePin(SSD1322_DC_Port,  SSD1322_DC_Pin,  GPIO_PIN_SET);   }
static inline void DEBUG_TOGGLE(void) { HAL_GPIO_TogglePin(DEBUG_PIN_PORT, DEBUG_PIN_PIN); }
static inline void DEBUG_HIGH(void) { HAL_GPIO_WritePin(DEBUG_PIN_PORT, DEBUG_PIN_PIN, GPIO_PIN_SET); }
static inline void DEBUG_LOW(void)  { HAL_GPIO_WritePin(DEBUG_PIN_PORT, DEBUG_PIN_PIN, GPIO_PIN_RESET); }

/* SPI ile gönderim (retry) */
static HAL_StatusTypeDef ssd1322_spi_tx(const uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef ret;
    for (int attempt = 0; attempt < SSD1322_SPI_RETRY_MAX; ++attempt) {
        ret = HAL_SPI_Transmit(&hspi2, (uint8_t*)data, len, 100);
        if (ret == HAL_OK) return HAL_OK;
        HAL_Delay(1);
    }
    return ret;
}


void SSD1322_EntireDisplayOn(void) {
    SSD1322_SendCommand(0xA5); // Entire display ON (tüm ekran beyaz)
}

void SSD1322_EntireDisplayOff(void) {
    SSD1322_SendCommand(0xA4); // Entire display OFF (normal)
}


/* Komut gönderimi */
void SSD1322_SendCommand(uint8_t cmd)
{
    DC_CMD();
    CS_LOW();
    ssd1322_spi_tx(&cmd, 1);
    CS_HIGH();
}

/* Komut + veri */
void SSD1322_SendCommandWithData(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    DC_CMD();
    CS_LOW();
    ssd1322_spi_tx(&cmd, 1);
    if (len) {
        DC_DAT();
        ssd1322_spi_tx(data, len);
    }
    CS_HIGH();
}

/* Reset palsi */
static void SSD1322_Reset(void)
{
    HAL_GPIO_WritePin(SSD1322_RST_Port, SSD1322_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(150);
    HAL_GPIO_WritePin(SSD1322_RST_Port, SSD1322_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(150);
}

/* Kolon/satır ayarları */
void SSD1322_SetColumn(uint8_t a, uint8_t b)
{
    SSD1322_SendCommandWithData(0x15, (uint8_t[]){a, b}, 2);
}

void SSD1322_SetRow(uint8_t a, uint8_t b)
{
    SSD1322_SendCommandWithData(0x75, (uint8_t[]){a, b}, 2);
}

/* Display ON/OFF */
void SSD1322_DisplayOnOff(bool on)
{
    if (on) SSD1322_SendCommand(0xAF);
    else   SSD1322_SendCommand(0xAE);
}

/* Başlatma sekansı */
void SSD1322_Init(void)
{
    SSD1322_Reset();

    SSD1322_DisplayOnOff(false);

    SSD1322_SendCommandWithData(0xFD, (uint8_t[]){0x12},1);    // Command Lock
    SSD1322_SendCommandWithData(0xB3, (uint8_t[]){0x91},1);    // Display Clock
    SSD1322_SendCommandWithData(0xCA, (uint8_t[]){0x3F},1);    // MUX Ratio
    SSD1322_SendCommandWithData(0xA2, (uint8_t[]){0x00},1);    // Display Offset
    SSD1322_SendCommandWithData(0xAB, (uint8_t[]){0x01},1);    // Function Select (internal VDD)
    SSD1322_SendCommandWithData(0xA1, (uint8_t[]){0x00},1);    // Start Line

    SSD1322_SendCommandWithData(0xA0, (uint8_t[]){0x16,0x11},2);  // Remap

    SSD1322_SendCommandWithData(0xC7, (uint8_t[]){0x0F},1);    // Master Contrast
    SSD1322_SendCommandWithData(0xC1, (uint8_t[]){0x9F},1);    // Contrast

    SSD1322_SendCommandWithData(0xB1, (uint8_t[]){0x72},1);    // Phase Length
    SSD1322_SendCommandWithData(0xBB, (uint8_t[]){0x1F},1);    // Precharge Voltage
    SSD1322_SendCommandWithData(0xB4, (uint8_t[]){0xA0,0xFD},2);// Display Enhancement A (VSL)
    SSD1322_SendCommandWithData(0xBE, (uint8_t[]){0x04},1);    // VCOMH

    SSD1322_SendCommand(0xA6);               // Normal Display
    SSD1322_SendCommand(0xA9);               // Exit Partial
    SSD1322_SendCommandWithData(0xD1, (uint8_t[]){0xA2,0x20},2); // Display Enhancement B
    SSD1322_SendCommandWithData(0xB5, (uint8_t[]){0x00},1);     // GPIO
    SSD1322_SendCommand(0xB9);               // Default Grayscale
    SSD1322_SendCommandWithData(0xB6, (uint8_t[]){0x08},1);     // 2nd Precharge

    SSD1322_DisplayOnOff(true);
}

/* Framebuffer: 2-bit grayscale (0..3), 64 satır x 128 kolon */
 uint8_t framebuf[64][128];

/* 2-bit -> byte mapping */
static inline uint8_t gray2byte(uint8_t g) {
    switch (g & 0x03) {
        case 0: return 0x00;
        case 1: return 0x55;
        case 2: return 0xAA;
        case 3: return 0xFF;
        default: return 0x00;
    }
}

/* Framebuffer'ı GDDRAM'a yazar */
void SSD1322_RefreshFromFramebuffer(void)
{
    SSD1322_SetColumn(COLUMN_START, COLUMN_END);
    SSD1322_SetRow(ROW_START, ROW_END);
    SSD1322_SendCommand(0x5C); // Write RAM

    uint8_t linebuf[256];
    for (int row = 0; row < 64; row++) {
        for (int col = 0; col < 128; col++) {
            uint8_t b = gray2byte(framebuf[row][col]);
            linebuf[col * 2 + 0] = b;
            linebuf[col * 2 + 1] = b;
        }
        DC_DAT();
        CS_LOW();
        ssd1322_spi_tx(linebuf, sizeof(linebuf));
        CS_HIGH();
    }
}

/* Ekranı framebuffer üzerinden temizle */
void SSD1322_Clear(void)
{
    for (int r=0; r<64; r++)
        for (int c=0; c<128; c++)
            framebuf[r][c] = 0;
    SSD1322_RefreshFromFramebuffer();
}

/* Basit karakter çizimi (6x8) */
void SSD1322_DrawChar(int x, int y, char c)
{
    if (c < 32 || c > 127) return;
    const uint8_t *glyph = Font6x8[c - 32];

    for (int col = 0; col < 6; col++) {
        int fx = x + col;
        if (fx < 0 || fx >= 128) continue;
        uint8_t column_bits = glyph[col];
        for (int row = 0; row < 8; row++) {
            int fy = y + row;
            if (fy < 0 || fy >= 64) continue;
            uint8_t pixel_on = (column_bits >> row) & 0x01;
            framebuf[fy][fx] = pixel_on ? 3 : 0;
        }
    }
}

/* Ortalanmış string (tek satır) */
void SSD1322_DrawStringCentered(const char *s)
{
    int len = 0;
    for (const char *p = s; *p; ++p) len++;
    int total_width = len * 6 + (len - 1) * 1;
    int x0 = (128 - total_width) / 2;
    int y0 = (64 - 8) / 2;

    /* temizle */
    for (int r=0;r<64;r++)
        for (int c=0;c<128;c++)
            framebuf[r][c]=0;

    for (int i = 0; i < len; i++) {
        int x = x0 + i * (6 + 1);
        SSD1322_DrawChar(x, y0, s[i]);
    }
    SSD1322_RefreshFromFramebuffer();
}

/* Offset ile kaydırmalı string çizimi (yatay scroll) */
void SSD1322_DrawStringAtOffset(const char *s, int y, int offset)
{
    // Sadece o satırı temizle
    for (int row = y; row < y + 8; row++)
        for (int col = 0; col < 128; col++)
            if (row >= 0 && row < 64)
                framebuf[row][col] = 0;

    int x = -offset;
    for (int i = 0; s[i]; i++) {
        SSD1322_DrawChar(x, y, s[i]);
        x += 7; // 6px + 1 boşluk
    }
}

/* Scroll line yapısı ve yönetimi */
void ScrollLine_Init(scrolling_line_t *line, const char *fmt, int y)
{
    snprintf(line->text, sizeof(line->text), fmt);
    int len = 0;
    for (const char *p = line->text; *p; ++p) len++;
    line->text_pixel_width = len * 7 - 1;
    line->offset = 0;
    line->direction = 1;
    line->y = y;
}

void ScrollLine_Tick(scrolling_line_t *line)
{
    if (line->text_pixel_width <= 128) {
        int pad = (128 - line->text_pixel_width) / 2;
        SSD1322_DrawStringAtOffset(line->text, line->y, -pad);
    } else {
        SSD1322_DrawStringAtOffset(line->text, line->y, line->offset);
        line->offset += line->direction;
        if (line->offset + 128 >= line->text_pixel_width) line->direction = -1;
        if (line->offset <= 0) line->direction = 1;
    }
}

/* Basit self-test (remap varyasyonları) */
void SSD1322_SelfTestRemap(void)
{
    const uint8_t remap_table[][2] = {
        {0x00, 0x00},
        {0x16, 0x11},
        {0x06, 0x11},
        {0x16, 0x01},
        {0x00, 0x11},
        {0x10, 0x11},
    };
    const int n = sizeof(remap_table) / sizeof(remap_table[0]);

    for (int i = 0; i < n; i++) {
        DEBUG_TOGGLE();
        SSD1322_SendCommandWithData(0xA0, (uint8_t[]){remap_table[i][0], remap_table[i][1]}, 2);
        DEBUG_TOGGLE();

        DEBUG_TOGGLE();
        // Basit desen: satır numarasına göre değişen
        for (int r=0;r<64;r++)
            for (int c=0;c<128;c++)
                framebuf[r][c] = (r + i) & 0x03;
        SSD1322_RefreshFromFramebuffer();
        DEBUG_TOGGLE();

        // Entire display ON / OFF
        SSD1322_SendCommand(0xA5);
        HAL_Delay(300);
        SSD1322_SendCommand(0xA4);
        HAL_Delay(300);

        DEBUG_TOGGLE();
        SSD1322_DisplayOnOff(false);
        HAL_Delay(200);
        SSD1322_DisplayOnOff(true);
        DEBUG_TOGGLE();

        HAL_Delay(300);
    }
}



void SSD1322_FillTestPattern(void)
{
    // 0..3 arasında artan mozaik
    for (int r = 0; r < 64; r++) {
        for (int c = 0; c < 128; c++) {
            framebuf[r][c] = (r + c) & 0x03;
        }
    }
    SSD1322_RefreshFromFramebuffer();
}




/* Basit logo gösterimi (orijinal format korunur) */
void SSD1322_DisplayImage(const uint8_t *img)
{
    SSD1322_SetColumn(COLUMN_START, COLUMN_END);
    SSD1322_SetRow(ROW_START, ROW_END);
    SSD1322_SendCommand(0x5C); // Write RAM

    uint8_t outbuf[256]; // genişlike göre satır buffer'ı (128*2)
    // Varsayım: gelen img uyumlu şekilde hazırlanmış (örneğin 64x64 veya remap'e uygun)
    for (int i = 0; i < 64 * 16; i++, img++) {
        uint8_t b = *img;
        int base_col = (i % 64) * 2;
        // Her 2-bit için linear mapping
        for (int sh = 6; sh >= 0; sh -= 2) {
            uint8_t px = (b >> sh) & 0x03;
            uint8_t val;
            switch (px) {
                case 0: val = 0x00; break;
                case 1: val = 0x55; break;
                case 2: val = 0xAA; break;
                case 3: val = 0xFF; break;
                default: val = 0x00; break;
            }
            // Her piksel çiftleniyor
            outbuf[base_col++] = val;
            outbuf[base_col++] = val;
        }
        DC_DAT();
        CS_LOW();
        ssd1322_spi_tx(outbuf, 128); // 64*2
        CS_HIGH();
    }
}

/* Framebuffer'ı sıfırlamak için helper */
void SSD1322_ClearFramebuffer(void)
{
    // static framebuf bu dosyada tanımlı olduğu için direkt erişebiliyoruz
    for (int r = 0; r < 64; r++) {
        for (int c = 0; c < 128; c++) {
            framebuf[r][c] = 0;
        }
    }
}





// Tek piksel koy
void SSD1322_SetPixel(int x, int y, uint8_t gray)
{
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    framebuf[y][x] = gray & 0x03; // 0..3
}


// Basit 16x16 aralıklarla grid testi
void SSD1322_DrawGridTest(void)
{
    SSD1322_ClearFramebuffer();
    for (int x = 0; x < 128; x += 16) {
        for (int y = 0; y < 64; y += 16) {
            SSD1322_SetPixel(x, y, 3); // beyaz nokta
        }
    }
    SSD1322_RefreshFromFramebuffer();
}



// Ekranda belli aralıklarla beyaz noktalar çizerek koordinat sistemini test eder.
void pixel_grid_test(void)
{
    SSD1322_ClearFramebuffer();

    // Her 16 pikselde bir nokta koy (hem x hem y)
    for (int y = 0; y < 64; y += 16) {
        for (int x = 0; x < 128; x += 16) {
            SSD1322_SetPixel(x, y, 3); // en parlak beyaz
        }
    }

    // Köşe kontrolleri (istersen ayrı)
    SSD1322_SetPixel(0, 0, 3);
    SSD1322_SetPixel(127, 0, 3);
    SSD1322_SetPixel(0, 63, 3);
    SSD1322_SetPixel(127, 63, 3);

    SSD1322_RefreshFromFramebuffer();
}

void draw_centered_at_y(const char *s, int y)
{
    int len = strlen(s);
    int total_width = len * 6 + (len + 1); // 6px karakter + 1px boşluk
    //    int x0 = (64 - total_width) / 2;
    int x0 = (128 - total_width) / 2;
    if (x0 < 0) x0 = 0; // sığmıyorsa sola yapıştır
    for (int i = 0; i < len; i++) {
        SSD1322_DrawChar(x0 + i * 7, y, s[i]); // 6px + 1px boşluk
    }
}
