/************************************************************************************
   class GxGDEP015OC1 : Display class example for GDEP015OC1 e-Paper from GoodDisplay.com

   based on Demo Example from GoodDisplay.com, avalable with any order for such a display, no copyright notice.

   Author : J-M Zingg

   modified by :

   Version : 2.0

   Support: minimal, provided as example only, as is, no claim to be fit for serious use

   connection to the e-Paper display is through DESTM32-S2 connection board, available from GoodDisplay

   DESTM32-S2 pinout (top, component side view):
       |-------------------------------------------------
       |  VCC  |o o| VCC 5V
       |  GND  |o o| GND
       |  3.3  |o o| 3.3V
       |  nc   |o o| nc
       |  nc   |o o| nc
       |  nc   |o o| nc
       |  MOSI |o o| CLK
       |  DC   |o o| D/C
       |  RST  |o o| BUSY
       |  nc   |o o| BS
       |-------------------------------------------------
*/
#include "GxGDEP015OC1.h"
#include "BitmapExamples.h"

#if defined(__AVR)
#include <avr/pgmspace.h>
#endif

#define xPixelsPar (GxGDEP015OC1_X_PIXELS - 1)
#define yPixelsPar (GxGDEP015OC1_Y_PIXELS - 1)

// old version
//const uint8_t LUTDefault_part[31] =
//{
//  0x32,  // command
//  0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//  0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
//};

const uint8_t LUTDefault_part[31] =
{
  0x32,	// command //C221 25C partial update waveform
  0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// old version
//const uint8_t LUTDefault_full[31] =
//{
//  0x32,  // command
//  0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22, 0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99,
//  0x88, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51, 0x35, 0x51, 0x51, 0x19, 0x01, 0x00
//};

const uint8_t LUTDefault_full[31] = {
  0x32,	// command //C221 25C Full update waveform
  0x50, 0xAA, 0x55, 0xAA, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// old version
//uint8_t GDOControl[] = {0x01, 0xC7, 0x00, 0x00}; //for 1.54inch
//uint8_t softstart[] = {0x0c, 0xd7, 0xd6, 0x9d};
//uint8_t VCOMVol[] = {0x2c, 0xa8};  // VCOM 7c
//uint8_t DummyLine[] = {0x3a, 0x1a}; // 4 dummy line per gate
//uint8_t Gatetime[] = {0x3b, 0x08};  // 2us per line
//uint8_t RamDataEntryMode[] = {0x11, 0x01};  // Ram data entry mode

uint8_t GDOControl[] = {0x01, yPixelsPar % 256, yPixelsPar / 256, 0x00}; //for 1.54inch
uint8_t softstart[] = {0x0c, 0xd7, 0xd6, 0x9d};
uint8_t Rambypass[] = {0x21, 0x8f};   // Display update
uint8_t MAsequency[] = {0x22, 0xf0};  // clock
uint8_t GDVol[] = {0x03, 0x00}; // Gate voltage +15V/-15V
uint8_t SDVol[] = {0x04, 0x0a}; // Source voltage +15V/-15V
uint8_t VCOMVol[] = {0x2c, 0x9b}; // VCOM 7c
uint8_t BOOSTERFB[] = {0xf0, 0x1f}; // Source voltage +15V/-15V
uint8_t DummyLine[] = {0x3a, 0x1a}; // 4 dummy line per gate
uint8_t Gatetime[] = {0x3b, 0x08}; // 2us per line
uint8_t BorderWavefrom[] = {0x3c, 0x33}; // Border
uint8_t RamDataEntryMode[] = {0x11, 0x01}; // Ram data entry mode

GxGDEP015OC1::GxGDEP015OC1(GxIO& io, uint8_t rst, uint8_t busy) :
  GxEPD(GxGDEP015OC1_WIDTH, GxGDEP015OC1_HEIGHT),
  IO(io), _rst(rst), _busy(busy),
  _current_page(-1), _using_partial_mode(false)
{
}

template <typename T> static inline void
swap(T& a, T& b)
{
  T t = a;
  a = b;
  b = t;
}

void GxGDEP015OC1::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEP015OC1_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEP015OC1_WIDTH - x - 1;
      y = GxGDEP015OC1_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEP015OC1_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEP015OC1_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_buffer)) return;
  }
  else
  {
    if (i < GxGDEP015OC1_PAGE_SIZE * _current_page) return;
    if (i >= GxGDEP015OC1_PAGE_SIZE * (_current_page + 1)) return;
    i -= GxGDEP015OC1_PAGE_SIZE * _current_page;
  }

  if (!color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}

