/***************************************************************************//**
 *   @file   ST7579.c
 *   @brief  Implementation of ST7579 Driver.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2012(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
********************************************************************************
 *   SVN Revision: 538
*******************************************************************************/

/******************************************************************************/
/* Include Files                                                              */
/******************************************************************************/
#include "Communication.h"	// Communication definitions.
#include "ST7579.h"			// ST7579 definitions.

/******************************************************************************/
/* Private variables                                                          */
/******************************************************************************/
const unsigned char st7579Font[95][5] =
{
	{ 0x00, 0x00, 0x00, 0x00, 0x00 },	// sp
	{ 0x00, 0x00, 0x2f, 0x00, 0x00 },	// !
	{ 0x00, 0x07, 0x00, 0x07, 0x00 },	// "
	{ 0x14, 0x7f, 0x14, 0x7f, 0x14 },	// #
	{ 0x24, 0x2a, 0x7f, 0x2a, 0x12 },	// $
	{ 0xc4, 0xc8, 0x10, 0x26, 0x46 },	// %
	{ 0x36, 0x49, 0x55, 0x22, 0x50 },	// &
	{ 0x00, 0x05, 0x03, 0x00, 0x00 },	// '
	{ 0x00, 0x1c, 0x22, 0x41, 0x00 },	// (
	{ 0x00, 0x41, 0x22, 0x1c, 0x00 },	// )
	{ 0x14, 0x08, 0x3E, 0x08, 0x14 },	// *
	{ 0x08, 0x08, 0x3E, 0x08, 0x08 },	// +
	{ 0x00, 0x00, 0x50, 0x30, 0x00 },	// ,
	{ 0x10, 0x10, 0x10, 0x10, 0x10 },	// -
	{ 0x00, 0x60, 0x60, 0x00, 0x00 },	// .
	{ 0x20, 0x10, 0x08, 0x04, 0x02 },	// /
	{ 0x3E, 0x51, 0x49, 0x45, 0x3E },	// 0
	{ 0x00, 0x42, 0x7F, 0x40, 0x00 },	// 1
	{ 0x42, 0x61, 0x51, 0x49, 0x46 },	// 2
	{ 0x21, 0x41, 0x45, 0x4B, 0x31 },	// 3
	{ 0x18, 0x14, 0x12, 0x7F, 0x10 },	// 4
	{ 0x27, 0x45, 0x45, 0x45, 0x39 },	// 5
	{ 0x3C, 0x4A, 0x49, 0x49, 0x30 },	// 6
	{ 0x01, 0x71, 0x09, 0x05, 0x03 },	// 7
	{ 0x36, 0x49, 0x49, 0x49, 0x36 },	// 8
	{ 0x06, 0x49, 0x49, 0x29, 0x1E },	// 9
	{ 0x00, 0x36, 0x36, 0x00, 0x00 },	// :
	{ 0x00, 0x56, 0x36, 0x00, 0x00 },	// ;
	{ 0x08, 0x14, 0x22, 0x41, 0x00 },	// <
	{ 0x14, 0x14, 0x14, 0x14, 0x14 },	// =
	{ 0x00, 0x41, 0x22, 0x14, 0x08 },	// >
	{ 0x02, 0x01, 0x51, 0x09, 0x06 },	// ?
	{ 0x32, 0x49, 0x59, 0x51, 0x3E },	// @
	{ 0x7E, 0x11, 0x11, 0x11, 0x7E },	// A
	{ 0x7F, 0x49, 0x49, 0x49, 0x36 },	// B
	{ 0x3E, 0x41, 0x41, 0x41, 0x22 },	// C
	{ 0x7F, 0x41, 0x41, 0x22, 0x1C },	// D
	{ 0x7F, 0x49, 0x49, 0x49, 0x41 },	// E
	{ 0x7F, 0x09, 0x09, 0x09, 0x01 },	// F
	{ 0x3E, 0x41, 0x49, 0x49, 0x7A },	// G
	{ 0x7F, 0x08, 0x08, 0x08, 0x7F },	// H
	{ 0x00, 0x41, 0x7F, 0x41, 0x00 },	// I
	{ 0x20, 0x40, 0x41, 0x3F, 0x01 },	// J
	{ 0x7F, 0x08, 0x14, 0x22, 0x41 },	// K
	{ 0x7F, 0x40, 0x40, 0x40, 0x40 },	// L
	{ 0x7F, 0x02, 0x0C, 0x02, 0x7F },	// M
	{ 0x7F, 0x04, 0x08, 0x10, 0x7F },	// N
	{ 0x3E, 0x41, 0x41, 0x41, 0x3E },	// O
	{ 0x7F, 0x09, 0x09, 0x09, 0x06 },	// P
	{ 0x3E, 0x41, 0x51, 0x21, 0x5E },	// Q
	{ 0x7F, 0x09, 0x19, 0x29, 0x46 },	// R
	{ 0x46, 0x49, 0x49, 0x49, 0x31 },	// S
	{ 0x01, 0x01, 0x7F, 0x01, 0x01 },	// T
	{ 0x3F, 0x40, 0x40, 0x40, 0x3F },	// U
	{ 0x1F, 0x20, 0x40, 0x20, 0x1F },	// V
	{ 0x3F, 0x40, 0x38, 0x40, 0x3F },	// W
	{ 0x63, 0x14, 0x08, 0x14, 0x63 },	// X
	{ 0x07, 0x08, 0x70, 0x08, 0x07 },	// Y
	{ 0x61, 0x51, 0x49, 0x45, 0x43 },	// Z
	{ 0x00, 0x7F, 0x41, 0x41, 0x00 },	// [
	{ 0x55, 0x2A, 0x55, 0x2A, 0x55 },	// /
	{ 0x00, 0x41, 0x41, 0x7F, 0x00 },	// ]
	{ 0x04, 0x02, 0x01, 0x02, 0x04 },	// ^
	{ 0x40, 0x40, 0x40, 0x40, 0x40 },	// _
	{ 0x00, 0x01, 0x02, 0x04, 0x00 },	// '
	{ 0x20, 0x54, 0x54, 0x54, 0x78 },	// a
	{ 0x7F, 0x48, 0x44, 0x44, 0x38 },	// b
	{ 0x38, 0x44, 0x44, 0x44, 0x20 },	// c
	{ 0x38, 0x44, 0x44, 0x48, 0x7F },	// d
	{ 0x38, 0x54, 0x54, 0x54, 0x18 },	// e
	{ 0x08, 0x7E, 0x09, 0x01, 0x02 },	// f
	{ 0x0C, 0x52, 0x52, 0x52, 0x3E },	// g
	{ 0x7F, 0x08, 0x04, 0x04, 0x78 },	// h
	{ 0x00, 0x44, 0x7D, 0x40, 0x00 },	// i
	{ 0x20, 0x40, 0x44, 0x3D, 0x00 },	// j
	{ 0x7F, 0x10, 0x28, 0x44, 0x00 },	// k
	{ 0x00, 0x41, 0x7F, 0x40, 0x00 },	// l
	{ 0x7C, 0x04, 0x18, 0x04, 0x78 },	// m
	{ 0x7C, 0x08, 0x04, 0x04, 0x78 },	// n
	{ 0x38, 0x44, 0x44, 0x44, 0x38 },	// o
	{ 0x7C, 0x14, 0x14, 0x14, 0x08 },	// p
	{ 0x08, 0x14, 0x14, 0x18, 0x7C },	// q
	{ 0x7C, 0x08, 0x04, 0x04, 0x08 },	// r
	{ 0x48, 0x54, 0x54, 0x54, 0x20 },	// s
	{ 0x04, 0x3F, 0x44, 0x40, 0x20 },	// t
	{ 0x3C, 0x40, 0x40, 0x20, 0x7C },	// u
	{ 0x1C, 0x20, 0x40, 0x20, 0x1C },	// v
	{ 0x3C, 0x40, 0x30, 0x40, 0x3C },	// w
	{ 0x44, 0x28, 0x10, 0x28, 0x44 },	// x
	{ 0x0C, 0x50, 0x50, 0x50, 0x3C },	// y
	{ 0x44, 0x64, 0x54, 0x4C, 0x44 },	// z
	{ 0x08, 0x3e, 0x41, 0x41, 0x00 },	// { 
	{ 0x00, 0x00, 0x77, 0x00, 0x00 },	// | 
	{ 0x00, 0x41, 0x41, 0x3e, 0x08 },	// } 
	{ 0x02, 0x01, 0x02, 0x01, 0x00 }	// ~
};

