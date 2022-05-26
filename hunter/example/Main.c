/***************************************************************************//**
 *   @file   Main.c
 *   @brief  Implementation of the program's main function.
 *   @author Bancisor Mihai
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
#include "YRDKRX62N.h"
#include "ST7579.h"
#include "AD9834.h"

/******************************************************************************/
/* Variables Declarations                                                     */
/******************************************************************************/
const unsigned char adiLogo [2 * 19] =
	{ 0xFF, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x07, 0x07,
	  0x0F, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F, 0x7F, 0x7F,
	  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xC0,
	  0xC0, 0xE0, 0xE0, 0xF0, 0xF0, 0xF8, 0xF8, 0xFC,
	  0xFC, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF };	// ADI Logo.
unsigned char 	periodEnd	= 0;	// Indicates when the OneShot period ends.

/***************************************************************************//**
 * @brief OneShot callback function.
 *
 * @param None.
 *
 * @return None.
*******************************************************************************/
void OneShotCallback(void)
{
	periodEnd = 1;
}

/***************************************************************************//**
 * @brief Creates a delay of seconds.
 *
 * @param seconds - time in seconds.
 *
 * @return None.
*******************************************************************************/
void DelaySeconds(float seconds)
{
	periodEnd = 0;
	R_TMR_CreateOneShot(PDL_TMR_UNIT0,
						PDL_TMR_OUTPUT_OFF,
						seconds,
						OneShotCallback,
						4);
	while(periodEnd == 0);
}

/***************************************************************************//**
 * @brief Main function.
 *
 * @param None.
 *
 * @return None.
*******************************************************************************/
void main(void)
{	
	YRDKRX62N_Init();
	ST7579_Init();
    ST7579_Image(2,0,adiLogo,19,16);
    ST7579_String(2,21,"ANALOG");
    ST7579_String(3,21,"DEVICES");
    ST7579_String(4,0,"wiki.analog.com");
	DelaySeconds(2);
	ST7579_Clear();
	ST7579_Image(0,0,adiLogo,19,16);	
	ST7579_String(0,22,"ST7579 OK"); 
    if(AD9834_Init())
    {
        ST7579_String(1, 22, "AD9834 OK");
    }
    else
    {
        ST7579_String(1 ,22, "AD9834 Err");
    }
	
	AD9834_Reset();
	AD9834_SetFrequency(AD9834_REG_FREQ0, 0x598);	// 400 Hz
	AD9834_SetFrequency(AD9834_REG_FREQ1, 0x15D9C0);// 400 kHz
	AD9834_SetPhase(AD9834_REG_PHASE0,0x00);		 
	AD9834_SetPhase(AD9834_REG_PHASE1,0x00);
	AD9834_ClearReset();
	ST7579_String(3,10, "Output signal: ");
	while(1)
	{	
		ST7579_String(2,5, "SW select:  ");
		ST7579_String(4,25, "Sinusoid  ");
		ST7579_String(5, 30, "400 Hz  ");
		AD9834_Setup(AD9834_FSEL0, 
					 AD9834_PSEL1, 
					 AD9834_OUT_SINUS, 
					 AD9834_CMD_SW);
		DelaySeconds(1);
		ST7579_String(4, 25, "Sinusoid  ");
		ST7579_String(5, 30, "400 kHz   ");
		AD9834_Setup(AD9834_FSEL1, 
					 AD9834_PSEL1, 
					 AD9834_OUT_SINUS,
					 AD9834_CMD_SW);
		DelaySeconds(1);
		ST7579_String(4,25, "Triangle  ");
		ST7579_String(5, 30, "400 Hz  ");
		AD9834_Setup(AD9834_FSEL0, 
					 AD9834_PSEL1, 
					 AD9834_OUT_TRIANGLE, 
					 AD9834_CMD_SW);
		DelaySeconds(1);
		ST7579_String(4,25, "Triangle  ");
		ST7579_String(5, 30, "400 kHz  ");
		AD9834_Setup(AD9834_FSEL1, 
					 AD9834_PSEL1,
					 AD9834_OUT_TRIANGLE, 
					 AD9834_CMD_SW);
		DelaySeconds(1);
	
		ST7579_String(2,5, "PIN select:  ");
		ST7579_String(4,25, "Sinusoid  ");
		ST7579_String(5, 30, "400 Hz  ");
		AD9834_Setup(AD9834_FSEL0, 
					 AD9834_PSEL1, 
					 AD9834_OUT_SINUS, 
					 AD9834_CMD_PIN);
		DelaySeconds(1);
		ST7579_String(4, 25, "Sinusoid  ");
		ST7579_String(5, 30, "400 kHz   ");
		AD9834_Setup(AD9834_FSEL1, 
					 AD9834_PSEL1, 
					 AD9834_OUT_SINUS, 
					 AD9834_CMD_PIN);
		DelaySeconds(1);
	
	}
}
