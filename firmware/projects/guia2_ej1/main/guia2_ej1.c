/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * En el siguiente ejercicio se desarrolla un firmware para la lectura de distancia con el sensor HC-SR04. La medicion
 * se inicia o se apaga pulsando el switch 1. Un vez medida la distancia, se enciende uno o mas LEDs dependiendo del
 * valor medido y se muestra la medicion en el display LCD. Al presionarse el pulsador switch 2 permite retener el valor
 * medido en el display.
 * El firmware emplea delays de la librería FreeRTOS para delegar el tiempo de uso de cada tarea.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO    	| 	GPIO_3	    |
 * | 	TRIGGER   	| 	GPIO_2	    |
 * | 	+5V 	 	| 	+5V 		|
 * | 	GND 	 	| 	GND 		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 10/09/2024 | Document creation		                         |
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
#include "switch.h"
#include "led.h"
#include "lcditse0803.h"
#include "hc_sr04.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_MEDIR 1000
#define CONFIG_BLINK_PERIOD_TECLAS 200
#define CONFIG_BLINK_PERIOD_MOSTRAR 1000
/*==================[internal data definition]===============================*/

/// @brief Variable global de tipo booleana que se utiliza para establecer si se debe medir o si no se debe medir.
bool encendido = false;

/// @brief Variable global de tipo booleana que se utiliza para establecer si se mantiene la lectura en el display o no.
bool hold = false;

/// @brief Variable global de tipo entera sin signo de 16 bits que se utiliza para almacenar la medicion de la distancia.
uint16_t distancia = 0;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Tarea que lee los interruptores y cambia el estado del LED según el interruptor presionado.
 * 
 * @param pvParameter El parametro para la tarea no es utilizado en este caso.
 * 
 * @return N/A
 */
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

		vTaskDelay(CONFIG_BLINK_PERIOD_TECLAS/portTICK_PERIOD_MS);
	}
}

/**
 * @brief Tarea que se encarga de medir la distancia con el sensor HC-SR04.
 * 
 * La tarea se encarga de medir la distancia en centimetros y guardar el valor
 * en la variable global distancia. La tarea se detiene si la variable encendido
 * es false.
 * 
 * @param pvParameter El parametro para la tarea no es utilizado en este caso.
 * 
 * @return N/A
 */
static void MedirDistanciaTasks(void *pvParameter){

	while(true){
		if(encendido)
		{
			distancia= HcSr04ReadDistanceInCentimeters();
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_MEDIR/portTICK_PERIOD_MS);
	}
}

/**
 * @brief Tarea que se encarga de mostrar la distancia en centimetros en los LEDs y en el display LCD.
 * 
 * La tarea se encarga de mostrar la distancia en centimetros en los LEDs
 * segun la siguiente logica:
 *   - si la distancia es menor a 10, apaga todos los LEDs.
 *   - si la distancia esta entre 10 (inclusive) y 20, enciende el LED_1.
 *   - si la distancia esta entre 20 (inclusive) y 30, enciende el LED_1 y el LED_2.
 *   - si la distancia es mayor o igual a 30, enciende los 3 LEDs.
 * La tarea tambien muestra la distancia en el display LCD, siempre y cuando la variable hold sea false. Si la
 * variable es true, se mantiene el valor de la mediciona previa.
 * La tarea se detiene si la variable encendido es false.
 * 
 * @param pvParameter El parametro para la tarea no es utilizado en este caso.
 * 
 * @return N/A
 */
static void MostrarDistanciaTasks(void *pvParameter){

	while(true)
	{
		if (encendido)
		{
			if (distancia < 10)
			{
				LedsOffAll();
			}
			else if (distancia >= 10 && distancia < 20)
			{
				LedsOffAll();
				LedOn(LED_1);
			}
			else if (distancia >= 20 && distancia < 30)
			{
				LedsOffAll();
				LedOn(LED_1);
				LedOn(LED_2);
			}
			else{
				LedOn(LED_1);
			    LedOn(LED_2);
				LedOn(LED_3);
			}

			if(!hold){
			LcdItsE0803Write(distancia);
			}
		}
		else {
			LedsOffAll();
			LcdItsE0803Off();
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_MOSTRAR / portTICK_PERIOD_MS);
	}
}


/*==================[external functions definition]==========================*/
void app_main(void){
	
//Inicializacion	
	LedsInit();
	SwitchesInit();
	LcdItsE0803Init();
	HcSr04Init(GPIO_3,GPIO_2);
	
//Creacion de tareas
	xTaskCreate(&TeclasTasks, "Teclas", 512, NULL, 5, NULL);
	xTaskCreate(&MedirDistanciaTasks, "MedirDistancia", 512, NULL, 5, NULL);
	xTaskCreate(&MostrarDistanciaTasks, "MostrarDistancia", 512, NULL, 5, NULL);

}
/*==================[end of file]============================================*/