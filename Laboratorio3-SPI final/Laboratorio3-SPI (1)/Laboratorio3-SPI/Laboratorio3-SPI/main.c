/*
 * Laboratorio3-SPI-Master.c
 *
 * Descripcion: Laboroatorio spi y uart
 * Autor: Edwin Parada - Abner Quiej
 */

#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include "SPI.h"
#include "UART.h"

// Mapear valores de los leds
void mostrarEnLeds(uint8_t valor)
{
    // Bits 0 al 5 van a PD2 a PD7
    PORTD = (PORTD & 0x03) | (valor << 2);
    // Bits 6 y 7 van a PB0 y PB1
    PORTB = (PORTB & 0xFC) | (valor >> 6);
}

int main(void)
{
    // COnfiguracion de pines
    // SS esclavo
    DDRC |= (1<<DDC5);     
    PORTC |= (1<<PORTC5);  // SS = 1 (inactivo)

    // Pines leds D2 a D9
    DDRD |= 0xFC; // PD2-PD7
    DDRB |= 0x03; // PB0-PB1
	// Iniciar apagados
    mostrarEnLeds(0);

    // Iniciar modulos
    spiInit(SPI_MASTER_OSC_DIV4, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);
    initUART9600();

    // vaariables globales
    char rx_buffer[4]; 
    uint8_t rx_index = 0;
    uint8_t valor_8bits = 0;
    
    uint8_t pot1 = 0;
    uint8_t pot2 = 0;
	//Temporizador
    uint16_t timer_spi = 0; 

    writeTextUART("Sistema Maestro Iniciado.\r\n");
    writeTextUART("Ingrese valor LEDs (0-255):\r\n> ");

    while (1)
    {
        //Uart para enviar datos
		
        if (availableUART())
        {
            char c = readUART();
            
            // Si se presiona enter
            if (c == '\n' || c == '\r')
            {
                if (rx_index > 0)
                {
                    rx_buffer[rx_index] = '\0';
                    valor_8bits = (uint8_t)atoi(rx_buffer); 
                    rx_index = 0; 
                    
                    writeTextUART("\r\n[+] LEDs configurados a: ");
                    writeNumberUART(valor_8bits);
                    writeTextUART("\r\n> ");
                    
                    // 1. Mostrar en LEDs del Maestro
                    mostrarEnLeds(valor_8bits);
                    
                    // 2. Enviar el valor a los LEDs del Esclavo
                    PORTC &= ~(1<<PORTC5);
                    spiWrite(valor_8bits); 
                    PORTC |= (1<<PORTC5);
                }
            }
            // Si es un número válido, guardarlo en el buffer
            else if (c >= '0' && c <= '9')
            {
                if (rx_index < 3) 
                {
                    rx_buffer[rx_index++] = c;
                    writeUART(c); // Hacer "eco" en la terminal
                }
            }
        }


		//Leer los potenciometros del esclavo
		
        timer_spi++;
        if (timer_spi >= 250)
        {
            timer_spi = 0; // Reiniciar contador
            
            // Pedir Potenciómetro 1
            PORTC &= ~(1<<PORTC5); 
            spiWrite('c');         
            _delay_us(20);         
            pot1 = spiRead();      
            PORTC |= (1<<PORTC5);

            _delay_us(50); // Pausa de seguridad entre lecturas

            // Pedir Potenciómetro 2
            PORTC &= ~(1<<PORTC5); 
            spiWrite('d');         
            _delay_us(20);         
            pot2 = spiRead();      
            PORTC |= (1<<PORTC5);

            // Imprimir en la terminal
            writeTextUART("\r\n--- Lectura SPI ---");
            writeTextUART("\r\nPot1: ");
            writeNumberUART(pot1);
            writeTextUART(" | Pot2: ");
            writeNumberUART(pot2);
            writeTextUART("\r\n> ");
        }

		//delay de 1ms 255 veces
        _delay_ms(1);
    }
}