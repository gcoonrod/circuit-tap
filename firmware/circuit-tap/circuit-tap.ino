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

#define F_CPU 20000000UL
#define __DEBUG__

// Library and System Includes
#include <Arduino.h>
#include <avr/io.h>
#include <SPI.h>
#include <SD.h>

// Local Includes
#include "src/CircuitTap.h"

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


// Global FSM
static StateManager manager;
SRPortManager srPort;

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
    pinMode(DUT_CLK, INPUT);
}

// Setup SD Card
void setup_sd()
{
    // Setup SPI
    setup_spi();

    // Setup SD Card
    if (!SD.begin(SPI_CS))
    {
        Serial1.println("SD Card failed to initialize");
        manager.setErrorState(true);
    }
    Serial1.println("SD Card initialized");
}

// Button ISRs
void btn_arm_isr()
{
    Serial1.println("ARM Button Pressed");
    if (manager.getArmState() == FSM::ArmState::Disarmed)
    {
        // If Disarmed and Running, End
        if (manager.getRunState() == FSM::RunState::Running)
        {
            manager.setRunState(FSM::RunState::Ended);
        }
        else if (manager.getRunState() == FSM::RunState::Ended)
        {
            manager.setRunState(FSM::RunState::Stopped);
        }
        else
        {
            manager.setArmState(FSM::ArmState::Armed);
        }
    }
    else
    {
        manager.setArmState(FSM::ArmState::Disarmed);
    }
}

void btn_mode_isr()
{
    Serial1.println("MODE Button Pressed");
    manager.cycleClkModeState();
}

// DUT Clock ISR
void dut_clk_isr()
{

    // If armed, start running and disarm
    if (manager.getArmState() == FSM::ArmState::Armed)
    {
        Serial1.println(F("Run started by DUT Clock Interrupt"));
        manager.setRunState(FSM::RunState::Running);
        manager.setArmState(FSM::ArmState::Disarmed);
    }

    // If running, trigger SR latch and shift in data
    if (manager.getRunState() == FSM::RunState::Running)
    {
        Serial1.println(F("Placeholder for SR Latch Trigger"));
        // TODO Trigger SR Latch and Shift in Data
        // TODO if ISR is triggered before Shift in is complete set Error state and end run
    }

    // If ended or stopped do nothing
    if (manager.getRunState() == FSM::RunState::Ended || manager.getRunState() == FSM::RunState::Stopped)
    {
#ifdef __DEBUG__
        Serial1.println(F("DUT Clock ISR while Stopped or Ended"));
#endif
        return;
    }
}

// LED Update
void updateLEDs(StateManager *manager)
{
    // Update ARM LED
    digitalWrite(T_ARM, manager->getArmState() == FSM::ArmState::Armed ? 1 : 0);

    // Update RUN and END LEDs
    switch (manager->getRunState())
    {
    case FSM::RunState::Running:
        digitalWrite(T_RUN, 1);
        digitalWrite(T_END, 0);
        break;
    case FSM::RunState::Stopped:
        digitalWrite(T_RUN, 0);
        digitalWrite(T_END, 0);
        break;
    case FSM::RunState::Ended:
        digitalWrite(T_RUN, 0);
        digitalWrite(T_END, 1);
        break;
    }

    // Update ERR LED
    digitalWrite(T_ERR, manager->getErrorState() ? 1 : 0);
}

// DUT Clock Update
void updateDUTClk(StateManager *manager)
{
    // Do nothing if RunState is Running
    if (manager->getRunState() == FSM::RunState::Running)
    {
        // A test is running, do not make any changes to
        return;
    }

    // Update DUT Clock
    switch (manager->getClkModeState())
    {
        // Output Mode, Disable DUT Clock Interrupt
    case FSM::ClkModeState::Output:
        detachInterrupt(DUT_CLK);
        pinMode(DUT_CLK, OUTPUT);
        break;
        // Input Mode, Enable DUT Clock Interrupt if Armed
    case FSM::ClkModeState::Input:
        if (manager->getArmState() == FSM::ArmState::Armed)
        {
            attachInterrupt(DUT_CLK, dut_clk_isr, FALLING);
        }
        else
        {
            detachInterrupt(DUT_CLK);
        }
        pinMode(DUT_CLK, INPUT_PULLUP);
        break;
        // HighZ Mode, Disable DUT Clock Interrupt
    case FSM::ClkModeState::HighZ:
        detachInterrupt(DUT_CLK);
        pinMode(DUT_CLK, INPUT);
        break;
    }
}

// Arduino Functions
void setup()
{

    // Setup State Manager
    manager = StateManager();

    // Setup SR Port Manager
    srPort = SRPortManager();
    srPort.begin();

    // Setup LEDs
    setup_leds();

    // Setup Buttons
    setup_btns();

    // Setup HSIO Port
    setup_hsio_port(HSIO_MASK);

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
    // Update LEDs based on current state
    updateLEDs(&manager);

    // Update DUT clock based on current state
    updateDUTClk(&manager);

#ifdef __DEBUG__
    // Print Current State from State Manager to Serial
    if (manager.isDirty())
    {
        Serial1.println(manager);
    }
#endif

    // Update State Manager
    manager.update();
}