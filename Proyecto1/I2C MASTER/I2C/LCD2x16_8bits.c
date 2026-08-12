/*
 * LCD2x16_8bits.c  -  Misma libreria del proyecto, pines adaptados al master.
 * (ver tabla de pines en LCD2x16_8bits.h)
 */
#include "LCD2x16_8bits.h"

// Funcion para inicializar modo en 8 bits
void LCD16x2_8bits_init(void){
	DDRD |= (1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5)|(1<<DDD6)|(1<<DDD7);	// RS, RW, E, D0-D2
	DDRB |= (1<<DDB0)|(1<<DDB1)|(1<<DDB2)|(1<<DDB3)|(1<<DDB4);				// D3-D7
	PORTD &= ~(RS | RW | E);   // RW siempre en escritura (0)

	LCD16x2_8bits_port(0x00);
	_delay_ms(20);
	LCD16x2_8bits_cmd(0x30);
	_delay_ms(5);
	LCD16x2_8bits_cmd(0x30);
	_delay_ms(11);
	LCD16x2_8bits_cmd(0x30);

	//Function Set
	LCD16x2_8bits_cmd(0x38);

	//Display On Off
	LCD16x2_8bits_cmd(0x0C);

	//Entry Mode
	LCD16x2_8bits_cmd(0x06);

	//Clear Display
	LCD16x2_8bits_cmd(0x01);
	_delay_ms(2);
}

// Funcion para colocar en el puerto un valor
void LCD16x2_8bits_port(char a){
	if (a & 1)			// D0 -> PD5
		PORTD |= (1<<PORTD5);
	else
		PORTD &= ~(1<<PORTD5);

	if (a & 2)			// D1 -> PD6
		PORTD |= (1<<PORTD6);
	else
		PORTD &= ~(1<<PORTD6);

	if (a & 4)			// D2 -> PD7
		PORTD |= (1<<PORTD7);
	else
		PORTD &= ~(1<<PORTD7);

	if (a & 8)			// D3 -> PB0
		PORTB |= (1<<PORTB0);
	else
		PORTB &= ~(1<<PORTB0);

	if (a & 16)			// D4 -> PB1
		PORTB |= (1<<PORTB1);
	else
		PORTB &= ~(1<<PORTB1);

	if (a & 32)			// D5 -> PB2
		PORTB |= (1<<PORTB2);
	else
		PORTB &= ~(1<<PORTB2);

	if (a & 64)			// D6 -> PB3
		PORTB |= (1<<PORTB3);
	else
		PORTB &= ~(1<<PORTB3);

	if (a & 128)		// D7 -> PB4
		PORTB |= (1<<PORTB4);
	else
		PORTB &= ~(1<<PORTB4);
}

// Funcion para enviar un comando
void LCD16x2_8bits_cmd(char a){
	PORTD &= ~RS;			// RS = 0, dato en el puerto a interpretar como comando
	LCD16x2_8bits_port(a);

	PORTD |= E;
	_delay_us(1);
	PORTD &= ~E;
	_delay_us(100);
}

// Funcion para enviar un caracter
void LCD16x2_8bits_char(char c){
	PORTD |= RS;			// RS = 1, dato en el puerto a interpretar como caracter
	LCD16x2_8bits_port(c);

	PORTD |= E;
	_delay_us(1);
	PORTD &= ~E;
	_delay_us(100);
}

// Funcion para enviar una cadena
void LCD16x2_8bits_strg(char *a){
	int i;
	for(i=0; a[i]!='\0'; i++)
		LCD16x2_8bits_char(a[i]);
}

// Funcion para enviar un numero entero
void LCD16x2_8bits_int(int numero){
	char buffer[7];
	int i = 0;
	unsigned int magnitud;

	if(numero == 0){
		LCD16x2_8bits_char('0');
		return;
	}

	if(numero < 0){
		LCD16x2_8bits_char('-');
		magnitud = -numero;
	}
	else
		magnitud = numero;

	while(magnitud > 0){			// los digitos salen al reves
		buffer[i] = '0' + (magnitud % 10);
		magnitud /= 10;
		i++;
	}

	while(i > 0){				// y aqui se imprimen en orden
		i--;
		LCD16x2_8bits_char(buffer[i]);
	}
}

// Funcion para enviar un numero con decimales
void LCD16x2_8bits_float(float numero, char decimales){
	char i;
	unsigned long factor = 1;
	unsigned long escalado, entera, decimal, divisor;

	if(numero < 0){
		LCD16x2_8bits_char('-');
		numero = -numero;
	}

	for(i=0; i<decimales; i++)
		factor *= 10;

	escalado = numero * factor + 0.5;	// se corre el punto y se redondea
	entera = escalado / factor;
	decimal = escalado % factor;

	LCD16x2_8bits_int(entera);

	if(decimales == 0) return;

	LCD16x2_8bits_char('.');

	divisor = factor / 10;
	while(divisor > 0){			// digito por digito, para no perder los ceros
		LCD16x2_8bits_char('0' + (decimal / divisor) % 10);
		divisor /= 10;
	}
}

// Funcion para limpiar la pantalla
void LCD16x2_8bits_clear(void){
	LCD16x2_8bits_cmd(0x01);
	_delay_ms(2);
}

// Dezplazamiento a la derecha
void LCD16x2_8bits_shift_right(void){
	LCD16x2_8bits_cmd(0x1C);
}

// Dezplazamiento a la izquierda
void LCD16x2_8bits_shift_left(void){
	LCD16x2_8bits_cmd(0x18);
}

// Establecer el cursor
void LCD16x2_8bits_set_cursor(char c, char f){
	if(f == 1)
		LCD16x2_8bits_cmd(0x80 + c - 1);
	else if(f == 2)
		LCD16x2_8bits_cmd(0xC0 + c - 1);
}
