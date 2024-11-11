/*! @mainpage recuperatorio
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
 * | PIN+ GALGA_1	| 	GPIO_9 		|
 * | 	GND 	 	| 	GND 		|
 * | PIN+ GALGA_2 	| 	GPIO_10		|
 * | 	GND 	 	| 	GND 		|
 * | PIN+ BARRERA 	| 	GPIO_11 	|
 * | 	GND 	 	| 	GND 		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/11/2024 | Document creation		                         |
 *
 * @author Franco Lunzyj
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hc_sr04.h"
#include "led.h"
#include "analog_io_mcu.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

#define BUFFER_SIZE_DISTANCIAS 10

#define CONFIG_BLINK_PERIOD_MEDIR_DISTANCIA_MS 100

#define BUFFER_SIZE_PESOS 50

#define DELTA_T 0.1

#define CONFIG_BLINK_PERIOD_PESAR_VEHICULO_US 5000

#define CONFIG_BLINK_PERIOD_SEND_DATA_MS 250

#define BASE_DECIMAL 10

/*==================[internal data definition]===============================*/

uint16_t vector_distancias[BUFFER_SIZE_DISTANCIAS] = {0,0,0,0,0,0,0,0,0,0};

uint16_t vector_pesos_galga1[BUFFER_SIZE_PESOS];

uint16_t vector_pesos_galga2[BUFFER_SIZE_PESOS];

uint16_t velocidad = 0;

uint16_t velocidad_maxima = 0;

uint16_t peso_vehiculo = 0;

TaskHandle_t PesarVehiculo_handle = NULL;

/*==================[internal functions declaration]=========================*/

//Cada 100ms tengo que medir distancia

static void medirDistancia(void *pvParameter)
{
	uint16_t numero_muestra=0;
	uint16_t distancia_nueva=0;

	while(true)
	{	
		distancia_nueva = HcSr04ReadDistanceInCentimeters();

			for(uint8_t i=1;i<BUFFER_SIZE_DISTANCIAS;i++)
			{
				vector_distancias[i-1] = vector_distancias[i];
			}
			vector_distancias[BUFFER_SIZE_DISTANCIAS] = distancia_nueva;

		if(distancia_nueva < 1000)
		{	
			velocidad = ((vector_distancias[BUFFER_SIZE_DISTANCIAS] - vector_distancias[BUFFER_SIZE_DISTANCIAS-1]) / 100) / DELTA_T;

			if(velocidad_maxima < velocidad)
			{
				velocidad_maxima = velocidad;
			}

			if(velocidad >= 8)
			{
				LedOn(LED_3);
				LedOff(LED_2);
				LedOff(LED_1);
			}
			else if(velocidad > 0 && velocidad < 8)
			{
				LedOn(LED_2);
				LedOff(LED_3);
				LedOff(LED_1);
			}
			else
			{
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
		}

		vTaskDelay(CONFIG_BLINK_PERIOD_MEDIR_DISTANCIA_MS / portTICK_PERIOD_MS);
	}
}

static void PesarVehiculo(void *pvParameter)
{	
	uint16_t galga_1 = 0;
	uint16_t galga_2 = 0;
	uint16_t mediciones_para_promedio = 0;
	uint16_t promedio_galga_1 = 0;
	uint16_t promedio_galga_2 = 0;

	while(true)
	{
		AnalogInputReadSingle(CH1, &galga_1);
		AnalogInputReadSingle(CH2, &galga_2);

		galga_1 = (0 + ( ( (20000-0) / (3,3 - 0) ) * (galga_1 - 0) ));
		galga_2 = (0 + ( ( (20000-0) / (3,3 - 0) ) * (galga_2 - 0) ));

		vector_pesos_galga1[mediciones_para_promedio] = galga_1;
		vector_pesos_galga2[mediciones_para_promedio] = galga_2;

		mediciones_para_promedio++;

		if(mediciones_para_promedio == BUFFER_SIZE_PESOS-1)
		{	
			promedio_galga_1 = vector_pesos_galga1[0];
			promedio_galga_2 = vector_pesos_galga2[0];
			
			for(uint8_t i=0;i<BUFFER_SIZE_PESOS;i++)
			{
				promedio_galga_1 = promedio_galga_1 + vector_pesos_galga1[i];
				promedio_galga_2 = promedio_galga_2 + vector_pesos_galga2[i];
			}

			promedio_galga_1 = promedio_galga_1 / BUFFER_SIZE_PESOS;
			promedio_galga_2 = promedio_galga_2 / BUFFER_SIZE_PESOS;

			peso_vehiculo = promedio_galga_1 + promedio_galga_2;
		}

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void FuncionTimerA()
{
	vTaskNotifyGiveFromISR(PesarVehiculo_handle, pdFALSE);
}

static void SendData(void *pvParameter)
{
	while(true)
	{	
		UartSendString(UART_PC, "Peso:");
		UartSendString(UART_PC, (char*) UartItoa(peso_vehiculo, BASE_DECIMAL));
		UartSendString(UART_PC, "\r\n");
		UartSendString(UART_PC, "Velocidad Maxima:");
		UartSendString(UART_PC, (char*) UartItoa(velocidad_maxima, BASE_DECIMAL));
		UartSendString(UART_PC, "\r\n");

		vTaskDelay(CONFIG_BLINK_PERIOD_SEND_DATA_MS/ portTICK_PERIOD_MS);
	}
}

void AbrirCerrarBarrera()
{
	uint8_t data;
	UartReadByte(UART_PC, &data);

	switch (data){
		case 'o':
			GPIOOn(GPIO_11);
			break;
		case 'c':
			GPIOOff(GPIO_11);
			break;
		default:
			break;
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
//Inicializacion HC_SR04 , GPIOs y Leds

	HcSr04Init(GPIO_3,GPIO_2);
	GPIOInit(GPIO_11,GPIO_OUTPUT);
	LedsInit();

	//Inicializacion TIMER
	timer_config_t timer_PesarVehiculo = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_PESAR_VEHICULO_US,
		.func_p = FuncionTimerA,
		.param_p = NULL	
};
TimerInit(&timer_PesarVehiculo);
TimerStart(timer_PesarVehiculo.timer);

//Inicializacion UART
serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = AbrirCerrarBarrera,
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

}
/*==================[end of file]============================================*/