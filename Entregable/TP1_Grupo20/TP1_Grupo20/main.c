/*
 * TP1_Grupo20.c
 *
 * Created: 7/4/2026 10:56:59
 * Author : Santiago Robaldi y Lucila Juri
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>      
#include <util/delay.h>

// PROTOTIPOS DE FUNCIONES
void configurar_io();
void leer_pulsadores();
void ejecutar_secuencias_D();
static inline void mandar_cero();
static inline void mandar_uno();
void ejecutar_secuencias_NEO();
static inline void mandar_verde();
static inline void mandar_rojo();
static inline void mandar_azul();
static inline void mandar_vacio();

// VARIABLES PARA SECUENCIA D (LEDs comunes)
uint8_t modo_D = 0;       // 0: Secuencia A, 1: B
int8_t indice_led = 0;    // Cuál LED prender
int8_t direccion = 1;     // Dirección para el rebote

// VARIABLES PARA PULSADORES Y NEOPIXEL
uint8_t estado_ant_PC0 = 1;
uint8_t estado_ant_PC1 = 1; 
uint8_t modo_Neo = 0;       // Memoria para la secuencia del Neopixel
uint8_t color_ant= 0;		// 1 rojo , 0 azul
int8_t pos_verde = 7;


int main(void) {
    configurar_io();
    uint8_t contA = 1;
    uint8_t contB = 1;

    while(1) {
        leer_pulsadores();

        // Puerto D (Cada 100ms)
        if (contA == 10) {
            ejecutar_secuencias_D();
            contA = 1;
        } else {
            contA++;
        }

        // Neopixel (Cada 150ms)
        if (contB == 15) {
			ejecutar_secuencias_NEO();
            contB = 1;
        } else {
            contB++;
        }

        _delay_ms(10); 
    }
}

void configurar_io() {
    // Puerto D: Todo salida
    DDRD = 0xFF;
    
    // Puerto B: PB0 como salida (Neopixel)
    DDRB |= (1 << PB0);

    // Puerto C: PC0 y PC1 como entradas (Pulsadores)
    DDRC &= ~((1 << PC0) | (1 << PC1));
	
    // Activar Pull-ups internos 
    PORTC |= (1 << PC0) | (1 << PC1);
}

void leer_pulsadores() {
    // LÓGICA PARA PC0 ( LEDs del Puerto D)
    uint8_t estado_act_PC0 = (PINC & (1 << PINC0)) ? 1 : 0;

    if (estado_ant_PC0 == 0 && estado_act_PC0 == 1) {
        modo_D = !modo_D; // Cambia el modo
        
        // Si entramos a la Secuencia B, forzamos que retroceda
        if (modo_D == 1) {
            direccion = -1;
        }
    } 
    
    estado_ant_PC0 = estado_act_PC0;
    
    // LÓGICA PARA PC1 (Neopixels)
    uint8_t estado_act_PC1 = (PINC & (1 << PINC1)) ? 1 : 0;
    
    if (estado_ant_PC1 == 0 && estado_act_PC1 == 1) {
        modo_Neo = !modo_Neo; // Cambia el modo del Neopixel
		pos_verde=7; // Fuerza arracar de la derecha
    }
    
    estado_ant_PC1 = estado_act_PC1;
}

void ejecutar_secuencias_D() {
    // Prendemos el LED correspondiente al índice actual
    PORTD = (1 << indice_led);

    // Calculamos cuál será el próximo LED según el modo
    if (modo_D == 0) {
        // SECUENCIA A: LSB (0) hacia MSB (7) repetitivamente
        indice_led++;
        if (indice_led > 7) {
            indice_led = 0; // Vuelve a empezar desde el LSB
        }
    }
    else {
        // SECUENCIA B: Rebote
        indice_led += direccion;
        
        if (indice_led >= 7) {
            direccion = -1; // Chocó arriba, ahora baja
        }
        else if (indice_led <= 0) {
            direccion = 1;  // Chocó abajo, ahora sube
        }
    }
}

static inline void __attribute__((always_inline)) mandar_cero() {
	PORTB |= (1 << PB0);   // Pin en ALTO (2 ciclos gracias al sbi)
	
	// T0H: Mantenemos en ALTO 4 ciclos más (Total 6 ciclos = 375 ns)
	asm volatile(
	"nop\n" "nop\n" "nop\n" "nop\n"
	);
	
	PORTB &= ~(1 << PB0);  // Pin en BAJO (2 ciclos gracias al cbi)
	
	// T0L: Mantenemos en BAJO 12 ciclos más (Total 14 ciclos = 875 ns)
	asm volatile(
	"nop\n" "nop\n" "nop\n" "nop\n"
	"nop\n" "nop\n" "nop\n" "nop\n"
	"nop\n" "nop\n" "nop\n" "nop\n"
	);
}

static inline void __attribute__((always_inline)) mandar_uno() {
	PORTB |= (1 << PB0);   // Pin en ALTO (2 ciclos gracias al sbi)
	
	// T1H: Mantenemos en ALTO 11 ciclos más (Total 13 ciclos = 812 ns)
	asm volatile (
	"nop\n" "nop\n" "nop\n" "nop\n"
	"nop\n" "nop\n" "nop\n" "nop\n"
	"nop\n" "nop\n" "nop\n"
	);
	
	PORTB &= ~(1 << PB0);  // Pin en BAJO (2 ciclos gracias al cbi)
	
	// T1L: Mantenemos en BAJO 5 ciclos más (Total 7 ciclos = 437 ns)
	asm volatile(
	"nop\n" "nop\n" "nop\n" "nop\n"
	"nop\n"
	);
}

void ejecutar_secuencias_NEO(){
	
	if (modo_Neo==0) {
		// Si la secuencia anterior es azul, ahora mandamos rojo
		if (color_ant==0) {
			mandar_vacio(); mandar_rojo();
			mandar_vacio(); mandar_rojo();
			mandar_vacio(); mandar_rojo();
			mandar_vacio(); mandar_rojo();
			color_ant=1;
			// Si la secuencia anterior es rojo, ahora mandamos azul
			} else {
			mandar_azul(); mandar_vacio();
			mandar_azul(); mandar_vacio();
			mandar_azul(); mandar_vacio();
			mandar_azul(); mandar_vacio();
			color_ant=0;
		}
	} else {
		
		// Recorremos los 8 LEDs de la tira (del 0 al 7)
		for (uint8_t i = 0; i < 8; i++) {
			if (i == pos_verde) {
				mandar_verde(); // Si es la posición actual, prende verde
				} else {
				mandar_vacio(); // Si no, lo manda apagado
			}
		}
		
		// Actualizamos la posición para la próxima vuelta (de derecha a izquierda)
		pos_verde--;
		// Si se pasó del primer LED, vuelve a arrancar desde la derecha
		if (pos_verde < 0) {
			pos_verde = 7;
		}	
	}
}

// --- LED ROJO --- (G=0, R=255, B=0)
static inline void mandar_rojo() {
	// VERDE (0)
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	// ROJO (255)
	mandar_uno(); mandar_uno(); mandar_uno(); mandar_uno();
	mandar_uno(); mandar_uno(); mandar_uno(); mandar_uno();
	// AZUL (0)
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
}

// --- LED VERDE --- (G=255, R=0, B=0)
static inline void mandar_verde() {
	// VERDE (255)
	mandar_uno(); mandar_uno(); mandar_uno(); mandar_uno();
	mandar_uno(); mandar_uno(); mandar_uno(); mandar_uno();
	// ROJO (0)
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	// AZUL (0)
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
}

// --- LED AZUL --- (G=0, R=0, B=255)
static inline void mandar_azul() {
	// VERDE (0)
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	// ROJO (0)
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	// AZUL (255)
	mandar_uno(); mandar_uno(); mandar_uno(); mandar_uno();
	mandar_uno(); mandar_uno(); mandar_uno(); mandar_uno();
}

// --- LED VACÍO --- (G=0, R=0, B=0)
static inline void mandar_vacio() {
	// VERDE (0)
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	// ROJO (0)
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	// AZUL (0)
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
	mandar_cero(); mandar_cero(); mandar_cero(); mandar_cero();
}