/***************************************************************************//**
 * @brief Transmits 8 bits to ST7579 controller.
 *
 * @param data - data to transmit.
 *
 * @return None.
*******************************************************************************/
void ST7579_WriteByte(unsigned char data)
{
    unsigned char spiWord[2]    = {0};
    spiWord[0] = 0x02;
    spiWord[1] = data;
	SPI_Write(spiWord,1);
}

/***************************************************************************//**
 * @brief Initializes the ST7579 controller.
 *
 * @param None.
 *
 * @return None.
*******************************************************************************/
unsigned char ST7579_Init(void)
{
	unsigned char status  = 0x0;
	status = SPI_Init(0, 1000000, 1, 0);
	ST7579_RS_PIN_OUT;
	ST7579_RS_LOW;
	/* Select LCD bias ratio of the voltage required for driving the LCD. */
	ST7579_WriteByte(0x21);
	ST7579_WriteByte(0x16);
	/* Select Booster efficiency and Booster stage. */
	ST7579_WriteByte(0x23);
	ST7579_WriteByte(0x99);
	/* Set V0 Range. */
	ST7579_WriteByte(0x20);	
	ST7579_WriteByte(0x04);
	/* 50 ms delay is recommended. */
	/* Set the frame frequency */
	ST7579_WriteByte(0x23);
	ST7579_WriteByte(0x0C);
	/* Set V0. */
	ST7579_WriteByte(0x21);	
	ST7579_WriteByte(0xE7);
	/* Select the display mode */
	ST7579_WriteByte(0x20);
	ST7579_WriteByte(0x0C);
	ST7579_RS_HIGH;
	/* Clear RAM. */
	ST7579_Clear();

	return(status);
}

