# Enhanced SSD1322 OLED Driver Features

This document describes the enhanced features added to the SSD1322 OLED display driver, providing advanced font management, bitmap rendering, and display control capabilities.

## Overview

The enhanced driver maintains full backward compatibility while adding powerful new features:

- **Advanced Text Management**: Text size scaling, color control, and automatic wrapping
- **Bitmap Rendering**: 4bpp and 2bpp grayscale bitmap support
- **Cursor Management**: Printf-style text positioning and output
- **Dynamic Contrast Control**: Runtime adjustable display brightness
- **Comprehensive Self-Test**: Automated testing of all display features

## Enhanced Font Management

### Text Size Control
```c
SSD1322_SetTextSize(uint8_t size);    // Set text size (1-4x scaling)
uint8_t size = SSD1322_GetTextSize(); // Get current text size
```

### Text Color (Grayscale Level)
```c
SSD1322_SetTextColor(uint8_t color);  // Set text color (0-3, 0=black, 3=white)
uint8_t color = SSD1322_GetTextColor(); // Get current text color
```

### Text Wrapping
```c
SSD1322_SetTextWrap(bool wrap);       // Enable/disable automatic text wrapping
bool wrap = SSD1322_GetTextWrap();    // Get current wrap setting
```

### Cursor Management
```c
SSD1322_SetCursor(int x, int y);      // Set text cursor position
SSD1322_GetCursor(int *x, int *y);    // Get current cursor position
```

## Advanced Drawing Functions

### Enhanced String Drawing
```c
// Draw string at specific position with current settings
SSD1322_DrawString(int x, int y, const char *str);

// Print character at cursor position (advances cursor)
SSD1322_PrintChar(char c);

// Print string at cursor position (advances cursor)
SSD1322_PrintString(const char *str);

// Draw character with specific size and color
SSD1322_DrawCharAdvanced(int x, int y, char c, uint8_t size, uint8_t color);
```

## Bitmap Rendering

### 4-bit Grayscale Bitmaps
```c
// Render 4bpp bitmap (0-15 levels, auto-scaled to 0-3)
SSD1322_Draw4bppBitmap(int x, int y, int width, int height, const uint8_t *bitmap);
```

**Bitmap Format**: Each byte contains two 4-bit pixels (high nibble first)
- Bitmap size: `(width + 1) / 2 * height` bytes
- Pixel values: 0-15 (automatically scaled to display's 0-3 range)

### Direct 2-bit Grayscale Bitmaps
```c
// Render native 2bpp bitmap (optimal performance)
SSD1322_DrawGrayscaleBitmap(int x, int y, int width, int height, const uint8_t *bitmap);
```

**Bitmap Format**: Each byte contains four 2-bit pixels (MSB first)
- Bitmap size: `(width + 3) / 4 * height` bytes
- Pixel values: 0-3 (matches display's native format)

## Display Control

### Contrast Management
```c
SSD1322_SetContrast(uint8_t contrast);        // Set display contrast (0-255)
SSD1322_SetMasterContrast(uint8_t contrast);  // Set master contrast (0-15)
uint8_t contrast = SSD1322_GetContrast();     // Get current contrast
```

## Comprehensive Self-Test

```c
SSD1322_ComprehensiveSelfTest();  // Run complete functionality test
```

The self-test demonstrates:
1. Display initialization and text rendering
2. Text size variations (1x, 2x scaling)
3. All grayscale levels (0-3)
4. Text wrapping functionality
5. Bitmap rendering with test patterns
6. Contrast level adjustments
7. Pixel grid test
8. Fill pattern test

## Example Usage

### Basic Enhanced Text Display
```c
// Initialize display
SSD1322_Init();

// Clear and setup text
SSD1322_ClearFramebuffer();
SSD1322_SetTextSize(2);           // 2x size
SSD1322_SetTextColor(3);          // White
SSD1322_SetTextWrap(true);        // Enable wrapping
SSD1322_SetCursor(0, 0);          // Top-left corner

// Print text
SSD1322_PrintString("Hello World!\nSecond Line");
SSD1322_RefreshFromFramebuffer();
```

### Bitmap Display Example
```c
// Create a simple checkerboard pattern (4bpp format)
uint8_t checkerboard[] = {
    0xFF, 0x00, 0xFF, 0x00,  // White-Black-White-Black
    0x00, 0xFF, 0x00, 0xFF,  // Black-White-Black-White
    0xFF, 0x00, 0xFF, 0x00,  // White-Black-White-Black
    0x00, 0xFF, 0x00, 0xFF   // Black-White-Black-White
};

SSD1322_ClearFramebuffer();
SSD1322_Draw4bppBitmap(32, 16, 8, 4, checkerboard);
SSD1322_RefreshFromFramebuffer();
```

### Multi-Level Grayscale Text
```c
SSD1322_ClearFramebuffer();
SSD1322_SetTextSize(1);

for (int level = 0; level < 4; level++) {
    SSD1322_SetTextColor(level);
    SSD1322_SetCursor(0, level * 12);
    char msg[32];
    snprintf(msg, sizeof(msg), "Grayscale Level %d", level);
    SSD1322_PrintString(msg);
}

SSD1322_RefreshFromFramebuffer();
```

## Hardware Compatibility

- **Display**: 128x64 SSD1322 OLED
- **Interface**: SPI with STM32H7xx HAL
- **Color Depth**: 2-bit grayscale (4 levels: 0=black, 1=dark, 2=light, 3=white)
- **Memory**: Uses internal framebuffer for optimal performance

## Integration Notes

- All functions are thread-safe when used with proper SPI synchronization
- Framebuffer operations are fast; call `SSD1322_RefreshFromFramebuffer()` when ready to update display
- Enhanced features maintain full compatibility with existing code
- Text wrapping respects display boundaries and automatically handles line breaks
- Bitmap rendering includes bounds checking for safe operation

## Performance Considerations

- **Text Rendering**: Scaled text (size > 1) requires more processing time
- **Bitmap Rendering**: Direct 2bpp bitmaps are faster than 4bpp conversion
- **Memory Usage**: Framebuffer is 8KB (128 × 64 × 1 byte per pixel)
- **SPI Optimization**: Uses existing retry mechanism for reliable communication