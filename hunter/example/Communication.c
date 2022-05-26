/***************************************************************************//**
 *   @file   Communication.c
 *   @brief  Implementation of Communication Driver for RENESAS RX62N
 *           Processor.
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
#include "Communication.h"

/***************************************************************************//**
 * @brief Initializes the SPI communication peripheral.
 *
 * @param lsbFirst - Transfer format (0 or 1).
 *                   Example: 0x0 - MSB first.
 *                            0x1 - LSB first.
 * @param clockFreq - SPI clock frequency (Hz).
 *                    Example: 1000 - SPI clock frequency is 1 kHz.
 * @param clockPol - SPI clock polarity (0 or 1).
 *                   Example: 0x0 - idle state for SPI clock is low.
 *	                          0x1 - idle state for SPI clock is high.
 * @param clockPha - SPI clock phase (0 or 1).
 *                   Example: 0x0 - data is latched on the leading edge of SPI
 *                                  clock and data changes on trailing edge.
 *                            0x1 - data is latched on the trailing edge of SPI
 *                                  clock and data changes on the leading edge.
 *
 * @return 0 - Initialization failed, 1 - Initialization succeeded.
*******************************************************************************/
unsigned char SPI_Init(unsigned char lsbFirst,
                       unsigned long clockFreq,
                       unsigned char clockPol,
                       unsigned char clockPha)
{
	MISOA_PIN_IN;
	MOSIA_PIN_OUT;
    ST7579_CS_PIN_OUT;
    ST7579_CS_HIGH;
    ADI_CS_PIN_OUT;
    ADI_CS_HIGH;
	R_SPI_Create(0,							// Channel selection.
				 PDL_SPI_MODE_SPI_MASTER |	// Connection mode.
				 PDL_SPI_PIN_A |			// A pins for signal MISO, MOSI, RSPCK, SSL0, SSL1, SSL2 and SSL3.
				 PDL_SPI_PIN_MOSI_IDLE_LOW,	// The MOSI output state when no SSLn pin is active.
				 PDL_SPI_FRAME_1_1,			// Frame configuration selection.
				 PDL_NO_DATA,				// Extended timing control - default settings.
                 0x80000000);				// Bit rate or register value.
	R_SPI_Command(0,						// Channel selection.
				  0,						// Command selection.
				  PDL_SPI_CLOCK_MODE_3 |	// Clock is high when idle; data is sampled on the rising edge.
				  PDL_SPI_DIV_8 |			// Bit rate (specified for R_SPI_Create) : 8.
				  PDL_SPI_LENGTH_8 |	    // The number of bits in the frame transfer.
				  PDL_SPI_MSB_FIRST,	    // Most-significant bit first.
                  PDL_NO_DATA);			    // Extended timing control - default settings.
	
    return(1);
}

/***************************************************************************//**
 * @brief Writes data to SPI.
 *
 * @param data - Write data buffer:
 *               - first byte is the chip select number;
 *               - from the second byte onwards are located data bytes to write.
 * @param bytesNumber - Number of bytes to write.
 *
 * @return Number of written bytes.
*******************************************************************************/
unsigned char SPI_Write(unsigned char* data,
                        unsigned char bytesNumber)
{
	unsigned char chipSelect    = data[0];
	unsigned char writeData[5]	= {0,0, 0, 0, 0};
	unsigned char byte          = 0;
	for(byte = 0;byte < bytesNumber;byte ++)
	{
		writeData[byte] = data[byte + 1];
	}
	if(chipSelect == 3)
	{
		// Change SPI Clock mode
		R_SPI_Command(0,					// Channel selection.
				  0,						// Command selection.
				  PDL_SPI_CLOCK_MODE_2 |	// Clock is high when idle; data is sampled on the falling edge.
				  PDL_SPI_DIV_8 |			// Bit rate (specified for R_SPI_Create) : 8.
				  PDL_SPI_LENGTH_8 |	    // The number of bits in the frame transfer.
				  PDL_SPI_MSB_FIRST,	    // Most-significant bit first.
                  PDL_NO_DATA);			    // Extended timing control - default settings.	
	}
    if(chipSelect == 1)
    {
        ADI_CS_LOW;
    }
    if(chipSelect == 2)
    {
		// Change SPI Clock mode
		R_SPI_Command(0,					// Channel selection.
				  0,						// Command selection.
				  PDL_SPI_CLOCK_MODE_3 |	// Clock is high when idle; data is sampled on the rising edge.
				  PDL_SPI_DIV_8 |			// Bit rate (specified for R_SPI_Create) : 8.
				  PDL_SPI_LENGTH_8 |	    // The number of bits in the frame transfer.
				  PDL_SPI_MSB_FIRST,	    // Most-significant bit first.
                  PDL_NO_DATA);			    // Extended timing control - default settings.
        ST7579_CS_LOW;
    }
	for(byte = 0;byte < bytesNumber;byte ++)
    {
        R_SPI_Transfer(0,						    		// Channel selection.
                       PDL_NO_DATA,							// DMAC / DTC control.
                       (unsigned long*)&writeData[byte],	// Transmit data start address.
                       PDL_NO_PTR,							// Receive data start address.
                       1,									// Sequence loop count.
                       PDL_NO_FUNC,							// Callback function.
                       0);									// Interrupt priority level.
    }
 	if(chipSelect == 1)
	{
		ADI_CS_HIGH;
	}
    if(chipSelect == 2)
	{
		ST7579_CS_HIGH;
	}
	return(bytesNumber);
}

