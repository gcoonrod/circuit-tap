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

#ifndef __CircuitTAP_StateManager__
#define __CircuitTAP_StateManager__

#include <Arduino.h>
#include "./FSM.h"

class StateManager : public Printable
{
public:
    StateManager()
    {
        _currentState = FSM();
        _nextState = FSM();
        _isDirty = false;
    };

    StateManager(FSM::ArmState armState, FSM::ClkModeState clkModeState)
    {
        _currentState = FSM();
        _nextState = FSM();
        _isDirty = false;
    };

    void setArmState(FSM::ArmState armState)
    {
        _nextState.setArmState(armState);
        _isDirty = true;
    };

    void setClkModeState(FSM::ClkModeState clkModeState)
    {
        _nextState.setClkModeState(clkModeState);
        _isDirty = true;
    };

    void setRunState(FSM::RunState runState)
    {
        _nextState.setRunState(runState);
        _isDirty = true;
    };

    void setErrorState(bool errorState)
    {
        _nextState.setErrorState(errorState);
        _isDirty = true;
    };

    FSM::ArmState getArmState()
    {
        return _currentState.getArmState();
    };

    FSM::ClkModeState getClkModeState()
    {
        return _currentState.getClkModeState();
    };

    FSM::RunState getRunState()
    {
        return _currentState.getRunState();
    };

    bool getErrorState()
    {
        return _currentState.getErrorState();
    };

    void toggleArmState()
    {
        if (_currentState.getArmState() == FSM::ArmState::Armed)
        {
            _nextState.setArmState(FSM::ArmState::Disarmed);
        }
        else
        {
            _nextState.setArmState(FSM::ArmState::Armed);
        }
        _isDirty = true;
    };

    void cycleClkModeState()
    {
        FSM::ClkModeState clkModeState = _currentState.getClkModeState();
        switch (clkModeState)
        {
        case FSM::ClkModeState::Output:
            _nextState.setClkModeState(FSM::ClkModeState::HighZ);
            break;
        case FSM::ClkModeState::Input:
            _nextState.setClkModeState(FSM::ClkModeState::Output);
            break;
        case FSM::ClkModeState::HighZ:
            _nextState.setClkModeState(FSM::ClkModeState::Input);
            break;
        }
        _isDirty = true;
    };

    void update()
    {
        if (!_currentState.isEqualTo(_nextState))
        {
            _currentState = _nextState;
            _isDirty = false;
        }
    };

    bool isDirty()
    {
        return _isDirty;
    };

    size_t printTo(Print &p) const
    {
        size_t size = 0;
        size += p.print(F("Current State: "));
        size += p.print(_currentState);
        size += p.print(F("\nNext State: "));
        size += p.print(_nextState);
        return size;
    };

private:
    FSM _currentState;
    FSM _nextState;
    bool _isDirty;
};

#endif