void GxGDEP015OC1::init(void)
{
  IO.init();
  IO.setFrequency(4000000); // 4MHz : 250ns > 150ns min RD cycle
  digitalWrite(_rst, HIGH);
  pinMode(_rst, OUTPUT);
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
}

void GxGDEP015OC1::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void GxGDEP015OC1::update(void)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _Init_Full();
  _writeCommand(0x24);
  for (uint16_t y = 0; y < GxGDEP015OC1_HEIGHT; y++)
  {
    for (uint16_t x = GxGDEP015OC1_WIDTH / 8; x > 0; x--)
    {
      uint16_t idx = y * (GxGDEP015OC1_WIDTH / 8) + x - 1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      uint8_t mirror = 0x00;
      for (uint8_t i = 0; i < 8; i++)
      {
        mirror |= ((data >> i) & 0x01) << (7 - i);
      }
      _writeData(~mirror);
    }
  }
  _Update_Full();
  _PowerOff();
}

void  GxGDEP015OC1::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  for (uint16_t x1 = x; x1 < x + w; x1++)
  {
    for (uint16_t y1 = y; y1 < y + h; y1++)
    {
      uint16_t i = x1 / 8 + y1 * w / 8;
#if defined(__AVR)
      uint16_t pixelcolor = (pgm_read_byte(bitmap + i) & (0x80 >> x1 % 8)) ? GxEPD_WHITE  : color;
#else
      uint16_t pixelcolor = (bitmap[i] & (0x80 >> x1 % 8)) ? GxEPD_WHITE  : color;
#endif
      drawPixel(x1, y1, pixelcolor);
    }
  }
}

void GxGDEP015OC1::drawBitmap(const uint8_t *bitmap, uint32_t size)
{
  drawBitmap(bitmap, size, false);
}

void GxGDEP015OC1::drawBitmap(const uint8_t *bitmap, uint32_t size, bool using_partial_update)
{
  if (using_partial_update)
  {
    _using_partial_mode = true; // remember
    _Init_Part();
    // set full screen
    _SetRamArea(0x00, xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
    _SetRamPointer(0x00, yPixelsPar % 256, yPixelsPar / 256); // set ram
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEP015OC1_BUFFER_SIZE; i++)
    {
#if defined(__AVR)
      _writeData((i < size) ? pgm_read_byte(bitmap + i) : 0xFF);
#else
      _writeData((i < size) ? bitmap[i] : 0xFF);
#endif
    }
    _Update_Part();
    _PowerOff();
  }
  else
  {
    _using_partial_mode = false; // remember
    _Init_Full();
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEP015OC1_BUFFER_SIZE; i++)
    {
#if defined(__AVR)
      _writeData((i < size) ? pgm_read_byte(bitmap + i) : 0xFF);
#else
      _writeData((i < size) ? bitmap[i] : 0xFF);
#endif
    }
    _Update_Full();
    _PowerOff();
  }
}

void GxGDEP015OC1::eraseDisplay(bool using_partial_mode)
{
  if (using_partial_mode)
  {
    _using_partial_mode = true; // remember
    _Init_Part();
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEP015OC1_BUFFER_SIZE; i++)
    {
      _writeData(0xFF);
    }
    _Update_Part();
    _PowerOff();
  }
  else
  {
    _Init_Full();
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEP015OC1_BUFFER_SIZE; i++)
    {
      _writeData(0xFF);
    }
    _Update_Full();
    _PowerOff();
  }
}

