#include "LS027B7DH01.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint8_t LCD_BIT_VCOM = 0x40;
static uint8_t LCD_CMD_CLEAR = 0x20;
static uint8_t LCD_CMD_UPDATE = 0x88;
static uint8_t LCD_CMD_NOP = 0x00;
static uint8_t SMLCD_VCOM = 0x00;

// This buffer holds 400 Columns * 240 Row / 8 pixels per byte = 12K of Display buffer
static uint8_t DispBuf[SCR_H * (SCR_W >> 3)];  // entire display buffer.
static uint8_t DATA_LINE_LEN = SCR_W >> 3;

static const uint8_t LUT_LINE[240] = {
		0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 0x08,
		0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04,
		0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 0x0C,
		0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 0x02,
		0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 0x0A,
		0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA, 0x06,
		0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 0x0E,
		0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01,
		0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1, 0x09,
		0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 0x05,
		0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D,
		0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD, 0x03,
		0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 0x0B,
		0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07,
		0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 0x0F
};

__STATIC_INLINE uint8_t __reverse8bit(uint8_t byte) {
	// Using ARM RBIT instruction
	// Since it operates with 32-bit values only, result must be shifted by 24 bits to the right
	return (uint8_t)(__RBIT(byte) >> 24);
}

// Display Initialization
void LCD_Init(LS027B7DH01* MemDisp, SPI_HandleTypeDef* Bus,
			  GPIO_TypeDef* dispGPIO, uint16_t LCDcs, uint16_t LCDon,
              TIM_HandleTypeDef* TimerX, uint32_t COMpwm) {
  // Store params into our struct
  MemDisp->Bus = Bus;
  MemDisp->dispGPIO = dispGPIO;
  MemDisp->TimerX = TimerX;
  MemDisp->COMpwm = COMpwm;
  MemDisp->LCDcs = LCDcs;
  MemDisp->LCDon = LCDon;

  memset(DispBuf, 0xFF, sizeof(DispBuf));

  HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDon, GPIO_PIN_SET);  // Turn display back on
  HAL_Delay(10);

  // Start 50Hz PWM for COM inversion of the display
//  HAL_TIM_PWM_Start(MemDisp->TimerX, MemDisp->COMpwm);
//  MemDisp->TimerX->Instance->CCR1 = 5;

  LCD_Clean(MemDisp);
}

void LCD_ToggleVCOM(LS027B7DH01* MemDisp) {
	SMLCD_VCOM ^= LCD_BIT_VCOM;
	uint8_t cmd[2] = {SMLCD_VCOM, LCD_CMD_NOP};
	HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_SET);
	HAL_SPI_Transmit(MemDisp->Bus, cmd, 2, 150);
	HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_RESET);
}

// Display update (Transmit data)
void LCD_Update(LS027B7DH01* MemDisp) {


  uint8_t* ptr = DispBuf;

//  uint8_t msg[54];

//  memset(msg, LCD_CMD_NOP, sizeof(msg));
//  msg[0] = 0x01;
//  msg[1] = 1;
//  msg[3] = 0xFF;
//  msg[32] = 0xFF;
//  msg[52] = LCD_CMD_NOP;
//  msg[53] = LCD_CMD_NOP;
//
//  HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_SET);
//  HAL_SPI_Transmit(MemDisp->Bus, msg, sizeof(msg), 3);
//  HAL_Delay(1);
//  HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_RESET);

//  msg[0] = LCD_CMD_UPDATE;
//  for (uint8_t line = 0; line < SCR_H; line++) {
//	msg[1] = LUT_LINE[line + 1];
//	memcpy(msg + 2, ptr, DATA_LINE_LEN);
//	ptr += DATA_LINE_LEN;
//
//    HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_SET);  // Begin
//    HAL_SPI_Transmit(MemDisp->Bus, msg, sizeof(msg), 150);
//    HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_RESET);  // Done
//
//    // just for rendering debugging
//    HAL_Delay(10);
//  }

  uint8_t cmd[2] = {LCD_CMD_UPDATE, 0x00};
  uint8_t trailer[2] = {LCD_CMD_NOP, LCD_CMD_NOP};
  for (uint8_t line = 0; line < SCR_H; line++) {
	  cmd[1] = LUT_LINE[line + 1];

      HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_SET);  // Begin
      HAL_SPI_Transmit(MemDisp->Bus, cmd, 2, 0);
      HAL_SPI_Transmit(MemDisp->Bus, ptr, DATA_LINE_LEN, 0);
      HAL_SPI_Transmit(MemDisp->Bus, trailer, 2, 0);
      HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_RESET);  // Done

      ptr += DATA_LINE_LEN;
      // just for rendering debugging
      HAL_Delay(10);
  }

}

// Clean the Buffer
void LCD_BufClean(void) { memset(DispBuf, 0xFF, sizeof(DispBuf)); }

// Clear entire Display
void LCD_Clean(LS027B7DH01* MemDisp) {
  uint8_t buf[2] = {LCD_CMD_CLEAR, LCD_CMD_NOP};
  // At lease 3 + 13 clock is needed for Display clear (16 Clock = 8x2 bit = 2 byte)
  HAL_GPIO_WritePin(MemDisp->dispGPIO, MemDisp->LCDcs, GPIO_PIN_SET);
  HAL_SPI_Transmit(MemDisp->Bus, buf, 2, 150);  // According to Datasheet
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

void LCD_LoadFull(uint8_t* BMP) { memcpy(DispBuf, BMP, sizeof(DispBuf)); }

void LCD_Fill(int fill) {
  memset(DispBuf, (fill ? 0 : 0xFF), SCR_W * SCR_H / 8);
  HAL_Delay(10);
}
