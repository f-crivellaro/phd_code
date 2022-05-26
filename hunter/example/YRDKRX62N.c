/***************************************************************************//**
 *   @file   YRDKRX62N.h
 *   @brief  Header file of YRDKRX62N Driver.
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
#include "YRDKRX62N.h"

/***************************************************************************//**
 * @brief Reset the YRDKRX62N's peripherals.
 *
 * @param None.
 *
 * @return None.
*******************************************************************************/
void YRDKRL78G13_Reset(void)
{
    unsigned short delay;
	ST7579_RESET_PIN_OUT;
	ST7579_RESET_HIGH;
    for(delay = 0;delay < 1000;delay ++)
    {
		nop();
    }
	ST7579_RESET_LOW;
}

/***************************************************************************//**
 * @brief Initializes the YRDKRX62N.
 *
 * @param None.
 *
 * @return None.
*******************************************************************************/
void YRDKRX62N_Init(void)
{
	YRDKRL78G13_Reset();
	R_CGC_Set(12.0E6,				// Input frequency 12 MHz.
			  12E6,					// System frequency 12 MHz.
			  12E6,					// Peripheral module frequency 12 MHz.
			  PDL_NO_DATA,			// External bus clock frequency.
			  PDL_CGC_BCLK_HIGH);	// Configuration options.
	/* All LEDs connected to Port D off. */
	R_IO_PORT_Write(PDL_IO_PORT_D, 0xFF);
	/* Port D - Output. */
    R_IO_PORT_Set(LED14_PIN | LED10_PIN | LED6_PIN  | LED12_PIN |
				  LED8_PIN  | LED4_PIN  | LED15_PIN | LED11_PIN,
				  PDL_IO_PORT_OUTPUT);
	/* All LEDs connected to Port E off. */
	R_IO_PORT_Write(PDL_IO_PORT_E, 0x0F);
	/* Port E - Pins 0, 1, 2, 3 output. */
    R_IO_PORT_Set(LED7_PIN | LED13_PIN | LED9_PIN | LED5_PIN,
				  PDL_IO_PORT_OUTPUT);
	/* Port 4 - Pins 0, 1, 2 input. */
	R_IO_PORT_Set(SW1_PIN | SW2_PIN | SW3_PIN,
				  PDL_IO_PORT_INPUT | PDL_IO_PORT_INPUT_BUFFER_ON);
}
