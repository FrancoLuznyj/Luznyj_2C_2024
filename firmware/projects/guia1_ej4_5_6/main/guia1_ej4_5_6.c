/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Ejercicio 4
 * Se crea una funcion que recibe un numero y lo separa en sus digitos para guardarlos dentro de un arreglo
 * 
 * Ejercicio 5
 * Se crea una funcion que recibe un numero decimal y lo convierte a codigo BCD para que ponga en alto o en
 * bajo cada GPIO. Cada GPIO se comunica con una entrada de un conversor BCD a 7 segmentos. Por medio de un 
 * display se visualiza el numero con el que se trabajo.
 * 
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 14/08/2024 | Document creation		                         |
 *
 * @author Luznyj Franco (francoluznyj1999@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

typedef struct 
{
	gpio_t pin;
	io_t dir;
} gpioConf_t;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

void convertToBcdArray (uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	for (uint8_t i = 0; i < digits; i++)
	{
		bcd_number[digits-1-i] = data % 10;
		data /=10; // divide al 'data' por 10 y devuelve el resultado entero; ej: 245/10= 24
	}
	
}

void cambiarEstado(uint8_t numeroBCD, gpioConf_t *vector)
{
	uint8_t mascara=1;

	for (uint8_t i = 0; i < 4; i++)
	{
		if((numeroBCD & mascara))
		{
			GPIOOn(vector[i].pin);
		}
		
		else
		{
			GPIOOff(vector[i].pin);
		}
		mascara=mascara<<1;
	}
		
}

/*==================[external functions definition]==========================*/
void app_main(void){

//Ejercicio 4

	/*uint8_t arreglo[3];

	convertToBcdArray(245,3,arreglo);

	for (uint8_t i = 0; i < 3; i++)
	{
		printf("%d",arreglo[i]);
	}*/


//Ejercicio 5

	gpioConf_t vector []=
	{
		{GPIO_20,GPIO_OUTPUT},
		{GPIO_21,GPIO_OUTPUT},
		{GPIO_22,GPIO_OUTPUT},
		{GPIO_23,GPIO_OUTPUT}
	};

	for (uint8_t i = 0; i < 4; i++) // el numero 4 es porque son 4 elementos en el vector de tipo gpioConf_t
	{
		GPIOInit(vector[i].pin,vector[i].dir);
	}

	cambiarEstado(6,vector);
	
}
/*==================[end of file]============================================*/