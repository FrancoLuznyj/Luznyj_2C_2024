/*! @mainpage guia2_ej4
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
* |    Peripheral  |     ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN1-POT 	|	   CH0		|
 * | 	PIN2-POT 	|	   CH1		|
 * | 	PIN3-POT 	|	   GND		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 07/10/2024 | Document creation		                         |
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
#include "uart_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/

/** @def CONFIG_BLINK_PERIOD_ADC_US
 * @brief Tiempo del timer ADC
*/
#define CONFIG_BLINK_PERIOD_ADC_US 2000

/** @def CONFIG_BLINK_PERIOD_DAC_US
 * @brief Tiempo del timer DAC
*/
#define CONFIG_BLINK_PERIOD_DAC_US 4000

/** @def BASE_DECIMAL
 * @brief Indica la base decimal para la conversion
*/
#define BASE_DECIMAL 10

/** @def BUFFER_SIZE
 * @brief Indica el tamaño del buffer del vector ecg
*/
#define BUFFER_SIZE  231

/*==================[internal data definition]===============================*/

/*volatile se usa para que el compilador no optimice la vairable*/

/// @brief Variable de tipo entera de 16 bits volatil que almacena los valores del ECG
volatile uint16_t valores = 5;

/// @brief Variable de tipo entera de 16 bits volatil que se usa para recorrer el vector ecg
volatile uint8_t contador = 0;

/// @brief Variable de tipo TaskHandle_t que almacena la referencia de la tarea principal del sistema
TaskHandle_t main_task_handle = NULL;

/// @brief Variable de tipo TaskHandle_t que almacena la referencia de la tarea que se encarga de la conversion ADC
TaskHandle_t ADC_Task_handle = NULL;

/// @brief Variable de tipo TaskHandle_t que almacena la referencia de la tarea que se encarga de la conversion DAC
TaskHandle_t DAC_Task_handle = NULL;

/// @brief Variable de tipo TaskHandle_t que almacena la referencia de la tarea que se encarga de enviar datos por la UART
TaskHandle_t EnviarDatosTask_handle = NULL;

/** @const ecg[]
 *  @brief Vector de tipo const char que simula un ecg.
*/
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/*==================[internal functions declaration]=========================*/


/**
 * @brief Funcion que se ejecuta al finalizar el conteo del timer ADC
 * 
 * Esta funcion se encarga de notificar a las tareas ADC y EnviarDatos que 
 * el timer ha finalizado su conteo y que ya pueden ejecutar sus tareas.
 * 
 * @param pvParameter puntero a un void, parametro no utilizado.
 */
void funcTimerADC(void *pvParameter)
{
		vTaskNotifyGiveFromISR(ADC_Task_handle, NULL);
		vTaskNotifyGiveFromISR(EnviarDatosTask_handle, NULL);
};

/**
 * @brief Funcion que se ejecuta al finalizar el conteo del timer DAC
 * 
 * Esta funcion se encarga de notificar a la tarea DAC que el timer ha
 * finalizado su conteo y que ya puede ejecutar su tarea.
 * 
 * @param pvParameter puntero a un void, parametro no utilizado.
 */
void funcTimerDAC(void *pvParameter)
{
		vTaskNotifyGiveFromISR(DAC_Task_handle, NULL);
}


/**
 * @brief Tarea que se encarga de convertir de digital a analogicos los valores del ECG
 * 
 * Esta tarea se encarga de convertir los valores del ECG de digital a analogicos, 
 * utilizando la funcion AnalogOutputWrite. La tarea se ejecuta indefinidamente y
 * utiliza la funcion ulTaskNotifyTake para esperar a que el timer DAC termine su
 * conteo. El contador se incrementa en 1 en cada iteracion, y cuando alcanza el
 * valor de BUFFER_SIZE-1, se reinicia a 0.
 * 
 * @param pvParameter puntero a un void, parametro no utilizado.
 */
static void DAC_Task(void *pvParameter)
{
		while (true)
		{
			AnalogOutputWrite(ecg[contador]);
			
			if(contador < BUFFER_SIZE-1)
			{
				contador++;
			}
			else
			{
				contador = 0;
			}	
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		}
}

/**
 * @brief Tarea que se encarga de convertir el valor analgico del canal CH1 a un valor digital
 * 
 * Esta tarea se encarga de leer el valor analogico del canal CH1, 
 * utilizando la funcion AnalogInputReadSingle, convirtiendolo a un valor digital. La tarea se ejecuta
 * indefinidamente y utiliza la funcion ulTaskNotifyTake para esperar a que el timer ADC termine
 * su conteo.
 * @param pvParameter puntero a un void, parametro no utilizado.
 */
static void ADC_Task(void *pvParameter)
{
		while (true)
		{
			AnalogInputReadSingle(CH1, &valores);
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		}
}

/**
 * @brief Tarea que se encarga de enviar por UART el valor digital leido
 * 
 * Esta tarea se encarga de enviar por UART el valor digital leido, utilizando la funcion UartSendString.
 * La tarea se ejecuta indefinidamente y utiliza la funcion ulTaskNotifyTake para esperar a que el timer ADC 
 * termine su conteo.
 * @param pvParameter puntero a un void, parametro no utilizado.
 */
static void EnviarDatosTask(void *pvParameter)
{
		while (true)
		{
			UartSendString(UART_PC, (char*) UartItoa(valores, BASE_DECIMAL));
			UartSendString(UART_PC, " \r");
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		}
}

/*==================[external functions definition]==========================*/
void app_main(void){

 	/* Inicialización de timers */ 
	timer_config_t timer_ADC = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_ADC_US,
        .func_p = funcTimerADC,
        .param_p = NULL
    };
	TimerInit(&timer_ADC);

	timer_config_t timer_DAC = {
		.timer = TIMER_B,
		.period = CONFIG_BLINK_PERIOD_DAC_US,
		.func_p = funcTimerDAC,
		.param_p = NULL
	};
	TimerInit(&timer_DAC);

	/* Inicializacion de ADC */
	analog_input_config_t config_ADC = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0
	};
	AnalogInputInit(&config_ADC);
	AnalogOutputInit();

	/* Inicializacion de UART */
	serial_config_t uart_enviar = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL
	};
	UartInit(&uart_enviar);

	/* Creacion de tareas */
	xTaskCreate(&DAC_Task,"DAC_Task", 2048, NULL, 5, &DAC_Task_handle);
	xTaskCreate(&ADC_Task,"ADC_Task", 2048, NULL, 4, &ADC_Task_handle);
	xTaskCreate(&EnviarDatosTask,"EnviarDataTask", 2048, NULL, 4, &EnviarDatosTask_handle);

	TimerStart(timer_ADC.timer);
	TimerStart(timer_DAC.timer);

}
/*==================[end of file]============================================*/