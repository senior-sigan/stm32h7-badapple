#include "LS027B7DH01.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Display Commands
uint8_t clearCMD[2] = {SMLCD_CMD_CLS, SMLCD_CMD_NOP};    // Display Clear 0x04 (HW_LSB)
uint8_t printCMD[2] = {SMLCD_CMD_WRITE, SMLCD_CMD_NOP};  // Display Bitmap (after issued display update) 0x01 (HW_LSB)

// This buffer holds 400 Columns * 240 Row / 8 pixels per byte = 12K of Display buffer
uint8_t* DispBuf;  // entire display buffer.

// This buffer holds temporary 2 Command bytes
static uint8_t SendBuf[2];

static uint8_t smallRbit(uint8_t re) { return (uint8_t) (__RBIT(re) >> 24); }

// Display Initialization
void LCD_Init(LS027B7DH01* MemDisp, SPI_HandleTypeDef* Bus, GPIO_TypeDef* dispGPIO, uint16_t LCDcs, uint16_t LCDon,
              TIM_HandleTypeDef* TimerX, uint32_t COMpwm) {
  // Store params into our struct
  MemDisp->Bus = Bus;
  MemDisp->dispGPIO = dispGPIO;
  MemDisp->TimerX = TimerX;
  MemDisp->COMpwm = COMpwm;
  MemDisp->LCDcs = LCDcs;
  MemDisp->LCDon = LCDon;

  DispBuf = malloc(SCR_W * SCR_H / 8);
  memset(DispBuf, 0xFF, SCR_W * SCR_H / 8);

  HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDon,
                    GPIO_PIN_SET);  // Turn display back on
  // Start 50Hz PWM for COM inversion of the display
  HAL_TIM_PWM_Start(MemDisp->TimerX, MemDisp->COMpwm);
  MemDisp->TimerX->Instance->CCR1 = 5;

  // At lease 3 + 13 clock is needed for Display clear (16 Clock = 8x2 bit = 2 byte)
  HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_SET);
  HAL_SPI_Transmit(MemDisp->Bus, (uint8_t*) clearCMD, 2, 150);  // According to Datasheet
  HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_RESET);
}

// Display update (Transmit data)
void LCD_Update(LS027B7DH01* MemDisp) {
  SendBuf[0] = printCMD[0];                                            // M0 High, M2 Low
  HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_SET);  // Begin

  for (uint8_t count = 0; count < SCR_H; count++) {
    SendBuf[1] = smallRbit(count + 1);  // counting from row number 1 to row number 240
    // row to DispBuf offset
    uint16_t offset = count * SCR_W / 8;

    HAL_SPI_Transmit(MemDisp->Bus, SendBuf, 2, 150);
    HAL_SPI_Transmit(MemDisp->Bus, DispBuf + offset, 50, 150);
  }
  // Send the Dummies bytes after whole display data transmission
  HAL_SPI_Transmit(MemDisp->Bus, 0x00, 2, 150);

  HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_RESET);  // Done
}

// Clean the Buffer
void LCD_BufClean(void) { memset(DispBuf, 0xFF, SCR_W * SCR_H / 8); }

// Clear entire Display
void LCD_Clean(LS027B7DH01* MemDisp) {
  // At lease 3 + 13 clock is needed for Display clear (16 Clock = 8x2 bit = 2 byte)
  HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_SET);
  HAL_SPI_Transmit(MemDisp->Bus, (uint8_t*) clearCMD, 2, 150);  // According to Datasheet
  HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_RESET);
}

void LCD_LoadPart(uint8_t* BMP, uint8_t Xcord, uint8_t Ycord, uint8_t bmpW, uint8_t bmpH) {
  uint16_t XYoff, WHoff = 0;

  // Counting from Y origin point to bmpH using for loop
  for (uint8_t loop = 0; loop < bmpH; loop++) {
    // turn X an Y into absolute offset number for Buffer
    XYoff = (Ycord + loop) * SCR_W / 8;
    XYoff += Xcord;  // offset start at the left most, The count from left to right for Xcord times

    // turn W and H into absolute offset number for Bitmap image
    WHoff = loop * bmpW;

    memcpy(DispBuf + XYoff, BMP + WHoff, bmpW);
  }
}

void LCD_LoadFull(uint8_t* BMP) { memcpy(DispBuf, BMP, 12000); }

void LCD_Fill(int fill) {
  memset(DispBuf, (fill ? 0 : 0xFF), SCR_W * SCR_H / 8);
  HAL_Delay(10);
}