void GxGDEP015OC1::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  //fillScreen(0x0);
  if (x >= GxGDEP015OC1_WIDTH) return;
  if (y >= GxGDEP015OC1_HEIGHT) return;
  uint16_t xe = min(GxGDEP015OC1_WIDTH, x + w) - 1;
  uint16_t ye = min(GxGDEP015OC1_HEIGHT, y + h) - 1;
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  uint16_t p_xs = (GxGDEP015OC1_WIDTH - xe - 1) / 8;
  uint16_t p_xe = (GxGDEP015OC1_WIDTH - x - 1) / 8;
  uint16_t p_ys = (GxGDEP015OC1_HEIGHT - y - 1);
  uint16_t p_ye = (GxGDEP015OC1_HEIGHT - ye - 1);
  _Init_Part();
  _SetRamArea(p_xs, p_xe, p_ys % 256, p_ys / 256, p_ye % 256, p_ye / 256); // X-source area,Y-gate area
  _SetRamPointer(p_xs, p_ys % 256, p_ys / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xe_bx; x1 > xs_bx; x1--)
    {
      uint16_t idx = y1 * (GxGDEP015OC1_WIDTH / 8) + x1 - 1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      uint8_t mirror = 0x00;
      for (uint8_t i = 0; i < 8; i++)
      {
        mirror |= ((data >> i) & 0x01) << (7 - i);
      }
      _writeData(~mirror);
    }
  }
  _Update_Part();
  delay(300);
  _SetRamArea(p_xs, p_xe, p_ys % 256, p_ys / 256, p_ye % 256, p_ye / 256); // X-source area,Y-gate area
  _SetRamPointer(p_xs, p_ys % 256, p_ys / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xe_bx; x1 > xs_bx; x1--)
    {
      uint16_t idx = y1 * (GxGDEP015OC1_WIDTH / 8) + x1 - 1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      uint8_t mirror = 0x00;
      for (uint8_t i = 0; i < 8; i++)
      {
        mirror |= ((data >> i) & 0x01) << (7 - i);
      }
      _writeData(~mirror);
    }
  }
  delay(300);
  _PowerOff();
}

void GxGDEP015OC1::_writeCommand(uint8_t command)
{
  //while (digitalRead(_busy));
  if (digitalRead(_busy))
  {
    String str = String("command 0x") + String(command, HEX);
    _waitWhileBusy(str.c_str());
  }
  IO.writeCommandTransaction(command);
}

void GxGDEP015OC1::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void GxGDEP015OC1::_writeCommandData(const uint8_t* pCommandData, uint8_t datalen)
{
  //while (digitalRead(_busy)); // wait
  if (digitalRead(_busy))
  {
    String str = String("command 0x") + String(pCommandData[0], HEX);
    _waitWhileBusy(str.c_str());
  }
  IO.startTransaction();
  IO.writeCommand(*pCommandData++);
  for (uint8_t i = 0; i < datalen - 1; i++)  // sub the command
  {
    IO.writeData(*pCommandData++);
  }
  IO.endTransaction();

}

void GxGDEP015OC1::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  {
    if (!digitalRead(_busy)) break;
    delay(1);
  }
  if (comment)
  {
    unsigned long elapsed = micros() - start;
    //Serial.print(comment);
    //Serial.print(" : ");
    //Serial.println(elapsed);
  }
}

void GxGDEP015OC1::_SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1)
{
  _writeCommand(0x44);
  _writeData(Xstart);
  _writeData(Xend);
  _writeCommand(0x45);
  _writeData(Ystart);
  _writeData(Ystart1);
  _writeData(Yend);
  _writeData(Yend1);
}

void GxGDEP015OC1::_SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1)
{
  _writeCommand(0x4e);
  _writeData(addrX);
  _writeCommand(0x4f);
  _writeData(addrY);
  _writeData(addrY1);
}

void GxGDEP015OC1::_PowerOn(void)
{
  _writeCommand(0x22);
  _writeData(0xc0);
  _writeCommand(0x20);
}

void GxGDEP015OC1::_PowerOff(void)
{
  _writeCommand(0x22);
  _writeData(0xc3);
  _writeCommand(0x20);
}

void GxGDEP015OC1::showDemoExample()
{
  _Init_Full();
  delay(300);

  _Display_Full(first, 1); // pic
  delay(3000);

  _Display_Full(second, 1); //  pic
  delay(3000);

#if !defined(__AVR)

  _Display_Full(third, 1); // pic
  delay(3000);

#endif

  _Display_Full(logo, 1); //  pic
  delay(3000);

  _Init_Part();
  delay(300);

  _Display_Part(0, xPixelsPar, 0, yPixelsPar, logo, 1); //pic

  for (uint16_t m = 0; m < 9; m++)
  {
    _Display_Part(0, 31, 0, 63, (uint8_t*)&Num[m], 1);
  }
  delay(1500);

  _Init_Full();
  delay(300);
  uint8_t w = 0xff;
  _Display_Full(&w, 0); //all white
  delay(1500);

  _Init_Part();
  delay(300);

  while (1)
  {
    _Display_Part(0, xPixelsPar, 0, yPixelsPar, first, 1); //pic
    delay(1000);

    _Display_Part(0, xPixelsPar, 0, yPixelsPar, second, 1); //pic
    delay(950);

#if !defined(__AVR)

    _Display_Part(0, xPixelsPar, 0, yPixelsPar, third, 1); //pic
    delay(900);

    _Display_Part(0, xPixelsPar, 0, yPixelsPar, fourth, 1); //pic
    delay(850);

    _Display_Part(0, xPixelsPar, 0, yPixelsPar, fifth, 1); //pic
    delay(800);

    _Display_Part(0, xPixelsPar, 0, yPixelsPar, sixth, 1); //pic
    delay(750);

    //    _Display_Part(0,xDot-1,0,yDot-1,senventh,1); //pic
    //    delay(700);

    //    _Display_Part(0,xDot-1,0,yDot-1,eighth,1); //pic
    //    delay(650);
#endif
  }
}

