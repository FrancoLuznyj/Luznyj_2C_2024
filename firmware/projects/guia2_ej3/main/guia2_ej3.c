/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este ejercicio utiliza un sistema de medición de distancia, por ultrasonido (HC-SR04), encendiendo o 
 * apagando LEDs según el rango de distancia y mostrando la distancia en un display LCD. Se utiliza un puerto
 * serie para comunicarse con la UART, para poder observar la medición en un terminal en la PC, siguiendo un 
 * formato específico. Además, la tarea puede ser controlada mediante las teclas 'O' y 'H', la primera activa o
 * desactiva la medicion y la segunda mantiene o no el valor de medicion en el display.
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
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 25/09/2024 | Document creation		                         |
 *
 * @author Franco Luznyj (francoluznyj1999@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "led.h"
#include "switch.h"
#include "led.h"
#include "lcditse0803.h"
#include "hc_sr04.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_MEDIR_US 1000*1000
#define CONFIG_BLINK_PERIOD_MOSTRAR_US 1000*1000

/*==================[internal data definition]===============================*/

/// @brief Variable global de tipo booleana que se utiliza para establecer si se debe medir o si no se debe medir.
bool encendido = false;

/// @brief Variable global de tipo booleana que se utiliza para establecer si se mantiene la lectura en el display o no.
bool hold = false;

/// @brief Variable global de tipo entera sin signo de 16 bits que se utiliza para almacenar la medicion de la distancia.
uint16_t distancia = 0;

/// @brief Variable de tipo TaskHandle_t que almacena la referencia de la tarea que se encarga de medir la distancia.
TaskHandle_t MedirDistanciaTasks_handle = NULL;

/// @brief Variable de tipo TaskHandle_t que almacena la referencia de la tarea que se encarga de mostrar la distancia.
TaskHandle_t MostrarDistanciaTasks_handle = NULL;


/*==================[internal functions declaration]=========================*/

/**
 * @brief Funcion que se llama cuando se presiona el switch 1. Alterna el valor de la variable global encendido.
 * 
 * @param pvParameter Parametro no utilizado.
 * 
 * @return N/A
 */
void Tecla1(void *pvParameter){		
	encendido = !encendido;
}

/**
 * @brief Funcion que se llama cuando se presiona el switch 2. Alterna el valor de la variable global hold.
 * 
 * @param pvParameter Parametro no utilizado.
 * 
 * @return N/A
 */
void Tecla2(void *pvParameter){
	hold = !hold;
}

/**
 * @brief Funcion que se llama cuando finaliza el conteo del timer de medicion de distancia.
 * 
 * Esta funcion se encarga de notificar a la tarea MedirDistanciaTasks 
 * para que vuelva a ejecutarse.
 * 
 * @param pvParameter Parametro no utilizado.
 * 
 * @return N/A
 */
void MedirTimer(void *pvParameter){
	vTaskNotifyGiveFromISR(MedirDistanciaTasks_handle, pdFALSE);
}

/**
 * @brief Funcion que se llama cuando finaliza el conteo del timer de mostrar distancia.
 * 
 * Esta funcion se encarga de notificar a la tarea MostrarDistanciaTasks 
 * para que vuelva a ejecutarse.
 * 
 * @param pvParameter Parametro no utilizado.
 * 
 * @return N/A
 */
void MostrarTimer(void *pvParameter){
	vTaskNotifyGiveFromISR(MostrarDistanciaTasks_handle, pdFALSE);
}

/**
 * @brief Tarea que se encarga de medir la distancia con el sensor HC-SR04
 * 
 * La tarea se encarga de medir la distancia en centimetros y guardar el valor
 * en la variable global distancia. La tarea se detiene si la variable encendido
 * es false.
 * 
 * @param pvParameter El parametro para la tarea no es utilizado en este caso
 * 
 * @return N/A
 */
static void MedirDistanciaTasks(void *pvParameter){

	while(true){

		if(encendido)
		{
			distancia= HcSr04ReadDistanceInCentimeters();
		}
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/**
 * @brief Tarea que se encarga de mostrar la distancia en centimetros en los LEDs
 *       y en el display LCD.
 * 
 * La tarea se encarga de mostrar la distancia en centimetros en los LEDs
 * segun la siguiente logica:
 *   - si la distancia es menor a 10, apaga todos los LEDs.
 *   - si la distancia esta entre 10 (inclusive) y 20, enciende el LED_1.
 *   - si la distancia esta entre 20 (inclusive) y 30, enciende el LED_1 y el LED_2.
 *   - si la distancia es mayor o igual a 30, enciende los 3 LEDs.
 * La tarea tambien muestra la distancia en el display LCD, siempre y cuando
 * la variable hold sea false, si es true se mantiene el valor de la mediciona previa.
 * La tarea se detiene si la variable encendido es false.
 * 
 * @param pvParameter El parametro para la tarea no es utilizado en este caso
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
			
			UartSendString(UART_PC,(char*) UartItoa(distancia, 10)); //aca tengo que mandar el valor de distancia desde la uart a la pc
			UartSendString(UART_PC, " cm\r\n");
		}
		else {
			LedsOffAll();
			LcdItsE0803Off();
		}
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/**
 * @brief Funcion que se encarga de leer desde la uart y modificar las variables encendido y hold segun sea
 * necesario. La funcion se encarga de leer desde la uart y dependiendo del caracter recibido, modifica las
 * variables encendido y hold.
 *
 * @param pvParameter Parametro no utilizado.
 *
 * @return N/A
 */
void funcionUartLectura(void *pvParameter){
	
	uint8_t data;
	UartReadByte(UART_PC, &data);

	switch (data){
		case 'O':
			encendido = !encendido;
			break;
		case 'H':
			hold = !hold;
			break;
		default:
			break;
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){

		/* Inicializaciones */
	LedsInit();
	SwitchesInit();
	LcdItsE0803Init();
	HcSr04Init(GPIO_3,GPIO_2);
	
	serial_config_t uart_leer = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = funcionUartLectura,
		.param_p = NULL
	};
	UartInit(&uart_leer);

		/* Interrupciones SWITCHES */
	SwitchActivInt(SWITCH_1,Tecla1,NULL);
	SwitchActivInt(SWITCH_2,Tecla2,NULL);

	    /* Inicialización de timers */ 
	timer_config_t timer_medir = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_MEDIR_US,
        .func_p = MedirTimer,
        .param_p = NULL
    };
	TimerInit(&timer_medir);

    timer_config_t timer_mostrar = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_MOSTRAR_US,
        .func_p = MostrarTimer,
        .param_p = NULL
    };
	TimerInit(&timer_mostrar);

		/* Creacion de tareas */
	xTaskCreate(&MedirDistanciaTasks, "MedirDistancia", 512, NULL, 5, &MedirDistanciaTasks_handle);
	xTaskCreate(&MostrarDistanciaTasks, "MostrarDistancia", 512, NULL, 5, &MostrarDistanciaTasks_handle);

	    /* Inicialización del conteo de timers */
    TimerStart(timer_medir.timer);
    TimerStart(timer_mostrar.timer);

}
/*==================[end of file]============================================*/