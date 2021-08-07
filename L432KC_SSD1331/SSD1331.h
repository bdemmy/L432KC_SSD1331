#pragma once
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include <string.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
	
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _b : _a; })

typedef struct SSD_Pin_Config_t
{
	uint16_t DC_PIN, CS_PIN, RES_PIN;
	GPIO_TypeDef *DC_PORT, *CS_PORT, *RES_PORT;
} SSD_Pin_Config_t;

typedef struct Pixel_65k1_t
{
	uint8_t b1, b2;
} Pixel_65k1_t;

typedef struct SSD1331
{
	SSD_Pin_Config_t m_cfg;
	SPI_HandleTypeDef* m_hspi;
	Pixel_65k1_t buffer[96][64];
} SSD1331;

void SSD_Command(SSD1331 *display, uint8_t cmd)
{
	HAL_GPIO_WritePin(display->m_cfg.DC_PORT, display->m_cfg.DC_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(display->m_cfg.CS_PORT, display->m_cfg.CS_PIN, GPIO_PIN_RESET);
	
	HAL_SPI_Transmit(display->m_hspi, &cmd, 1, 100);
		
	HAL_GPIO_WritePin(display->m_cfg.DC_PORT, display->m_cfg.DC_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(display->m_cfg.CS_PORT, display->m_cfg.CS_PIN, GPIO_PIN_SET);
}

void SSD_Write(SSD1331 *display, uint8_t cmd)
{
	HAL_GPIO_WritePin(display->m_cfg.CS_PORT, display->m_cfg.CS_PIN, GPIO_PIN_RESET);
	
	HAL_SPI_Transmit(display->m_hspi, &cmd, 1, 100);
		
	HAL_GPIO_WritePin(display->m_cfg.CS_PORT, display->m_cfg.CS_PIN, GPIO_PIN_SET);
}

void SSD_SetPixel(SSD1331 *display, int x, int y, int r, int g, int b)
{
	x = max(min(x, 95), 0);
	y = max(min(y, 63), 0);
	
	display->buffer[x][y].b1 = r << 3 | g >> 3;
	display->buffer[x][y].b2 = g << 5 | b;
}

void SSD_Init(SSD1331 *display)
{
	// Clear the memory buffer
	memset(&display->buffer, 0, sizeof(display->buffer));
	
	// Display reset sequence
	HAL_GPIO_WritePin(display->m_cfg.RES_PORT, display->m_cfg.RES_PIN, GPIO_PIN_RESET);
	HAL_Delay(250);
	HAL_GPIO_WritePin(display->m_cfg.RES_PORT, display->m_cfg.RES_PIN, GPIO_PIN_SET);
	HAL_Delay(250);
	
	// Init some parameters
	SSD_Command(display, 0xaf);
	
	// Driver remap and color depth
	SSD_Command(display, 0b10100000);
	SSD_Command(display, 0b01100001);
	
	SSD_Command(display, 0b10110011);
	SSD_Command(display, 0b11110000);
	SSD_Command(display, 0xa4);
}

void SSD_Rect(SSD1331 *display, int x, int y, int w, int h, int r, int g, int b)
{
	for (int i = y; i < y + h; i++)
	{
		for (int j = x; j < x + w; j++)
		{
			if (i == y || j == x || i == y + h - 1 || j == x + w - 1)
			{
				SSD_SetPixel(display, j, i, r, g, b);
			}
		}
	}
}

void SSD_FilledRect(SSD1331 *display, int x, int y, int w, int h, int border_r, int border_g, int border_b, int r, int g, int b)
{
	for (int i = y; i < y + h; i++)
	{
		for (int j = x; j < x + w; j++)
		{
			if (i == y || j == x || i == y + h - 1 || j == x + w - 1)
			{
				SSD_SetPixel(display, j, i, border_r, border_g, border_b);
			}
			else
			{
				SSD_SetPixel(display, j, i, r, g, b);
			}
		}
	}
}

void SSD_Clear(SSD1331 *display)
{
	for (int x = 0; x <= 95; x++)
	{
		for (int y = 0; y <= 63; y++)
		{
			SSD_Write(display, 0);
			SSD_Write(display, 0);
		}
	}
}

void SSD_Circle(SSD1331 *display, int origX, int origY, int radius, int r, int g, int b)
{
	for (int y = -radius; y <= radius; y++) {
		for (int x = -radius; x <= radius; x++) {
			if (x*x + y*y <= radius*radius)
			{
				SSD_SetPixel(display, origX + x + radius, origY + y + radius, r, g, b);
			}
		}
	}
}

void SSD_Render(SSD1331 *display)
{
	SSD_Command(display, 0b00010101);
	SSD_Command(display, 0);
	SSD_Command(display, 95);
	
	SSD_Command(display, 0b01110101);
	SSD_Command(display, 0);
	SSD_Command(display, 63);
	
	// Transmit the display buffer to the device
	HAL_GPIO_WritePin(display->m_cfg.CS_PORT, display->m_cfg.CS_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(display->m_hspi, (uint8_t*)&display->buffer, sizeof(display->buffer), 10);
	HAL_GPIO_WritePin(display->m_cfg.CS_PORT, display->m_cfg.CS_PIN, GPIO_PIN_SET);
}