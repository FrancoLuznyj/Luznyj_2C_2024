/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
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
 * | 12/09/2024 | Document creation		                         |
 *
 * @author Luznyj Franco (francoluznyj1999@gmail.com) 
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "switch.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_MEDIR 1000
#define CONFIG_BLINK_PERIOD_TECLAS 1000
#define CONFIG_BLINK_PERIOD_DISTANCIA 1000
/*==================[internal data definition]===============================*/
bool encendido = false;
bool hold = false;
uint16_t distancia = 0;
/*==================[internal functions declaration]=========================*/

static void TeclasTasks(void *pvParameter){
		
	uint8_t teclas;
	
	while(true){

		teclas = SwitchesRead();
		switch(teclas){
			case SWITCH_1:
				encendido = !encendido;
			break;

			case SWITCH_2:
				hold = !hold;
			break;

		default:
			break;
		}
	}
}

static void MedirDistanciaTasks(void *pvParameter){

	while(true){
		if(encendido)
		{
			distancia= HcSr04ReadDistanceInCentimeter();
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_MEDIR/portTICK_PERIOD_MS);
	}
}

static void MostrarDistanciaTasks(void *pvParameter){

	while(true){
		if(encendido)
	{
		{
			if(distancia<10)
			{
				LedsOffAll();
			}
			else
			{
				if(distancia>=10 && distancia<=20)
				{
					LedOn(LED_1);
				}
				else if(distancia>=20 && distancia<=30)
				{
					LedOn(LED_1);
					LedOn(LED_2);
				}
				else
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}
		}
	}
}
}


/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
}
/*==================[end of file]============================================*/