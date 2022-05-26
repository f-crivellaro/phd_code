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
#ifndef _YRDKRX62N_H_
#define _YRDKRX62N_H_

/******************************************************************************/
/* Include Files                                                              */
/******************************************************************************/
#include <r_pdl_cgc.h>			// RPDL definitions.
#include <r_pdl_io_port.h>		// RPDL definitions.
#include <r_pdl_spi.h>			// RPDL definitions.
#include <r_pdl_iic.h>			// RPDL definitions.
#include <r_pdl_tmr.h>			// RPDL definitions.
#include <r_pdl_definitions.h>	// RPDL device-specific definitions.

/******************************************************************************/
/* Macros and Constants Definitions                                           */
/******************************************************************************/
#define ST7579_CS_PIN		 PDL_IO_PORT_C_2
#define ST7579_CS_PIN_OUT	 R_IO_PORT_Set(ST7579_CS_PIN, PDL_IO_PORT_OUTPUT)
#define ST7579_CS_LOW		 R_IO_PORT_Write(ST7579_CS_PIN, 0)
#define ST7579_CS_HIGH		 R_IO_PORT_Write(ST7579_CS_PIN, 1)
#define ST7579_RS_PIN		 PDL_IO_PORT_5_1
#define ST7579_RS_PIN_OUT	 R_IO_PORT_Set(ST7579_RS_PIN, PDL_IO_PORT_OUTPUT)
#define ST7579_RS_LOW        R_IO_PORT_Write(ST7579_RS_PIN, 0)
#define	ST7579_RS_HIGH	     R_IO_PORT_Write(ST7579_RS_PIN, 1)
#define ST7579_RESET_PIN	 PDL_IO_PORT_C_3
#define ST7579_RESET_PIN_OUT R_IO_PORT_Set(ST7579_RESET_PIN, PDL_IO_PORT_OUTPUT)
#define ST7579_RESET_LOW	 R_IO_PORT_Write(ST7579_RESET_PIN, 0)
#define ST7579_RESET_HIGH	 R_IO_PORT_Write(ST7579_RESET_PIN, 1)
#define MOSIA_PIN			 PDL_IO_PORT_C_6
#define MOSIA_PIN_OUT		 R_IO_PORT_Set(MOSIA_PIN, PDL_IO_PORT_OUTPUT)
#define MISOA_PIN			 PDL_IO_PORT_C_7
#define MISOA_PIN_IN		 R_IO_PORT_Set(MISOA_PIN, PDL_IO_PORT_INPUT)
#define LED4_PIN			 PDL_IO_PORT_D_5
#define LED5_PIN			 PDL_IO_PORT_E_3
#define LED6_PIN			 PDL_IO_PORT_D_2
#define LED7_PIN			 PDL_IO_PORT_E_0
#define LED8_PIN			 PDL_IO_PORT_D_4
#define LED9_PIN			 PDL_IO_PORT_E_2
#define LED10_PIN			 PDL_IO_PORT_D_1
#define LED11_PIN			 PDL_IO_PORT_D_7
#define LED12_PIN			 PDL_IO_PORT_D_3
#define LED13_PIN			 PDL_IO_PORT_E_1
#define LED14_PIN			 PDL_IO_PORT_D_0
#define LED15_PIN			 PDL_IO_PORT_D_6
#define SW1_PIN				 PDL_IO_PORT_4_0
#define SW2_PIN				 PDL_IO_PORT_4_1
#define SW3_PIN				 PDL_IO_PORT_4_2
/******************************************************************************/
/* Functions Prototypes                                                       */
/******************************************************************************/
void YRDKRX62N_Init(void);
#endif	// _YRDKRX62N_H_
