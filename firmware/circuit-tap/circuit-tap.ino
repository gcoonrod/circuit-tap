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
#define F_CPU 20000000UL

#include "./CircuitTap.h"

#define __DEBUG__

// Circuit Tap

// Define pins on the ATMega4809 40-pin DIP package
// UART 1 default pins
#define MCU_TX PIN_PC0
#define MCU_RX PIN_PC1

// State LEDs
#define T_ARM PIN_PC2
#define T_RUN PIN_PC3
#define T_ERR PIN_PC4
#define T_END PIN_PC5

// High Speed IO (HSIO) uses Port D [0..7]
// Use the native AVR port registers for speed
#define HSIO_PORT PORTD
#define HSIO_DDR PORTD.DIR
#define HSIO_PIN PORTD.PIN
#define HSIO_MASK 0b00000000

// Shift Register Inputs (SRI) Control Pins
#define SRI_CEB PIN_PE1   // Active Low
#define SRI_LOADB PIN_PE2 // Active Low
#define SRI_CLK PIN_PE3

// Shift Register Inputs (SRI) Data Pins Port F [0..5]
// Uses the native AVR port registers for speed
// PF6 reserved for RESETB
// All pins are input only
#define SRI_DATA PORTF
#define SRI_DATA_DDR PORTF.DIR
#define SRI_DATA_PIN PORTF.PIN
#define SRI_DATA_MASK 0b0000000

// SPI Pins and Selects
// Primarily intended for SD card use
#define SPI_MOSI PIN_SPI_MOSI
#define SPI_MISO PIN_SPI_MISO
#define SPI_SCK PIN_SPI_SCK
#define SPI_CS PIN_PA3

// Buttons
// All active low
#define BTN_MODE PIN_PA2
#define BTN_ARM PIN_PA1

// Device Under Test (DUT) Clock IO
#define DUT_CLK PIN_PA7

// MCP2221A USB Ready Signal
#define USB_RDY PIN_PE0

// Global Variables/State
volatile uint8_t STARTED = 0;
volatile uint8_t USB_RDY_STATE = 0;
volatile uint8_t CLK_MODE = 0;
volatile uint8_t ARMED = 0;

// Global FSM
StateManager manager;

// LEDs Setup
void setup_leds()
{
    pinMode(T_ARM, OUTPUT);
    pinMode(T_RUN, OUTPUT);
    pinMode(T_ERR, OUTPUT);
    pinMode(T_END, OUTPUT);
}

// Buttons Setup
void setup_btns()
{
    pinMode(BTN_ARM, INPUT);
    pinMode(BTN_MODE, INPUT);
}

// HSIO Port Setup
void setup_hsio_port(uint8_t bitmask)
{
    HSIO_DDR = bitmask;
}

// SR Input Port Setup
void setup_sri_port(uint8_t bitmask)
{
    SRI_DATA_DDR = bitmask;
}

// SR Input Ctrl Setup
void setup_sri_ctrl()
{
    pinMode(SRI_CEB, OUTPUT);
    pinMode(SRI_LOADB, OUTPUT);
    pinMode(SRI_CLK, OUTPUT);
}

// SPI Setup
void setup_spi()
{
    pinMode(SPI_MOSI, OUTPUT);
    pinMode(SPI_MISO, INPUT);
    pinMode(SPI_SCK, OUTPUT);
    pinMode(SPI_CS, OUTPUT);

    // Disable CS by default
    digitalWrite(SPI_CS, HIGH);
}

// USB Ready Setup
void setup_usb_rdy()
{
    pinMode(USB_RDY, INPUT);
}

// DUT Clock Setup
void setup_dut_clk()
{
    pinMode(DUT_CLK, OUTPUT);
}

// Button ISRs
void btn_arm_isr()
{
    Serial1.println("ARM Button Pressed");
    manager.toggleArmState();
}

void btn_mode_isr()
{
    Serial1.println("MODE Button Pressed");
    manager.cycleClkModeState();
}

// Arduino Functions
void setup()
{

    // Setup State Manager
    manager = StateManager();

    // Setup LEDs
    setup_leds();

    // Setup Buttons
    setup_btns();

    // Setup HSIO Port
    setup_hsio_port(HSIO_MASK);

    // Setup SR Input Port
    setup_sri_port(SRI_DATA_MASK);

    // Setup SR Input Ctrl
    setup_sri_ctrl();

    // Setup SPI
    setup_spi();

    // Setup USB Ready
    setup_usb_rdy();

    // Setup DUT Clock
    setup_dut_clk();

    // Attach ISRs
    attachInterrupt(BTN_ARM, btn_arm_isr, FALLING);
    attachInterrupt(BTN_MODE, btn_mode_isr, FALLING);

    Serial1.begin(115200);
    Serial1.println("Circuit Tap - Started");
}

void loop()
{
    // Check system state
    if (STARTED == 0)
    {
        // Check USB Ready
        // USBCFG on MCP2221A doesn't behave as expected
        // Ignore for now
    }

    // Set STARTED
    STARTED = 1;

    // Check ARMED State
    digitalWrite(T_ARM, ARMED);

#ifdef __DEBUG__
    // Print Current State from State Manager to Serial
    if (manager.isDirty()) {
        Serial1.println(manager);
    }
    
#endif

    // Update State Manager
    manager.update();
}