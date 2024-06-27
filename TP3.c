/*
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    TP3.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL46Z4.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */

/* BIBILIOTECAS EXTRA PARA GENERAR NUMEROS ALEATORIOS */
#include<stdlib.h>
#include<time.h>

/* TODO: insert other definitions and declarations here. */

// Estados de la MEF
typedef enum{OPEN_VALVE, GAS_WARNING, CLOSED_VALVE} ESTADOS;


// Constantes de la implementacion
#define SEG30 18750000		// 30 SEGUNDOS
#define RETARDO 150000;		// retardo para el parpadeo del led con valvula abierta


/*
 * @brief   Application entry point.
 */
int main(void) {

	// !!! COSAS DE LA PLACA !!! NO TOCAR !!!
	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();
#endif
	// !!! COSAS DE LA PLACA !!! NO TOCAR !!!


	// INICIALIZACION DE LOS COMPONENTES A UTILIZAR
	// LED VERDE, PORT D, PIN 5
	SIM->SCGC5 |= 1<<12;	// SETEA CLOCK
	PORTD->PCR[5] |= 1<<8; 	// SETEA MUX
	PTD->PCOR |= 1<<5;		// LIMPIA ORDENES
	PTD->PDDR |= 1<<5;		// LO INCIALIZA COMO SALIDA

	// LED ROJO, PORT E, PIN 29
	SIM->SCGC5 |= 1<<13;	// SETEA CLOCK
	PORTE->PCR[29] |= 1<<8; // SETEA MUX
	PTE->PCOR |= 1<<29;		// LIMPIA ORDENES
	PTE->PDDR |= 1<<29;		// LO INCIALIZA COMO SALIDA

	// SWITCH 1, PORT C, PIN 3
	SIM->SCGC5 |= 1<<11;	// SETEA CLOCK
	PORTC->PCR[3] |= 1<<8; 	// SETEA MUX
	PORTC->PCR[3] |= 3; 	// SETEA PULLS

	// SWITCH 3, PORT C, PIN 12
	SIM->SCGC5 |= 1<<11;	// SETEA CLOCK
	PORTC->PCR[12] |= 1<<8; // SETEA MUX
	PORTC->PCR[12] |= 3; 	// SETEA PULLS

	// INICIALIZACION DE ESTADOS
	ESTADOS estado_actual = OPEN_VALVE;
	ESTADOS proximo_estado = OPEN_VALVE;

	// APAGO LOS LEDS
	PTE->PTOR |= 1<<29;
	PTD->PTOR |= 1<<5;

	// TIEMPOS A UTILIZAR
	uint32_t tiempo, treinta_segundos, tiempo_fuga, titila;
	titila = RETARDO;			// tiempo para el parpadeo
	treinta_segundos = SEG30;	// 30 segundos
	srand(time(NULL));

	while(1) {
		switch(estado_actual){
		// VALVULA ABIERTA --------------------------------------------------
		case OPEN_VALVE:						// Si el sistema funciona con
			PTD->PCOR |= 1<<5;					// normalidad, el led verde
			if(!(PTC->PDIR & (1<<3))){			// permancece prendido.
				PTD->PTOR |= 1<<5;				// Si se detecta una concen-
				PTE->PTOR |= 1<<29;				// tracion peligrosa de gas
				tiempo = 0;						// en el ambiente (sw1 pres-
				titila = RETARDO;				// ionado), se cambia de
				proximo_estado = GAS_WARNING;	// estado.
			}									//
			break;								//
		// ------------------------------------------------------------------
		// CANTIDAD DE GAS EN EL AMBIENTE PELIGROSA ------------------------------
		case GAS_WARNING:								// Mientras la concen-
			tiempo++;									// tracion de gas en
			if(titila > 0){								// el ambiente sea
				titila--;								// peligrosa (sw1 pre-
			}											// sionado), el led
														// verde titila, y se
			if(titila == 0){							// enciende el ex-
				PTD->PTOR |= 1<<5;						// tractor (led rojo).
				titila = RETARDO;						// Si esto persiste por
			}											// mas de 30 segundos,
			if (tiempo > treinta_segundos){				// se cierra la valvula
				PTE->PSOR |= 1<<29;						// de gas, pasando a un
				PTD->PSOR |= 1<<5;
				titila = RETARDO;						// nuevo estado. Sino,
				tiempo_fuga = 625000*(rand() % 31);		// se vuelve a un estado
				proximo_estado = CLOSED_VALVE;			// normalizado.
			}											//
														// Si se cierra la valvula,
			if(PTC->PDIR & (1<<3)){						// se apaga el extractor, y
				PTD->PCOR |= 1<<5;						// se determina un tiempo
				PTE->PSOR |= 1<<29;						// para cual baje la con-
				proximo_estado = OPEN_VALVE;			// centracion del gas.
			}											//
			break;										//
		// ------------------------------------------------------------------------
		// VALVULA CERRADA --------------------------------------------------------
		case CLOSED_VALVE:							// El led verde titila hasta
			if(titila > 0){							// que se llege al tiempo de
				titila--;							// fuga determinado.
			}										//
													// Una vez que la concentracion
			if(tiempo_fuga > 0){					// de gas en el ambiente se haya
				tiempo_fuga--;						// normalizado, se debe abrir
			}										// manualmente la valvula (pre-
													// sionar sw3), cambiando de
			if(tiempo_fuga == 0){					// estado a valvula abierta.
				PTD->PSOR |= 1<<5;					//
				PTE->PSOR |= 1<<29;					//
				if(!(PTC->PDIR & (1<<12))){			//
					proximo_estado = OPEN_VALVE;	//
				}									//
			}										//
													//
			if(titila == 0){						//
				PTD->PTOR |= 1<<5;					//
				PTE->PTOR |= 1<<29;
				titila = RETARDO;					//
			}										//
			break;									//
			// -----------------------------------------------------------------
		}
		// si cambia el estado, cambio estado_actual
		if(estado_actual != proximo_estado){
			estado_actual = proximo_estado;
		}
	}
	return 0 ;
}
