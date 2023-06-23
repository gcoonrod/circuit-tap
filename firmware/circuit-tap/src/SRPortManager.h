// Copyright 2023 Greg Coonrod
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRPORTMANAGER_H
#define SRPORTMANAGER_H

#include <Arduino.h>
#include <avr/io.h>

#define SRI_PORT_COUNT 6

// Shift Register Inputs (SRI) Control Pins
#define SRI_CTRL_PORT PORTE
#define SRI_CEB_PIN PIN_PE1   // Active Low
#define SRI_LOADB_PIN PIN_PE2 // Active Low
#define SRI_CLK_PIN PIN_PE3

#define SRI_CEB_bm PIN1_bm
#define SRI_LOADB_bm PIN2_bm
#define SRI_CLK_bm PIN3_bm

// Shift Register Inputs (SRI) Data Pins Port F [0..5]
// Uses the native AVR port registers for speed
// PF6 reserved for RESETB
// All pins are input only
#define SRI_DATA_PORT PORTF
#define SRI_DATA_DDR PORTF.DIR
#define SRI_DATA_PIN PORTF.PIN
#define SRI_DATA_MASK 0b0000000

#define SRI_PORTA_bm PIN0_bm
#define SRI_PORTB_bm PIN1_bm
#define SRI_PORTC_bm PIN2_bm
#define SRI_PORTD_bm PIN3_bm
#define SRI_PORTE_bm PIN4_bm
#define SRI_PORTF_bm PIN5_bm

/**
 * Shift Register Port Manager Class
 * The SRPortManager is intended to manage the interactions with the shift registers
 * that make up the low-speed input ports A through F of the Circuit Tap. The six shift
 * registers (74HC165) are joined in parallel to form a 48-bit shift register. The control
 * lines for the shift registers are connected to Port E (PE1-PE3) of the ATMega4809. The
 * serial data out (QH) of each shift register is connected to Port F (PF0-PF5) of the
 * ATMega4809.
 */
class SRPortManager
{
public:
    // Constructor
    SRPortManager();

    void begin();

    void shiftInAllPorts();
    
    uint8_t shiftInPort(uint8_t port);

    static uint8_t getByte(uint8_t index);

private:
    static uint8_t _shiftData[SRI_PORT_COUNT];
};

#endif