void GxGDEP015OC1::_writeDisplayRam(uint16_t XSize, uint16_t YSize, const uint8_t* data)
{
  uint16_t xBytes = (XSize % 8 != 0) ? ((XSize / 8) + 1) : (XSize / 8);
  _waitWhileBusy();
  _writeCommand(0x24);
  for (uint16_t i = 0; i < YSize; i++)
  {
    for (uint16_t j = 0; j < xBytes; j++)
    {
#if defined(__AVR)
      _writeData(pgm_read_byte(data));
#else
      _writeData(*data);
#endif
      data++;
    }
  }
}

void GxGDEP015OC1::_writeDisplayRamMono(uint16_t XSize, uint16_t YSize, uint8_t value)
{
  //uint16_t xBytes = (XSize % 8 != 0) ? ((XSize / 8) + 1) : (XSize / 8);
  uint16_t xBytes = (XSize + 7) / 8;
  _waitWhileBusy();
  _writeCommand(0x24);
  for (uint16_t i = 0; i < YSize; i++)
  {
    for (uint16_t j = 0; j < xBytes; j++)
    {
      _writeData(value);
    }
  }
}

void GxGDEP015OC1::_partial_display(uint8_t RAM_XST, uint8_t RAM_XEND, uint8_t RAM_YST, uint8_t RAM_YST1, uint8_t RAM_YEND, uint8_t RAM_YEND1)
{
  _SetRamArea(RAM_XST, RAM_XEND, RAM_YST, RAM_YST1, RAM_YEND, RAM_YEND1);    /*set w h*/
  _SetRamPointer (RAM_XST, RAM_YST, RAM_YST1);      /*set orginal*/
}

