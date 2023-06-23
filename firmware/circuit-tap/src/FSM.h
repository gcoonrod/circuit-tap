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
#ifndef __CircuitTAP_FSM__
#define __CircuitTAP_FSM__

#include <Arduino.h>
class FSM: public Printable
{
public:
    enum class ArmState : uint8_t
    {
        Disarmed = 0,
        Armed = 1
    };

    enum class ClkModeState : uint8_t
    {
        HighZ = 0,
        Input = 1,
        Output = 2
    };

    enum class RunState : uint8_t
    {
        Stopped = 0,
        Running = 1, 
        Ended = 2
    };

    FSM()
    {
        _armState = ArmState::Disarmed;
        _clkModeState = ClkModeState::HighZ;
        _runState = RunState::Stopped;
        _errorState = false;

    }

    void setArmState(ArmState armState)
    {
        _armState = armState;
    }

    void setClkModeState(ClkModeState clkModeState)
    {
        _clkModeState = clkModeState;
    }

    void setRunState(RunState runState)
    {
        _runState = runState;
    }

    void setErrorState(bool errorState)
    {
        _errorState = errorState;
    }

    ArmState getArmState()
    {
        return _armState;
    }

    ClkModeState getClkModeState()
    {
        return _clkModeState;
    }

    RunState getRunState()
    {
        return _runState;
    }

    bool getErrorState()
    {
        return _errorState;
    }

    bool isEqualTo(const FSM &other)
    {
        return (_armState == other._armState) &&
               (_clkModeState == other._clkModeState) &&
               (_runState == other._runState) &&
               (_errorState == other._errorState);
    }

    size_t printTo(Print &p) const
    {
        size_t size = 0;
        size += p.print(F("ArmState: "));
        size += p.print((uint8_t)_armState);
        size += p.print(F(" ClkModeState: "));
        size += p.print((uint8_t)_clkModeState);
        size += p.print(F(" RunState: "));
        size += p.print((uint8_t)_runState);
        return size;
    }


private:
    ArmState _armState;
    ClkModeState _clkModeState;
    RunState _runState;
    bool _errorState;
};

#endif
