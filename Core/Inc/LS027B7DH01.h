#ifndef __LS027B7DH01_H_
#define __LS027B7DH01_H_

#include "stm32h7xx_hal.h"

// Screen resolution (in pixels)
#define SCR_W 400  // width
#define SCR_H 240  // height

// Display commands
#define SMLCD_CMD_WRITE ((uint8_t) 0x93)  // Write line
#define SMLCD_CMD_VCOM ((uint8_t) 0x40)   // VCOM bit (not a command in fact)
#define SMLCD_CMD_CLS ((uint8_t) 0x56)    // Clear the screen to all white
#define SMLCD_CMD_NOP ((uint8_t) 0x00)    // No command

// This typedef holds the hardware parameters. For GPIO and SPI
typedef struct {
  SPI_HandleTypeDef* Bus;
  GPIO_TypeDef* dispGPIO;
  TIM_HandleTypeDef* TimerX;
  uint32_t COMpwm;
  uint16_t LCDcs;
  uint16_t LCDon;
} LS027B7DH01;

void LCD_Init(LS027B7DH01* memDisp, SPI_HandleTypeDef* bus,
			  GPIO_TypeDef* dispGPIO, uint16_t scs, uint16_t lcdOn,
              TIM_HandleTypeDef* timerX, uint32_t comPWM);

void LCD_Update(LS027B7DH01* MemDisp);

void LCD_ToggleVCOM(LS027B7DH01* MemDisp);

void LCD_BufClean(void);

void LCD_Clean(LS027B7DH01* MemDisp);

// Buffer update (with X,Y Coordinate and image WxH) X,Y Coordinate start at (1,1) to (50,240)
//
// NOTE THAT THE X COOR and WIDTH ARE BYTE NUMBER NOT PIXEL NUMBER (8 pixel = 1 byte). A.K.A IT'S BYTE ALIGNED
void LCD_LoadPart(uint8_t* BMP, uint8_t Xcord, uint8_t Ycord, uint8_t bmpW, uint8_t bmpH);

// Buffer update (full 400*240 pixels)
void LCD_LoadFull(uint8_t* BMP);

// Fill screen with either black or white color
void LCD_Fill(int fill);

#endif /* __LS027B7DH01_H_ */