/***************************************************************************//**
 * @brief Clears ST7579 RAM.
 *
 * @param None.
 *
 * @return None.
*******************************************************************************/
void ST7579_Clear(void)
{
	unsigned short clearPosition;
	unsigned char yPosition = 0x0;	
	for(clearPosition = 0;clearPosition < (64 * 96);clearPosition ++)
	{
		if(!(clearPosition % 96))
		{
			ST7579_RS_LOW;
			/* Set X address of RAM. */
			ST7579_WriteByte(0x28);
			ST7579_WriteByte(0x80);
			/* Set Y address of RAM. */
			ST7579_WriteByte(0x28);
			ST7579_WriteByte(0x40 + yPosition);
			yPosition ++;
			ST7579_RS_HIGH;
		}
		ST7579_WriteByte(0x00);
	}
}

/***************************************************************************//**
 * @brief Sends a character to ST7579 controller.
 *
 * @param yPosition - Y address of RAM.
 * @param xPosition - X address of RAM.
 * @param character - The character.
 *
 * @return None.
*******************************************************************************/
void ST7579_Char(unsigned char yPosition,
                 unsigned char xPosition,
                 unsigned char character)
{
	volatile unsigned char column;
	ST7579_RS_LOW;
	/* Set X address of RAM. */
	ST7579_WriteByte(0x28);
	ST7579_WriteByte(0x80 + xPosition);
	/* Set Y address of RAM. */
	ST7579_WriteByte(0x28);
	ST7579_WriteByte(0x40 + yPosition);
	ST7579_RS_HIGH;
	/* Send character data. */
	for(column = 0;column < 5;column ++)
	{
		ST7579_WriteByte(st7579Font[character - 32][column] << 1);
	}
	ST7579_WriteByte(0x00);
}

/***************************************************************************//**
 * @brief Sends a string to ST7579 controller.
 *
 * @param yPosition - Y address of RAM.
 * @param xPosition - X address of RAM.
 * @param string - The string.
 *
 * @return None.
*******************************************************************************/
void ST7579_String(unsigned char yPosition,
                   unsigned char xPosition,
                   unsigned char* string)
{
	while(*string)
	{
		/* Send each character of the string. */
		ST7579_Char(yPosition, xPosition, *string++);
		xPosition += 6;
	}
}

/***************************************************************************//**
 * @brief Sends an integer number to ST7579 controller.
 *
 * @param yPosition - Y address of RAM.
 * @param xPosition - X address of RAM.
 * @param number - The number.
 * @param charNumber - Number of characters.
 *
 * @return None.
*******************************************************************************/
void ST7579_Number(unsigned char yPosition,
                   unsigned char xPosition,
                   unsigned int number,
				   unsigned char charNumber)
{
	while(charNumber--)
	{
		ST7579_Char(yPosition, xPosition, (number % 10) + 0x30);
		xPosition -= 6;
		number = number / 10;
	}
}

/***************************************************************************//**
 * @brief Sends an integer number in Hexa format to ST7579 controller.
 *
 * @param yPosition - Y address of RAM.
 * @param xPosition - X address of RAM.
 * @param number - The number.
 * @param bytesNumber - Number of bytes.
 *
 * @return None.
*******************************************************************************/
void ST7579_HexNumber(unsigned char yPosition,
					  unsigned char xPosition,
					  unsigned int number,
					  unsigned char bytesNumber)
{
	char position = bytesNumber * 2;
	char character = 0;
	while(position--)
	{
		character = (number % 16);
		if (character <= 9)
		{
			character += 0x30;
		}
		else
		{
			character +=0x37;
		}
		ST7579_Char(yPosition, xPosition, character);
		xPosition -= 6;
		number = number / 16;
	}
}

/***************************************************************************//**
 * @brief Sends an image array to ST7579 controller.
 *
 * @param yPosition - Y address of RAM.
 * @param xPosition - X address of RAM.
 * @param image - The image array.
 * @param width - The width of the image (pixels).
 * @param height - The height of the image (pixels).
 *
 * @return None.
*******************************************************************************/
void ST7579_Image(unsigned char yPosition,
                  unsigned char xPosition,
                  const unsigned char image[],
                  unsigned char width,
                  unsigned char height)
{
	unsigned short index;
	for(index = 0;index < (width * height / 8);index ++)
	{
		if(!(index % width))
		{
			ST7579_RS_LOW;
			/* Set X address of RAM. */
			ST7579_WriteByte(0x28);
			ST7579_WriteByte(0x80 + xPosition);
			/* Set Y address of RAM. */
			ST7579_WriteByte(0x28);
			ST7579_WriteByte(0x40 + yPosition);
			yPosition ++;
			ST7579_RS_HIGH;
		}
		ST7579_WriteByte(image[index]);
	}
}
