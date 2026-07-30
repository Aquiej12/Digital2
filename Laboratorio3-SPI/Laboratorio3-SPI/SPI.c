/*
 * SPI.c
 *
 * Created: 23/07/2026 15:39:27
 *  Author: abner
 */

#include "SPI.h"

void spiInit(Spi_Type sType, Spi_Data_Order sDataOrder, SPI_Clock_Polarity sClockPolarity, SPI_Clock_Phase sClockPhase)
{
    // PB2 - SS
    // PB3 - MOSI
    // PB4 - MISO
    // PB5 - SCK

    if(sType & 0b00010000) //Master
    {
        DDRB |= (1<<PB2) | (1<<PB3) | (1<<PB5); //SS, MOSI, SCK output
        DDRB &= ~(1<<PB4); //MISO  input
        SPCR |= (1<<MSTR); //Master mode

        uint8_t temp = sType & 0b00000111;
        switch (temp)
        {
        case 0: // Div 2
            SPCR &= ~((1<<SPR1) | (1<<SPR0));
            SPSR |= (1<<SPI2X);
        break;

        case 1: // Div 4
            SPCR &= ~((1<<SPR1) | (1<<SPR0));
            SPSR &= ~(1<<SPI2X);
        break;

        case 2: // Div 8
            SPCR |= (1<<SPR0);
            SPCR &= ~(1<<SPR1);
            SPSR |= (1<<SPI2X);
        break;

        case 3: // Div 16
            SPCR |= (1<<SPR0);
            SPCR &= ~(1<<SPR1);
            SPSR &= ~(1<<SPI2X);
        break;

        case 4: // Div 32
            SPCR &= ~(1<<SPR0);
            SPCR |= (1<<SPR1);
            SPSR |= (1<<SPI2X);
        break;

        case 5: // Div 64
            SPCR &= ~(1<<SPR0);
            SPCR |= (1<<SPR1);
            SPSR &= ~(1<<SPI2X);
        break;

        case 6: // Div 128
            SPCR |= (1<<SPR0) | (1<<SPR1);
            SPSR &= ~(1<<SPI2X);
        break;
        }
    }
    else //Slave
    {
        DDRB |= (1<<DDB4); //MISO output
        DDRB &= ~((1<<DDB2) | (1<<DDB3) | (1<<DDB5)); //SS, MOSI, SCK input
        SPCR &= ~(1<<MSTR); //Slave mode
    }

    // Orden de datos, polaridad, fase y habilitación del SPI
    SPCR |= (1<<SPE) | sDataOrder | sClockPolarity | sClockPhase;
}

void spiWrite(uint8_t dat)
{
    SPDR = dat;
    while (!(SPSR & (1<<SPIF))); // Esperar a que termine la transmisión (maestro)
    (void)SPDR;                  // Leer SPDR para limpiar SPIF
}

unsigned spiDataReady()
{
    if(SPSR & (1<<SPIF))
    return 1;
    else
    return 0;
}

char spiRead()
{
    SPDR = 0x00;                 // Enviar dummy para generar el clock
    while (!(SPSR & (1<<SPIF))); // Esperar a que la transmisión se complete
    return(SPDR); // Leer el dato recibido
}
