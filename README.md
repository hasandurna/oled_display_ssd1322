# SSD1322 OLED Display Driver

Enhanced SSD1322 OLED display driver for STM32 microcontrollers with advanced font management and bitmap rendering capabilities.

## Features

### Core Features
- 128x64 SSD1322 OLED support via SPI interface
- 2-bit grayscale framebuffer (4 levels: black, dark gray, light gray, white)
- Optimized SPI communication with retry mechanism
- STM32H7xx HAL integration

### Enhanced Features ✨
- **Advanced Font Management**: Scalable text (1x-4x), color control, automatic wrapping
- **Bitmap Rendering**: 4bpp and 2bpp grayscale image support
- **Cursor Management**: Printf-style text positioning and output
- **Dynamic Contrast Control**: Runtime adjustable display brightness  
- **Comprehensive Self-Test**: Automated testing of all display features

## Quick Start

```c
#include "oled_ssd1322.h"

int main(void) {
    // Initialize display
    SSD1322_Init();
    
    // Enhanced text rendering
    SSD1322_SetTextSize(2);           // 2x scaling
    SSD1322_SetTextColor(3);          // White
    SSD1322_SetCursor(0, 0);
    SSD1322_PrintString("Hello\nWorld!");
    SSD1322_RefreshFromFramebuffer();
    
    // Run comprehensive test
    SSD1322_ComprehensiveSelfTest();
    
    while(1) { /* your code */ }
}
```

## Documentation

- [`ENHANCED_FEATURES.md`](ENHANCED_FEATURES.md) - Complete API reference and examples
- Hardware datasheet: `SSD1322.pdf`
- Display specification: `NHD-2.7-12864WDW3-M.pdf`

## Hardware Requirements

- STM32H7xx series microcontroller
- 128x64 SSD1322 OLED display
- SPI interface connection
- Control pins: DC, RST, CS

## Compatibility

The enhanced driver maintains full backward compatibility with existing code while adding powerful new features. All original functions continue to work as before.