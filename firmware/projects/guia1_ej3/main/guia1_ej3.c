/*! @mainpage guia1_ej3
 *
 * @section genDesc General Description
 *
 * La funcion enciende y apaga un led de manera intermitente estableciendo el modo TOGGLE, segun los ciclos 
 * de encendido y apagado que se establecen y la duración del periodo. Ademas, se tienen los modos ON y OFF
 * para encender o apagar un led.
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
 * | 12/09/2023 | Document creation		                         |
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
/*==================[macros and definitions]=================================*/
#define ON 1
#define OFF 0
#define TOGGLE 2
#define CONFIG_BLINK_PERIOD 100

/*==================[internal data definition]===============================*/
struct leds
{
	uint8_t mode;	  // ON, OFF, TOGGLE
	uint8_t n_led;	  // indica el número de led a controlar
	uint8_t n_ciclos; // indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo; // indica el tiempo de cada ciclo
} my_leds;
/*==================[internal functions declaration]=========================*/
void nombreFuncion(struct leds *puntero)
{
	switch (puntero->mode)
	{
	case ON:
		switch (puntero->n_led)
		{
		case 1:
			LedOn(LED_1);
			break;

		case 2:
			LedOn(LED_2);
			break;

		case 3:
			LedOn(LED_3);
			break;

		default:
			break;
		}

	case OFF:
		switch (puntero->n_led)
		{
		case 1:
			LedOff(LED_1);
			break;

		case 2:
			LedOff(LED_2);
			break;

		case 3:
			LedOff(LED_3);
			break;
		}

	case TOGGLE:
		for (uint8_t i = 0; i < puntero->n_ciclos; i++)
		{
			switch (puntero->n_led)
			{
			case 1:
				LedToggle(LED_1);
				break;

			case 2:
				LedToggle(LED_2);
				break;

			case 3:
				LedToggle(LED_3);
				break;
			}

			for (uint8_t j = 0; j < puntero->periodo / CONFIG_BLINK_PERIOD; j++)
			{
				vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
			}
		}
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	my_leds.periodo = 500;
	my_leds.n_ciclos = 10;
	my_leds.n_led = 1;
	my_leds.mode = TOGGLE;

	LedsInit();
	SwitchesInit();

	nombreFuncion(&my_leds);
}
/*==================[end of file]============================================*/