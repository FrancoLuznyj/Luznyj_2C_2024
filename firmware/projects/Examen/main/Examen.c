/*! @mainpage Examen
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
 * | 	ECHO    	| 	GPIO_3	    |
 * | 	TRIGGER   	| 	GPIO_2	    |
 * | 	+5V 	 	| 	+5V 		|
 * | 	GND 	 	| 	GND 		|
 * 
 *  * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * |	BUZZER		|	GPIO_11		|
 * |	GND			|	GND
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 04/11/2024 | Document creation		                         |
 *
 * @author Franco Luznyj (francoluznyj1999@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hc_sr04.h"
#include "led.h"
#include "gpio_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/

/** @def CONFIG_BLINK_PERIOD_MEDIR_DISTANCIA
 * @brief Periodo de medicion de distancia
*/
#define CONFIG_BLINK_PERIOD_MEDIR_DISTANCIA 500

/** @def CONFIG_BLINK_PERIOD_SONAR_ALARMA_PRECAUCION
 * @brief Periodo que suena la alarma en modo precaucion
*/
#define CONFIG_BLINK_PERIOD_SONAR_ALARMA_PRECAUCION 2000

/** @def CONFIG_BLINK_PERIOD_SONAR_ALARMA_PELIGRO
 * @brief Periodo que suena la alarma en modo peligro
*/
#define CONFIG_BLINK_PERIOD_SONAR_ALARMA_PELIGRO 1000

/** @def CONFIG_BLINK_PERIOD_SONAR_ALARMA_NOTIFICACIONES
 * @brief Periodo de tiempo que transcurre entre cada envio de notificaciones por UART
*/
#define CONFIG_BLINK_PERIOD_ENVIAR_NOTIFICACIONES 500

/** @def ACELERACION_UMBRAL
 * @brief Valor umbral de aceleracion para establecer si hubo caida o no
*/
#define ACELERACION_UMBRAL 4

/** @def CONFIG_BLINK_PERIOD_MY_TIMER_US
 * @brief Tiempo del timer A en microsegundos
*/
#define CONFIG_BLINK_PERIOD_MY_TIMER_US 10*1000

/*==================[internal data definition]===============================*/

/// @brief Variable de tipo entera de 16 bits que almacena la distancia medida por el HcSr04
uint16_t distancia = 10000;

/// @brief Variable de tipo entera de 16 bits que almacena la suma escalar de las aceleraciones
uint16_t aceleracion = 0;

/// @brief Variable de tipo TaskHandle_t que almacena la referencia de la tarea que mide la aceleracion
TaskHandle_t MedirAceleracionTask_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Tarea que se encarga de medir la distancia en cm con un HcSr04 y de encender los leds en funcion del
 * valor de dicha distancia.
 * 
 * @param pvParameter puntero a un void, parametro no utilizado.
 */
static void MedirDistancia(void *pvParameter)
{
	while(true)
	{
		distancia = HcSr04ReadDistanceInCentimeters();

		if(distancia > 500)
		{	
			LedOn(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}

		else if(distancia <= 500 && distancia >= 300)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOff(LED_3);
		}

		else if(distancia < 300)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
		}

		vTaskDelay(CONFIG_BLINK_PERIOD_MEDIR_DISTANCIA / portTICK_PERIOD_MS);
	}
}

/**
 * @brief Tarea que se encarga de hacer sonar una alarma con distinta frecuencia segun el valor de la distancia
 * 
 * @param pvParameter puntero a un void, parametro no utilizado.
 */
static void SonarAlarma(void *pvParameter)
{
	uint16_t delay = 0;
	while(true)
	{
		if(distancia <= 500 && distancia >= 300)
		{	
			delay = 1000;
			GPIOOn(GPIO_11);
			vTaskDelay(CONFIG_BLINK_PERIOD_SONAR_ALARMA_PRECAUCION - delay / portTICK_PERIOD_MS);
			GPIOOff(GPIO_11);
			
		}

		else if(distancia < 300)
		{	
			delay = 500;
			GPIOOn(GPIO_11);
			vTaskDelay(CONFIG_BLINK_PERIOD_SONAR_ALARMA_PELIGRO - delay / portTICK_PERIOD_MS);
			GPIOOff(GPIO_11);

		}

		vTaskDelay(delay / portTICK_PERIOD_MS);
	}
}

/**
 * @brief Tarea que se encarga de enviar notificaciones por la UART
 * 
 * @param pvParameter puntero a un void, parametro no utilizado.
 */
static void EnviarNotificaciones(void *pvParameter)
{
	while(true)
	{
		if(distancia <= 500 && distancia >= 300)
		{
			UartSendString(UART_CONNECTOR,"Precaucion, vehiculo cerca\r\n");
		}
		else if(distancia < 300)
		{
			UartSendString(UART_CONNECTOR,"Peligro, vehiculo cerca\r\n");
		}

		if(aceleracion > ACELERACION_UMBRAL)
		{
			UartSendString(UART_CONNECTOR,"Caida detectada\r\n");
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_ENVIAR_NOTIFICACIONES / portTICK_PERIOD_MS);
	}
}

/**
 * @brief Tarea que se encarga de medir la aceleracion
 * 
 * @param pvParameter puntero a un void, parametro no utilizado.
 */
static void MedirAceleracion(void *pvParameter)
{	
	uint16_t aceleracion_x = 0, aceleracion_y = 0, aceleracion_z = 0;
	while(true)
	{	
		//Mido la aceleracion en "volts" que me da el acelerometro
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &aceleracion_x);
		AnalogInputReadSingle(CH2, &aceleracion_y);
		AnalogInputReadSingle(CH3, &aceleracion_z);

	    //Paso de voltaje a valor de aceleracion en "G"
		aceleracion_x = (aceleracion_x * 5,5) / 3,3;
		aceleracion_y = (aceleracion_y * 5,5) / 3,3;
		aceleracion_z = (aceleracion_z * 5,5) / 3,3;

		aceleracion = aceleracion_x + aceleracion_y + aceleracion_z;
	}
}

/**
 * @brief Funcion del timer A
 */
void FuncTimerA()
{
	vTaskNotifyGiveFromISR(MedirAceleracionTask_handle, pdFALSE);
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	//Inicializaciones
	HcSr04Init(GPIO_3,GPIO_2);
	LedsInit();
	GPIOInit(GPIO_11,GPIO_OUTPUT);

	//Inicializacion UART
	serial_config_t my_uart = {
		.port = UART_CONNECTOR,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL
	};
	UartInit(&my_uart);

	//Inicializacion analog
	analog_input_config_t config_ADC1 = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0
	};
	AnalogInputInit(&config_ADC1);

	analog_input_config_t config_ADC2 = {
		.input = CH2,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0
	};
	AnalogInputInit(&config_ADC2);

	analog_input_config_t config_ADC3 = {
		.input = CH3,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0
	};
	AnalogInputInit(&config_ADC3);

	//Inicializacion TIMER
	timer_config_t my_timer = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_MY_TIMER_US,
		.func_p = FuncTimerA,
		.param_p = NULL
	};
	TimerInit(&my_timer);

	/* Creacion de tareas */
	xTaskCreate(&MedirDistancia,"MedirDistancia", 2048, NULL, 5, NULL);
	xTaskCreate(&MedirAceleracion,"MedirAceleracion", 2048, NULL, 5, &MedirAceleracionTask_handle);
	xTaskCreate(&SonarAlarma,"SonarAlarma", 2048, NULL, 5, NULL);
	xTaskCreate(&EnviarNotificaciones,"EnviarNotificaciones", 2048, NULL, 5, NULL);
}
/*==================[end of file]============================================*/