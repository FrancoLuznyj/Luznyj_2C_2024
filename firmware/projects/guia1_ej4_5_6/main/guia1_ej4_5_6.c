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
 * Ejercicio 6
 * Se crea una funcion que recibe un dato de 32 bits y se vale de las funciones de los ejercicios 4 y 5 para
 * separar el numero en sus digitos y luego convertir cada digito a su expresion binario con cuatro bits.
 * Ademas, a medida que convierte cada digito del dato en su expresion binaria, selecciona uno de tres display
 * disponible y lo activa, dependiendo del orden que corresponde, para mostrar un digito del numero. Al activar
 * los tres display, se muestra el numero completo. 
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	D1	    	| 	GPIO_20		|
 * | 	D2	    	| 	GPIO_21		|
 * | 	D3	 	    | 	GPIO_22		|
 * | 	D4  	 	| 	GPIO_23		|
 * | 	SEL_1   	| 	GPIO_19		|
 * | 	SEL_2	 	| 	GPIO_18		|
 * | 	SEL_3	 	| 	GPIO_9		|
 * | 	+5V 	 	| 	+5V 		|
 * | 	GND 	 	| 	GND 		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 28/08/2024 | Document creation		                         |
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
/**
 * @brief Estructura que permite configurar el pin y direccion de cada GPIO
 * 
 */
typedef struct 
{
	gpio_t pin;
	io_t dir;
} gpioConf_t;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/**
 * @brief  Convierte el dato recibido a BCD
 * 
 * @param data Entero sin signo de 32 bits que representa el dato que se pasa
 * @param digits Entero sin signo de 8 bits que representa la cantidad de digitos del dato
 * @param bcd_number Puntero de tipo entero sin signo de 8 bits al arreglo donde se almacenan los digitos
 */
void convertToBcdArray (uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	for (uint8_t i = 0; i < digits; i++)
	{
		bcd_number[digits-1-i] = data % 10;
		data /=10; // divide a 'data' por 10 y devuelve el resultado entero; ej: 245/10= 24
	}
	
}

/**
 * @brief Cambia el estado de un GPIO de '0' a '1' y viceversa
 * 
 * @param numeroBCD Entero sin signo de 8 bits que representa un digito en codigo BCD
 * @param vector Puntero a vector de estructura de tipo gpioConf_t que permite configurar el pin y la direccion
 * de los distintos GPIO
 */
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

/**
 * @brief Muestra por display el valor que recibe
 * 
 * @param dato Entero sin signo de 32 bits que representa el dato que se pasa
 * @param digitos Entero sin signo de 8 bits que representa la cantidad de digitos del dato
 * @param vector_1 Puntero a vector de estructura de tipo gpioConf_t que permite configurar el pin y
 * la direccion de los distintos GPIO
 * @param vector_mapeo Puntero a vector de estructura de tipo gpioConf_t que mapea los puertos con el digito del LCD a donde mostrar un dato
 */
void mostrarNumeroEnDisplay(uint32_t dato, uint8_t digitos, gpioConf_t *vector_1, gpioConf_t *vector_mapeo)
{
	uint8_t arreglo_aux[digitos];
	
	convertToBcdArray(dato, digitos, arreglo_aux);

	for (uint8_t i = 0; i < 3; i++)
	{
		cambiarEstado(arreglo_aux[i],vector_1);
		GPIOOn(vector_mapeo[i].pin);
		GPIOOff(vector_mapeo[i].pin);
	}
	
}

/*==================[external functions definition]==========================*/
void app_main(void){

//Ejercicio 4

	/*uint8_t arreglo[3];

	convertToBcdArray(138,3,arreglo);

	for (uint8_t i = 0; i < 3; i++)
	{
		printf("%d",arreglo[i]);
	}*/


//Ejercicio 5

	// gpioConf_t vector []=
	// {
	// 	{GPIO_20,GPIO_OUTPUT},
	// 	{GPIO_21,GPIO_OUTPUT},
	// 	{GPIO_22,GPIO_OUTPUT},
	// 	{GPIO_23,GPIO_OUTPUT}
	// };

	// for (uint8_t i = 0; i < 4; i++) // el numero 4 es porque son 4 elementos en el vector de tipo gpioConf_t
	// {
	// 	GPIOInit(vector[i].pin,vector[i].dir);
	// }

	// cambiarEstado(6,vector);


//Ejercicio 6

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

	gpioConf_t vector_mapeo []= //bit mas significativo GPIO_19
	{
		{GPIO_19,GPIO_OUTPUT},
		{GPIO_18,GPIO_OUTPUT},
		{GPIO_9,GPIO_OUTPUT},
	};
	
	for(uint8_t i = 0; i < 3; i++)
	{
		GPIOInit(vector_mapeo[i].pin,vector_mapeo[i].dir);
	}

	mostrarNumeroEnDisplay(196,3,vector,vector_mapeo);
}
/*==================[end of file]============================================*/