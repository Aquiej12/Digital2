#include <LCD.h>

//Funcion para inicializar modo en 4 bits

void initLCD4bits(void){
	DDRC |= (1<<DDC0)|(1<<DDC1)|(1<<DDC2)|(1<<DDC3)|(1<<DDC4)|(1<<DDC5);
	PORTC = 0;
	
	LCD_PORT(0x00);
	_delay_ms(20);
	LCD_CMD(0x03);
	_delay_ms(5);
	LCD_CMD(0x03);
	_delay_ms(11);
	LCD_CMD(0x03);
	
	LCD_CMD(0x02);
	
	//Function Set
	LCD_CMD(0x02);
	LCD_CMD(0x08);
	
	//Display On OFF
	LCD_CMD(0x00);
	LCD_CMD(0x0c);
	
	//Entry/Mode
	LCD_CMD(0x00);
	LCD_CMD(0x06);
	
	//Clear/Display
	LCD_CMD(0x00);
	LCD_CMD(0x01);
}


//FUncion para enviar un comando
void LCD_CMD(char a){
	//RS = 0; // RS =0 // Dato en el puerto a interpretar como comando
	PORTC &= ~(1<<PORTC0);
	LCD_Port(a);
	
	PORTC &= ~(1<<PORTC1);
	
	_delay_ms(4);
	
	PORTC &= ~(1<<PORTC1);	
}


//Funcion para colocar en el puerto un valor
void LCD_Port(char a){
	if (a & 1)
	//D4 = 1;
	PORTC |= (1<<PORTC2);
	else
	PORTC &= ~(1<<PORTC2);
	
	if (a & 2)
	PORTC |= (1<<PORTC3);
	else 
	PORTC &= ~(1<<PORTC3);
	
	if (a & 4)
	PORTC |= (1<<PORTC4);
	else
	PORTC &= ~(1<<PORTC4);
	
	if (a & 8)
	PORTC |= ~(1<<PORTC5);
	else
	PORTC &= ~(1<<PORTC5);
	
	//Funcion para enviar un caracter
	
void LCD_Write_Char(char c){
	char Cbajo, Calto;
	Cbajo = c & 0x0f;
	Calto = (c & 0x0F)>>4;
	
	PORTC |= (1<<PORTC0);
	LCD_Port(Calto);
	PORTC |= (1<<PORTC1);
	_delay_ms(4);

	PORTC &= ~(1<<PORTC1);
	LCD_Port(Cbajo);
	PORTC |= (1<<PORTC1);
	_delay_ms(4);
	
	PORTC &= (1<<PORTC1);
	LCD_Port(Calto);
	PORTC |= (1<<PORTC1);
	_delay_ms(4);
	
	PORTC |= (1<<PORTC0);
	LCD_Port(Calto);
	PORTC |= (1<<PORTC1);
	_delay_ms(4);
}
}



// Funcion para enviar una cadena


void LCD_Write_String(char *a){
    int i;
    for(i=0;a[i]!='\0';i++)
        LCD_Write_Char(a[i]);
}

// Dezplazamiento a la derecha

void LCD_Shift_Right(void){
    LCD_CMD(0x01);
    LCD_CMD(0x0C);

}

// Dezplazamiento a la izquierda
void LCD_Shift_Left(void){
    LCD_CMD(0x01);
    LCD_CMD(0x08);

}

// Establecer el cursor
void LCD_Set_Cursor(char c, char f){
    char temp,z,y;
    if(f == 1){
        temp = 0x80 + c - 1;
        z = temp>>4;
        y = temp & 0x0F;
        LCD_CMD(z);
        LCD_CMD(y);
    }
    else if(f == 2){
        temp = 0xC0 + c - 1;
        z = temp>>4;
        y = temp & 0x0F;
        LCD_CMD(z);
        LCD_CMD(y);
    }
}