/***************************************************************************//**
 * @brief Reads data from SPI.
 *
 * @param data - As an input parameter, data represents the write buffer:
 *               - first byte is the chip select number;
 *               - from the second byte onwards are located data bytes to write.
 *               As an output parameter, data represents the read buffer:
 *               - from the first byte onwards are located the read data bytes. 
 * @param bytesNumber - Number of bytes to write.
 *
 * @return Number of written bytes.
*******************************************************************************/
unsigned char SPI_Read(unsigned char* data,
                       unsigned char bytesNumber)
{
	unsigned char chipSelect    = data[0];
	unsigned char writeData[5]	= {0, 0, 0, 0, 0};
	unsigned char byte          = 0;
	for(byte = 0;byte < bytesNumber;byte ++)
	{
		writeData[byte] = data[byte + 1];
	}
	if(chipSelect == 3)
	{
		// Change SPI Clock mode
		R_SPI_Command(0,					// Channel selection.
				  0,						// Command selection.
				  PDL_SPI_CLOCK_MODE_2 |	// Clock is high when idle; data is sampled on the falling edge.
				  PDL_SPI_DIV_8 |			// Bit rate (specified for R_SPI_Create) : 8.
				  PDL_SPI_LENGTH_8 |	    // The number of bits in the frame transfer.
				  PDL_SPI_MSB_FIRST,	    // Most-significant bit first.
                  PDL_NO_DATA);			    // Extended timing control - default settings.	
	}
    if(chipSelect == 1)
    {
        ADI_CS_LOW;
    }
    if(chipSelect == 2)
    {
		// Change SPI Clock mode
		R_SPI_Command(0,					// Channel selection.
				  0,						// Command selection.
				  PDL_SPI_CLOCK_MODE_3 |	// Clock is high when idle; data is sampled on the rising edge.
				  PDL_SPI_DIV_8 |			// Bit rate (specified for R_SPI_Create) : 8.
				  PDL_SPI_LENGTH_8 |	    // The number of bits in the frame transfer.
				  PDL_SPI_MSB_FIRST,	    // Most-significant bit first.
                  PDL_NO_DATA);			    // Extended timing control - default settings.
        ST7579_CS_LOW;
    }
	for(byte = 0;byte < bytesNumber;byte ++)
    {
        R_SPI_Transfer(0,						    		// Channel selection.
                       PDL_NO_DATA,							// DMAC / DTC control.
                       (unsigned long*)&writeData[byte],	// Transmit data start address.
                       (unsigned long*)&data[byte],			// Transmit data start address.
                       1,									// Sequence loop count.
                       PDL_NO_FUNC,							// Callback function.
                       0);									// Interrupt priority level.
    }
 	if(chipSelect == 1)
	{
		ADI_CS_HIGH;
	}
    if(chipSelect == 2)
	{
		ST7579_CS_HIGH;
	}

	return(bytesNumber);
}