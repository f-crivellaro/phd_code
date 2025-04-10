/***************************************************************************//**
 *   @file   AD9834.h
 *   @brief  Header file of AD9834 Driver.
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
#ifndef _AD9834_H_
#define _AD9834_H_

/******************************************************************************/
/* Include Files                                                              */
/******************************************************************************/
#include "Communication.h"

/******************************************************************************/
/* AD9834                                                                    */
/******************************************************************************/
/* GPIO pins */

#define PSEL_PIN_OUT    	R_IO_PORT_Set(PSEL_PIN, PDL_IO_PORT_OUTPUT)
#define FSEL_PIN_OUT    	R_IO_PORT_Set(FSEL_PIN, PDL_IO_PORT_OUTPUT)
#define PSEL_LOW        	R_IO_PORT_Write(PSEL_PIN, 0)
#define PSEL_HIGH       	R_IO_PORT_Write(PSEL_PIN, 1)
#define FSEL_LOW        	R_IO_PORT_Write(FSEL_PIN, 0)
#define FSEL_HIGH       	R_IO_PORT_Write(FSEL_PIN, 1)

/* Registers */
#define AD9834_REG_CMD		(0 << 14)
#define AD9834_REG_FREQ0	(1 << 14)
#define AD9834_REG_FREQ1	(2 << 14)
#define AD9834_REG_PHASE0	(6 << 13)
#define AD9834_REG_PHASE1	(7 << 13)

/* Command Control Bits */

#define AD9834_B28			(1 << 13)
#define AD9834_HLB			(1 << 12)
#define AD9834_FSEL0		(0 << 11)
#define AD9834_FSEL1		(1 << 11)
#define AD9834_PSEL0		(0 << 10)
#define AD9834_PSEL1		(1 << 10)
#define AD9834_CMD_PIN		(1 << 9)
#define AD9834_CMD_SW		(0 << 9)
#define AD9834_RESET		(1 << 8)
#define AD9834_SLEEP1		(1 << 7)
#define AD9834_SLEEP12		(1 << 6)
#define AD9834_OPBITEN		(1 << 5)
#define AD9834_SIGN_PIB		(1 << 4)
#define AD9834_DIV2			(1 << 3)
#define AD9834_MODE			(1 << 1)

#define AD9834_OUT_SINUS	((0 << 5) | (0 << 1))
#define AD9834_OUT_TRIANGLE	((0 << 5) | (1 << 1))

/******************************************************************************/
/* Functions Prototypes                                                       */
/******************************************************************************/
/* Initializes the SPI communication peripheral and resets the part. */
unsigned char AD9834_Init(void);
/* Sets the Reset bit of the AD9834. */
void AD9834_Reset(void);
/* Clears the Reset bit of the AD9834. */
void AD9834_ClearReset(void);
/* Writes the value to a register. */
void AD9834_SetRegisterValue(unsigned short regValue);
/* Writes to the frequency registers. */
void AD9834_SetFrequency(unsigned short reg, unsigned long val);
/* Writes to the phase registers. */
void AD9834_SetPhase(unsigned short reg, unsigned short val);
/* Selects the Frequency,Phase and Waveform type. */
void AD9834_Setup(unsigned short freq,
				  unsigned short phase,
			 	  unsigned short type,
				  unsigned short commandType);
#endif // _AD9834_H
