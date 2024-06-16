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
#include<stdlib.h>
#include<time.h>
/* TODO: insert other definitions and declarations here. */
// enums de ESTADOS
typedef enum{NORMALIZADO, FUGA, VALVULA_CERRADA}ESTADOS;
#define SEG30 30000000;
#define RETARDO 150000;
/*
 * @brief   Application entry point.
 */
int main(void) {

	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();
#endif

	// INICIALIZACION DE LOS COMPONENTES A UTILIZAR
	// LED VERDE, PORT D, PIN 5
	SIM->SCGC5 |= 1<<12;	// SETEA CLOCK
	PORTD->PCR[5] |= 1<<8; 	// SETEA MUX
	PTD->PCOR |= 1<<5;		// LIMPIA ORDENES
	PTD->PDDR |= 1<<5;		// LO INCIALIZA COMO SALIDA

	// LED ROJO, PORT E, PIN 29
	SIM->SCGC5 |= 1<<13;	// SETEA CLOCK
	PORTE->PCR[29] |= 1<<8; 	// SETEA MUX
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
	ESTADOS estado_actual = NORMALIZADO;
	ESTADOS proximo_estado = NORMALIZADO;

	// APAGO LOS LEDS
	PTE->PTOR |= 1<<29;
	PTD->PTOR |= 1<<5;

	// TEMPORIZADOR ALEATORIO
	uint32_t tiempo, treinta_segundos;
	treinta_segundos = SEG30;

	while(1) {
		switch(estado_actual){
		case NORMALIZADO:
			if(!(PTC->PDIR & (1<<3))){		// si esta presionado el sw1
				PTD->PTOR |= 1<<5;			// prendo el led verde
				tiempo = 0;
				proximo_estado = FUGA;
			}
			break;
		case FUGA:
			tiempo++;

			if (tiempo > treinta_segundos){
				PTE->PTOR |= 1<<29;
				tiempo = RETARDO;
				proximo_estado = VALVULA_CERRADA;
			}

			if(PTC->PDIR & (1<<3)){			// si se suelta el sw1
				PTD->PTOR |= 1<<5;			// apago el led verde
				proximo_estado = NORMALIZADO;
			}
			break;
		case VALVULA_CERRADA:
			if(tiempo > 0){
				tiempo--;
			}

			if(tiempo == 0){
				PTE->PTOR |= 1<<29;
				tiempo = RETARDO;
			}

			if(!(PTC->PDIR & (1<<12))){
				PTD->PSOR |= 1<<5;
				PTE->PSOR |= 1<<29;
				proximo_estado = NORMALIZADO;
			}

		}
		// si cambia el estado, cambio estado_actual
		if(estado_actual != proximo_estado){
			estado_actual = proximo_estado;
		}

	}
	return 0 ;
}
