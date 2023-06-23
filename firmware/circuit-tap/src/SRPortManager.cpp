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

#include <Arduino.h>
#include <avr/io.h>

#include "./SRPortManager.h"

// SRPortManager Constructor
SRPortManager::SRPortManager()
{};

uint8_t SRPortManager::_shiftData[SRI_PORT_COUNT] = {0, 0, 0, 0, 0, 0};

/*
 * @brief Initialize all control and data pins
 */
void SRPortManager::begin()
{
    // Set all control pins as outputs
    pinMode(SRI_CEB_PIN, OUTPUT);
    pinMode(SRI_CLK_PIN, OUTPUT);
    pinMode(SRI_LOADB_PIN, OUTPUT);

    // Set clock inhibit (CEB) high to disable shift register
    digitalWrite(SRI_CEB_PIN, HIGH);

    // Set clock (CLK) low
    digitalWrite(SRI_CLK_PIN, LOW);

    // Set load (LOADB) low to load shift register
    digitalWrite(SRI_LOADB_PIN, LOW);

    // Set data port as input using AVR data direction registers
    // Port F is a 6 bit port (PF0-PF5)
    SRI_DATA_PORT.DIRCLR = 0b00111111;
}

/**
 * @brief Shift in all ports
 */
void SRPortManager::shiftInAllPorts()
{
    // Use the native AVR port and pin registers for speed

    // Set clock low
    SRI_CTRL_PORT.OUTCLR = SRI_CLK_bm;

    // Set clock enable (CEB) low to enable shift register
    SRI_CTRL_PORT.OUTCLR = SRI_CEB_bm;

    // Pulse LOADB to load shift register
    SRI_CTRL_PORT.OUTCLR = SRI_LOADB_bm;
    _NOP();
    _NOP();
    SRI_CTRL_PORT.OUTSET = SRI_LOADB_bm;

    // Set clock high
    SRI_CTRL_PORT.OUTSET = SRI_CLK_bm;

    // Shift in 6 bytes of data from Port F
    for (uint8_t i = 0; i < SRI_PORT_COUNT; i++)
    {
        _shiftData[i] = SRI_DATA_PORT.IN;
        SRI_CTRL_PORT.OUTCLR = SRI_CLK_bm;
        _NOP();
        _NOP();
        SRI_CTRL_PORT.OUTSET = SRI_CLK_bm;
    }
}

/**
 * @brief Shift in the specified port
 * @param port Port bit mask
 * @returns uint8_t data
 */
uint8_t SRPortManager::shiftInPort(uint8_t port)
{
    // TODO implement  
};

/**
 * @brief Get the specified byte from the shift data array. NO BOUNDS CHECKING!
*/
uint8_t SRPortManager::getByte(uint8_t index)
{
    return _shiftData[index];
}
