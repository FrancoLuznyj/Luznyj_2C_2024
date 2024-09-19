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
 * | 09/09/2024 | Document creation		                         |
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
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_MEDIR_US 1000*1000
#define CONFIG_BLINK_PERIOD_MOSTRAR_US 1000*1000
/*==================[internal data definition]===============================*/
bool encendido = false;
bool hold = false;
uint16_t distancia = 0;
TaskHandle_t MedirDistanciaTasks_handle = NULL;
TaskHandle_t MostrarDistanciaTasks_handle = NULL;
/*==================[internal functions declaration]=========================*/
void Tecla1(void *pvParameter){		
	encendido = !encendido;
}

void Tecla2(void *pvParameter){
	hold = !hold;
}

void MedirTimer(void *pvParameter){
	vTaskNotifyGiveFromISR(MedirDistanciaTasks_handle, pdFALSE);
}

void MostrarTimer(void *pvParameter){
	vTaskNotifyGiveFromISR(MostrarDistanciaTasks_handle, pdFALSE);
}

static void MedirDistanciaTasks(void *pvParameter){

	while(true){

		if(encendido)
		{
			distancia= HcSr04ReadDistanceInCentimeters();
		}
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

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
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){

		/* Inicializaciones */
	LedsInit();
	SwitchesInit();
	LcdItsE0803Init();
	HcSr04Init(GPIO_3,GPIO_2);

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