/************************************************************************************
   class GxGDE0213B1 : Display class example for GDE0213B1 e-Paper from GoodDisplay.com

   based on Demo Example from GoodDisplay.com, avalable with any order for such a display, no copyright notice.

   Author : J-M Zingg

   modified by :

   Version : 1.1

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

#include "GxGDE0213B1.h"

#define xPixelsPar (GxGDE0213B1_X_PIXELS-1)
#define yPixelsPar (GxGDE0213B1_Y_PIXELS-1)

const uint8_t LUTDefault_full[] =
{
  0x32,  // command
  0x22, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x01, 0x00, 0x00, 0x00, 0x00
};

const uint8_t LUTDefault_part[] =
{
  0x32,  // command
  0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x0F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t GDOControl[]={0x01,yPixelsPar%256,yPixelsPar/256,0x00}; //for 2.13inch
uint8_t softstart[] = {0x0c, 0xd7, 0xd6, 0x9d};
uint8_t VCOMVol[] = {0x2c, 0xa8};  // VCOM 7c
uint8_t DummyLine[] = {0x3a, 0x1a}; // 4 dummy line per gate
uint8_t Gatetime[] = {0x3b, 0x08};  // 2us per line
uint8_t RamDataEntryMode[] = {0x11, 0x01};  // Ram data entry mode

GxGDE0213B1::GxGDE0213B1(GxIO& io, uint8_t rst, uint8_t busy) :
  GxEPD(GxGDE0213B1_WIDTH, GxGDE0213B1_HEIGHT),
  IO(io), _rst(rst), _busy(busy)
{
}

template <typename T> static inline void
swap(T& a, T& b)
{
  T t = a;
  a = b;
  b = t;
}

void GxGDE0213B1::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDE0213B1_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDE0213B1_WIDTH - x - 1;
      y = GxGDE0213B1_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDE0213B1_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDE0213B1_WIDTH / 8;

  if (!color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}

void GxGDE0213B1::init(void)
{
  IO.init();
  IO.setFrequency(4000000); // 4MHz : 250ns > 150ns min RD cycle
  digitalWrite(_rst, HIGH);
  pinMode(_rst, OUTPUT);
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
}

void GxGDE0213B1::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < GxGDE0213B1_BUFFER_SIZE; x++)
  {
    _buffer[x] = data;
  }
}

void GxGDE0213B1::update(void)
{
  _wakeUp();
  _writeCommand(0x24);
  for (uint16_t y = 0; y < GxGDE0213B1_HEIGHT; y++)
  {
    for (uint16_t x = GxGDE0213B1_WIDTH / 8; x > 0; x--)
    {
      uint8_t data = _buffer[y * (GxGDE0213B1_WIDTH / 8) + x - 1];
      uint8_t mirror = 0x00;
      for (uint8_t i = 0; i < 8; i++)
      {
        mirror |= ((data >> i) & 0x01) << (7 - i);
      }
      _writeData(~mirror);
    }
  }
  // Update
  _writeCommand(0x22);
  _writeData(0xc7);
  _writeCommand(0x20);
  _writeCommand(0xff);
  _PowerOff();
}

void GxGDE0213B1::drawBitmap(const uint8_t *bitmap, uint32_t size)
{
  _wakeUp();
  _writeCommand(0x24);
  for (uint32_t i = 0; i < GxGDE0213B1_BUFFER_SIZE; i++)
  {
    _writeData((i < size) ? bitmap[i] : 0x00);
  }
  // Update
  _writeCommand(0x22);
  _writeData(0xc7);
  _writeCommand(0x20);
  _writeCommand(0xff);
  _PowerOff();
}

void  GxGDE0213B1::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  for (uint16_t x1 = x; x1 < x + w; x1++)
  {
    for (uint16_t y1 = y; y1 < y + h; y1++)
    {
      uint16_t i = x1 / 8 + y1 * w / 8;
      uint16_t pixelcolor = (bitmap[i] & (0x80 >> x1 % 8)) ? GxEPD_WHITE  : color;
      drawPixel(x1, y1, pixelcolor);
    }
  }
}

void GxGDE0213B1::_writeCommand(uint8_t command)
{
  //while (digitalRead(_busy));
  if (digitalRead(_busy))
  {
    String str = String("command 0x") + String(command, HEX);
    _waitWhileBusy(str.c_str());
  }
  IO.writeCommandTransaction(command);
}

void GxGDE0213B1::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void GxGDE0213B1::_writeCommandData(uint8_t *pCommandData, uint8_t datalen)
{
  //while (digitalRead(_busy)); // wait
  if (digitalRead(_busy))
  {
    String str = String("command 0x") + String(pCommandData[0], HEX);
    _waitWhileBusy(str.c_str());
  }
  IO.startTransaction();
  IO.writeCommand(*pCommandData++);
  for (uint8_t i = 0; i < datalen - 1; i++)	// sub the command
  {
    IO.writeData(*pCommandData++);
  }
  IO.endTransaction();

}

void GxGDE0213B1::_waitWhileBusy(const char* comment)
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

void GxGDE0213B1::_SetRamArea(uint8_t Xstart, uint8_t Xend,
                              uint8_t Ystart, uint8_t Ystart1,
                              uint8_t Yend, uint8_t Yend1)
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

void GxGDE0213B1::_SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1)
{
  _writeCommand(0x4e);
  _writeData(addrX);
  _writeCommand(0x4f);
  _writeData(addrY);
  _writeData(addrY1);
}

void GxGDE0213B1::_writeLUT(uint8_t *LUTvalue)
{
  _writeCommandData(LUTvalue, 31);
}

void GxGDE0213B1::_PowerOn(void)
{
  _writeCommand(0x22);
  _writeData(0xc0);
  _writeCommand(0x20);
}

void GxGDE0213B1::_PowerOff(void)
{
  _writeCommand(0x22);
  _writeData(0xc3);
  _writeCommand(0x20);
}

void GxGDE0213B1::_wakeUp()
{
  _writeCommandData(GDOControl, sizeof(GDOControl));  // Pannel configuration, Gate selection
  _writeCommandData(softstart, sizeof(softstart));  // X decrease, Y decrease
  _writeCommandData(VCOMVol, sizeof(VCOMVol));    // VCOM setting
  _writeCommandData(DummyLine, sizeof(DummyLine));  // dummy line per gate
  _writeCommandData(Gatetime, sizeof(Gatetime));    // Gate time setting
  _writeCommandData(RamDataEntryMode, sizeof(RamDataEntryMode));  // X decrease, Y decrease
  _SetRamArea(0x00, xPixelsPar/8, yPixelsPar%256, yPixelsPar/256, 0x00, 0x00);  // X-source area,Y-gate area
  _SetRamPointer(0x00, yPixelsPar%256, yPixelsPar/256); // set ram
  _writeLUT((uint8_t *)LUTDefault_full);
  _PowerOn();
}

