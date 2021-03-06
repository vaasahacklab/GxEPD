// Class GxGDE060BA : display class for GDE060BA e-paper display from Dalian Good Display Inc.
//
// Created by Jean-Marc Zingg based on demo code from Good Display for red DESTM32-L board.
//
// The GxGDE060BA class is not board specific, but relies on FMSC SRAM for buffer memory, as available on DESTM32-L,
// and on a specific IO channel class for parallel connection e-paper displays.
// 
// The GxIO_DESTM32L is board specific and serves as IO channel for the display class.
//
// These classes can serve as an example for other boards for this e-paper display,
// however this is not easy, because of the e-paper specific supply voltages and big RAM buffer needed.
//
// To be used with "BLACK 407ZE (V3.0)" of "BLACK F407VE/ZE/ZG boards" of package "STM32GENERIC for STM32 boards" for Arduino.
// https://github.com/danieleff/STM32GENERIC
//
// The e-paper display and demo board is available from:
// http://www.buy-lcd.com/index.php?route=product/product&path=2897_10571&product_id=22833
// or https://www.aliexpress.com/store/product/Epaper-demo-kit-for-6-800X600-epaper-display-GDE060BA/600281_32812255729.html

#ifndef _GxGDE060BA_H
#define _GxGDE060BA_H

#include <Arduino.h>
#include "../GxEPD.h"
#include "../GxIO/GxIO_DESTM32L/GxIO_DESTM32L.h"

#define GxGDE060BA_WIDTH 800
#define GxGDE060BA_HEIGHT 600
#define GxGDE060BA_BUFFER_SIZE GxGDE060BA_WIDTH * GxGDE060BA_HEIGHT / 4 // 2bits per pixel
#define GxGDE060BA_CL_DELAY 13 // this value produces the same CL period as the hex demo code

#define GxGDE060BA_FRAME_BEGIN_SIZE  8
const unsigned char wave_begin_60[4][GxGDE060BA_FRAME_BEGIN_SIZE] =
{
  0, 0, 0, 0, 2, 2, 2, 0, //GC0->GC3
  0, 0, 0, 1, 2, 2, 2, 0, //GC1->GC3
  0, 0, 1, 1, 2, 2, 2, 0, //GC2->GC3
  0, 1, 1, 1, 2, 2, 2, 0, //GC3->GC3
};

#define GxGDE060BA_FRAME_END_SIZE  18
const unsigned char wave_end_60[4][GxGDE060BA_FRAME_END_SIZE] =
{
  0, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 0, 0, //GC3->GC0
  0, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0, //GC3->GC1
  0, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2, 2, 2, 1, 0, 0, 0, 0, //GC3->GC2
  0, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2, 2, 2, 0, 0, 0, 0, 0, //GC3->GC3
};

#define GxGDE060BA_ROW_BUFFER_SIZE (GxGDE060BA_WIDTH / 4)

// fixed, multiple of 64 and >= GxGDE060BA_ROW_BUFFER_SIZE
#define WAVE_TABLE_SIZE 256

typedef uint8_t epd_buffer_type[GxGDE060BA_BUFFER_SIZE];

struct fmsc_sram_type
{
  epd_buffer_type epd_sram_buffer1;
  epd_buffer_type epd_sram_buffer2;
};

#define FMSC_SRAM ((fmsc_sram_type*)0x68000000) // NE3 PG10 on DESTM32-L

class GxGDE060BA : public GxEPD
{
  public:
    GxGDE060BA(GxIO_DESTM32L& io);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(void);
    void fillScreen(uint16_t color); // to buffer
    void update(void);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size);
    // to buffer, may be cropped, drawPixel() used, update needed
    void  drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    // undo last drawBitmap to prepare for next drawBitmap (turn display white);
    // any bitmap can be used, but real last bitmap gives slightly better result
    void eraseBitmap(const uint8_t *bitmap, uint32_t size);
    void eraseDisplay(); // alternative to eraseBitmap, without bitmap
    void DisplayTestPicture(uint8_t nr);
    void fillScreenTest();
  private:
    void init_wave_table(void);
  private:
    uint8_t wave_begin_table[WAVE_TABLE_SIZE][GxGDE060BA_FRAME_BEGIN_SIZE];
    uint8_t wave_end_table[WAVE_TABLE_SIZE][GxGDE060BA_FRAME_END_SIZE];
    uint8_t row_buffer[GxGDE060BA_ROW_BUFFER_SIZE];
    epd_buffer_type* p_active_buffer;
    epd_buffer_type* p_erase_buffer;
    GxIO_DESTM32L& IO;
};

#define GxEPD_Class GxGDE060BA

#define GxEPD_WIDTH GxGDE060BA_WIDTH
#define GxEPD_HEIGHT GxGDE060BA_HEIGHT
#define GxEPD_BitmapExamples <GxGDE060BA/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDE060BA/BitmapExamples.h"

#endif