void GxGDEP015OC1::_InitDisplay(void)
{
  _writeCommandData(GDOControl, sizeof(GDOControl));  // Pannel configuration, Gate selection
  _writeCommandData(softstart, sizeof(softstart));  // X decrease, Y decrease
  _writeCommandData(VCOMVol, sizeof(VCOMVol));    // VCOM setting
  _writeCommandData(DummyLine, sizeof(DummyLine));  // dummy line per gate
  _writeCommandData(Gatetime, sizeof(Gatetime));    // Gate time setting
  _writeCommandData(RamDataEntryMode, sizeof(RamDataEntryMode));  // X increase, Y decrease
  _SetRamArea(0x00, xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
  _SetRamPointer(0x00, yPixelsPar % 256, yPixelsPar / 256); // set ram
}

void GxGDEP015OC1::_Init_Full(void)
{
  _InitDisplay();
  _writeCommandData(LUTDefault_full, sizeof(LUTDefault_full));
  _PowerOn();
}

void GxGDEP015OC1::_Init_Part(void)
{
  _InitDisplay();
  _writeCommandData(LUTDefault_part, sizeof(LUTDefault_part));
  _PowerOn();
}

void GxGDEP015OC1::_Update_Full(void)
{
  _writeCommand(0x22);
  _writeData(0xc4);
  _writeCommand(0x20);
  _writeCommand(0xff);
}

void GxGDEP015OC1::_Update_Part(void)
{
  _writeCommand(0x22);
  _writeData(0x04);
  _writeCommand(0x20);
  _writeCommand(0xff);
}

void GxGDEP015OC1::_Display_Full(const uint8_t* DisBuffer, uint8_t Label)
{
  _SetRamPointer(0x00, yPixelsPar % 256, yPixelsPar / 256); // set ram
  if (Label == 0)
  {
    _writeDisplayRamMono(GxGDEP015OC1_X_PIXELS, GxGDEP015OC1_Y_PIXELS, 0xff); // white
  }
  else
  {
    _writeDisplayRam(GxGDEP015OC1_X_PIXELS, GxGDEP015OC1_Y_PIXELS, DisBuffer);
  }
  _Update_Full();

}

void GxGDEP015OC1::_Display_Part(uint8_t xStart, uint8_t xEnd, unsigned long yStart, unsigned long yEnd, const uint8_t* DisBuffer, uint8_t Label)
{
  if (Label == 0)
  {
    _partial_display(xStart / 8, xEnd / 8, yEnd % 256, yEnd / 256, yStart % 256, yStart / 256);
    _writeDisplayRamMono(xEnd - xStart, yEnd - yStart + 1, DisBuffer[0]); // black
    _Update_Part();
    delay(200);
    _partial_display(xStart / 8, xEnd / 8, yEnd % 256, yEnd / 256, yStart % 256, yStart / 256);
    _writeDisplayRamMono(xEnd - xStart, yEnd - yStart + 1, DisBuffer[0]); // black
  }
  else if (Label == 1)
  {
    _partial_display(xStart / 8, xEnd / 8, yEnd % 256, yEnd / 256, yStart % 256, yStart / 256); // set ram
    _writeDisplayRam(xEnd - xStart, yEnd - yStart + 1, DisBuffer);
    _Update_Part();
    delay(300);
    _partial_display(xStart / 8, xEnd / 8, yEnd % 256, yEnd / 256, yStart % 256, yStart / 256);
    _writeDisplayRam(xEnd - xStart, yEnd - yStart + 1, DisBuffer);
  }
  else
  {
    _partial_display(xStart / 8, xEnd / 8, yEnd % 256, yEnd / 256, yStart % 256, yStart / 256); // set ram
    _writeDisplayRam(xEnd - xStart, yEnd - yStart + 1, DisBuffer);
    _Update_Part();
    delay(300);
    _partial_display(xStart / 8, xEnd / 8, yEnd % 256, yEnd / 256, yStart % 256, yStart / 256);
    _writeDisplayRam(xEnd - xStart, yEnd - yStart + 1, DisBuffer);
  }
}

void GxGDEP015OC1::_drawCurrentPage()
{
  uint16_t x = 0;
  uint16_t xe = GxGDEP015OC1_WIDTH - 1;
  uint16_t y = GxGDEP015OC1_PAGE_HEIGHT * _current_page;
  uint16_t ye = y + GxGDEP015OC1_PAGE_HEIGHT - 1;
  uint16_t p_xs = (GxGDEP015OC1_WIDTH - xe - 1) / 8;
  uint16_t p_xe = (GxGDEP015OC1_WIDTH - x - 1) / 8;
  uint16_t p_ys = (GxGDEP015OC1_HEIGHT - y - 1);
  uint16_t p_ye = (GxGDEP015OC1_HEIGHT - ye - 1);
  _SetRamArea(p_xs, p_xe, p_ys % 256, p_ys / 256, p_ye % 256, p_ye / 256); // X-source area,Y-gate area
  _SetRamPointer(p_xs, p_ys % 256, p_ys / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = 0; y1 < GxGDEP015OC1_PAGE_HEIGHT; y1++)
  {
    for (int16_t x1 = GxGDEP015OC1_WIDTH / 8; x1 > 0; x1--)
    {
      uint16_t idx = y1 * (GxGDEP015OC1_WIDTH / 8) + x1 - 1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      uint8_t mirror = 0x00;
      for (uint8_t i = 0; i < 8; i++)
      {
        mirror |= ((data >> i) & 0x01) << (7 - i);
      }
      _writeData(~mirror);
    }
  }
  _Update_Part();
  delay(300);
#if 1 // this is required
  _SetRamArea(p_xs, p_xe, p_ys % 256, p_ys / 256, p_ye % 256, p_ye / 256); // X-source area,Y-gate area
  _SetRamPointer(p_xs, p_ys % 256, p_ys / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = 0; y1 < GxGDEP015OC1_PAGE_HEIGHT; y1++)
  {
    for (int16_t x1 = GxGDEP015OC1_WIDTH / 8; x1 > 0; x1--)
    {
      uint16_t idx = y1 * (GxGDEP015OC1_WIDTH / 8) + x1 - 1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      uint8_t mirror = 0x00;
      for (uint8_t i = 0; i < 8; i++)
      {
        mirror |= ((data >> i) & 0x01) << (7 - i);
      }
      _writeData(~mirror);
    }
  }
  delay(300);
#endif
}

void GxGDEP015OC1::drawPaged(void (*drawCallback)(void))
{
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  _Init_Part();
  for (_current_page = 0; _current_page < GxGDEP015OC1_PAGES; _current_page++)
  {
    fillScreen(0xFF);
    drawCallback();
    //fillScreen(0x00);
    _drawCurrentPage();
    //delay(2000);
  }
  _current_page = -1;
  _PowerOff();
}

