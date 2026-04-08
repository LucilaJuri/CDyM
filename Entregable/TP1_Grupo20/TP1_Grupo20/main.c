/*
 * TP1_Grupo20.c
 *
 * Created: 7/4/2026 10:56:59
 * Author : santi
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL // Definir la velocidad (16MHz para el 328P en Proteus)
#endif

#include <avr/io.h>      // Manejo de registros (DDR, PORT, PIN)
#include <util/delay.h>   // Funciones de retardo para el loop de 50ms
//#include "light_ws2812.h" // La biblioteca específica para los Neopixels

void configurar_io();
void leer_pulsadores();
void ejecutar_secuencias_D();

// Variables para Secuencia D
uint8_t modo_D = 0;       // 0: Secuencia A, 1: B
int8_t indice_led = 0;    // Cuál LED prender
int8_t direccion = 1;     // Dirección para el rebote

// Variables para Pulsadores (Memoria)
uint8_t estado_ant_PC0 = 1;
uint8_t estado_ant_PC1 = 1; // Ya la dejamos lista para los Neopixels
uint8_t modo_Neo = 0;       // Memoria para la otra secuencia

int main(void) {
	configurar_io();
	uint8_t contA = 1;
	uint8_t contB = 1;

	while(1) {
		leer_pulsadores();

		// Tarea Puerto D (Cada 100ms)
		if (contA == 10) {
			ejecutar_secuencias_D();
			contA = 1;
			} else {
			contA++;
		}

		// Tarea Neopixel (Cada 150ms)
		if (contB == 15) {
			//ejecutar_secuencia_Neo(modo_Neopixel);
			contB = 1;
			} else {
			contB++;
		}

		_delay_ms(10); // Base de tiempo del sistema
	}
}

void configurar_io() {
	// Puerto D: Todo salida (8 LEDs)
	DDRD = 0xFF;
	
	// Puerto B: PB0 como salida (Neopixel)
	DDRB |= (1 << PB0);

	// Puerto C: PC0 y PC1 como entradas (0)
	DDRC &= ~((1 << PC0) | (1 << PC1));
	// Activar Pull-ups internos (Escribir 1 en el PORT siendo entrada)
	PORTC |= (1 << PC0) | (1 << PC1);
}


void leer_pulsadores() {
	uint8_t estado_act_PC0 = (PINC & (1 << PINC0)) ? 1 : 0;

	if (estado_ant_PC0 == 0 && estado_act_PC0 == 1) {
		modo_D = !modo_D; // Cambia el modo
		
		// Si entramos a la Secuencia B, forzamos que retroceda
		// desde el LED actual para que se note el cambio al instante.
		if (modo_D == 1) {
			direccion = -1;
		}
		// ¡Listo! Al no tocar indice_led, el programa sigue desde donde estaba.
	}
	estado_ant_PC0 = estado_act_PC0;
	
	// --- LÓGICA PARA PC1 (Neopixels) ---
	uint8_t estado_act_PC1 = (PINC & (1 << PINC1)) ? 1 : 0;
	
	if (estado_ant_PC1 == 0 && estado_act_PC1 == 1) {
		modo_Neo = !modo_Neo;
	}
	estado_ant_PC1 = estado_act_PC1;
}

void ejecutar_secuencias_D() {
	// 1. Prendemos el LED correspondiente al índice actual
	PORTD = (1 << indice_led);

	// 2. Calculamos cuál será el próximo LED según el modo
	if (modo_D == 0) {
		// SECUENCIA A: LSB (0) hacia MSB (7) repetitivamente
		indice_led++;
		if (indice_led > 7) {
			indice_led = 0; // Vuelve a empezar desde el LSB
		}
	}
	else {
		// SECUENCIA B: Rebote (arranca en MSB por el enunciado,
		// pero la lógica de rebote se maneja con la dirección)
		indice_led += direccion;
		
		if (indice_led >= 7) {
			direccion = -1; // Chocó arriba, ahora baja
		}
		else if (indice_led <= 0) {
			direccion = 1;  // Chocó abajo, ahora sube
		}
	}
}