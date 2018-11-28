#include "mill.h"

uint8_t inputs[8];
uint8_t outputs[4];
uint8_t memo[30];
uint32_t timers[10];
uint8_t timersEnabled[10];

bool UpdateTimer(
    uint8_t phase,
	uint32_t now,
	uint8_t timerId,
	uint16_t time_ms,
	uint8_t* inputB, uint8_t inputMask,
	uint8_t* outputB, uint8_t outputMask)
{
    switch (phase)
    {
    case 0:
        if (timersEnabled[timerId] == 1)
        {
            uint32_t diff = now - timers[timerId];
            if (diff >= time_ms)
            {
                timersEnabled[timerId] = 2;
                uint8_t prev = *outputB;
                *outputB = prev | outputMask;
                if (prev != *outputB)
                    return true;
            }
        }
        break;
    case 1:
        if (*inputB & inputMask)
        {
            if (timersEnabled[timerId] == 0)
            {
                timersEnabled[timerId] = 1;
                timers[timerId] = now;
            }
        }
        else
        {
            timersEnabled[timerId] = 0;

            uint8_t prev = *outputB;
            *outputB = prev & ~outputMask;
            if (prev != *outputB)
                return true;
        }
        break;
    }

    return false;
}

bool UpdateTimers(uint8_t phase)
{
    bool relaxed = true;
    uint32_t now = millis();
    if (UpdateTimer(phase, now, 0, 2000, &memo[23], 4, &memo[23], 128))
        relaxed = false;
    if (UpdateTimer(phase, now, 1, 2500, &memo[26], 4, &memo[27], 128))
        relaxed = false;
    if (UpdateTimer(phase, now, 2, 3000, &memo[28], 16, &memo[28], 64))
        relaxed = false;
    if (UpdateTimer(phase, now, 3, 800, &memo[22], 8, &memo[28], 32))
        relaxed = false;
    if (UpdateTimer(phase, now, 5, 400, &memo[26], 64, &memo[26], 128))
        relaxed = false;
    if (UpdateTimer(phase, now, 6, 15000, &memo[29], 2, &memo[5], 128))
        relaxed = false;
    if (UpdateTimer(phase, now, 7, 500, &memo[21], 128, &memo[20], 128))
        relaxed = false;
    if (UpdateTimer(phase, now, 8, 1000, &memo[15], 128, &memo[8], 128))
        relaxed = false;
    if (UpdateTimer(phase, now, 9, 3000, &memo[13], 128, &memo[28], 128))
        relaxed = false;
    return !relaxed;
}

bool Update()
{
    uint8_t local0, local1, local2, local3, local4, local5, local6, local7, local8;
    bool val;
    bool relaxed = true;
    uint8_t prev;
    if (UpdateTimers(0))
        relaxed = false;

// Eq. 1
    local0 = inputs[1];
    local1 = memo[1];
    local2 = memo[18];
    local3 = memo[0];
    local4 = inputs[0];
    local5 = memo[8];
    val = (((((((((!(local0 & _BV(0)) && !(local0 & _BV(1))) || (local1 & _BV(3))) && !(local2 & _BV(4))) && !(local3 & _BV(7))) && (local4 & _BV(0))) && ((!(local0 & _BV(0)) && !(local0 & _BV(1))) || (local1 & _BV(7)))) && ((local0 & _BV(2)) || (local5 & _BV(5)) || (local3 & _BV(0)))) && (local4 & _BV(1))) && ((local4 & _BV(2)) || (local4 & _BV(3))));
    prev = memo[0];
    val ? memo[0] = prev | _BV(0) : memo[0] = prev & ~_BV(0);
    if (prev != memo[0]) {
        relaxed = false;
    }

// Eq. 2
    local0 = inputs[1];
    local1 = memo[1];
    local2 = memo[18];
    local3 = inputs[0];
    local4 = memo[0];
    val = (((((((((!(local0 & _BV(2)) && !(local0 & _BV(1))) || (local1 & _BV(3))) && !(local2 & _BV(4))) && !(local2 & _BV(5))) && (local3 & _BV(0))) && ((!(local0 & _BV(2)) && !(local0 & _BV(1))) || (local1 & _BV(7)))) && ((local0 & _BV(0)) || (local1 & _BV(4)) || (local4 & _BV(1)))) && (local3 & _BV(4))) && ((local3 & _BV(2)) || (local3 & _BV(3))));
    prev = memo[0];
    val ? memo[0] = prev | _BV(1) : memo[0] = prev & ~_BV(1);
    if (prev != memo[0]) {
        relaxed = false;
    }

// Eq. 3
    local0 = inputs[1];
    local1 = memo[1];
    local2 = memo[18];
    local3 = memo[0];
    local4 = inputs[0];
    val = (((((((((!(local0 & _BV(2)) && !(local0 & _BV(0))) || (local1 & _BV(3))) && !(local2 & _BV(5))) && !(local3 & _BV(7))) && (local4 & _BV(0))) && ((!(local0 & _BV(2)) && !(local0 & _BV(0))) || (local1 & _BV(7)))) && ((local0 & _BV(1)) || (local1 & _BV(5)) || (local3 & _BV(2)))) && (local4 & _BV(5))) && ((local4 & _BV(2)) || (local4 & _BV(3))));
    prev = memo[0];
    val ? memo[0] = prev | _BV(2) : memo[0] = prev & ~_BV(2);
    if (prev != memo[0]) {
        relaxed = false;
    }

// Eq. 4
    local0 = memo[0];
    local1 = memo[18];
    val = (local0 & _BV(0)) || (local0 & _BV(1)) || (local0 & _BV(2)) || (local1 & _BV(5)) || (local0 & _BV(7)) || (local1 & _BV(4));
    prev = memo[1];
    val ? memo[1] = prev | _BV(0) : memo[1] = prev & ~_BV(0);
    if (prev != memo[1]) {
        relaxed = false;
    }

// Eq. 5
    local0 = inputs[1];
    local1 = memo[1];
    local2 = inputs[0];
    val = (((!(local0 & _BV(3)) && !(local0 & _BV(4))) && (local0 & _BV(5))) && (((local1 & _BV(0)) && ((local2 & _BV(2)) || (local2 & _BV(3)))) || (local2 & _BV(6))));
    prev = memo[0];
    val ? memo[0] = prev | _BV(3) : memo[0] = prev & ~_BV(3);
    if (prev != memo[0]) {
        relaxed = false;
    }

// Eq. 6
    local0 = inputs[1];
    local1 = memo[1];
    local2 = inputs[0];
    val = (((!(local0 & _BV(3)) && !(local0 & _BV(4))) && (local0 & _BV(6))) && (((local1 & _BV(0)) && ((local2 & _BV(2)) || (local2 & _BV(3)))) || (local2 & _BV(6))));
    prev = memo[0];
    val ? memo[0] = prev | _BV(4) : memo[0] = prev & ~_BV(4);
    if (prev != memo[0]) {
        relaxed = false;
    }

// Eq. 7
    local0 = inputs[1];
    local1 = memo[1];
    local2 = memo[0];
    local3 = memo[25];
    local4 = inputs[0];
    val = ((((((local0 & _BV(4)) && (local1 & _BV(0))) || ((((local2 & _BV(5)) && !(local0 & _BV(3))) && !(local3 & _BV(7))) && (!(local1 & _BV(3)) || (local1 & _BV(1))))) && (local1 & _BV(6))) && !(local4 & _BV(6))) && !(local3 & _BV(1)));
    prev = memo[0];
    val ? memo[0] = prev | _BV(5) : memo[0] = prev & ~_BV(5);
    if (prev != memo[0]) {
        relaxed = false;
    }

// Eq. 8
    local0 = memo[1];
    local1 = inputs[1];
    local2 = inputs[0];
    val = (!(local0 & _BV(3)) && ((local1 & _BV(5)) || (local1 & _BV(6)))) || (((local0 & _BV(1)) && (!(local0 & _BV(2)) || ((!(local1 & _BV(5)) && !(local1 & _BV(6))) && (local0 & _BV(7))) || (local2 & _BV(2)))) && (local0 & _BV(6)));
    prev = memo[1];
    val ? memo[1] = prev | _BV(1) : memo[1] = prev & ~_BV(1);
    if (prev != memo[1]) {
        relaxed = false;
    }

// Eq. 9
    local0 = inputs[1];
    local1 = memo[1];
    val = (((((!(local0 & _BV(5)) && !(local0 & _BV(6))) && (local1 & _BV(1))) || (local1 & _BV(2))) && (local1 & _BV(6))) && (local1 & _BV(7)));
    prev = memo[1];
    val ? memo[1] = prev | _BV(2) : memo[1] = prev & ~_BV(2);
    if (prev != memo[1]) {
        relaxed = false;
    }

// Eq. 10
    local0 = inputs[1];
    local1 = memo[1];
    local2 = memo[0];
    local3 = memo[25];
    local4 = inputs[0];
    val = ((((((local0 & _BV(3)) && (local1 & _BV(0))) || ((((local2 & _BV(6)) && !(local0 & _BV(4))) && !(local3 & _BV(6))) && (!(local1 & _BV(3)) || (local1 & _BV(1))))) && (local1 & _BV(6))) && !(local4 & _BV(6))) && (local0 & _BV(7)));
    prev = memo[0];
    val ? memo[0] = prev | _BV(6) : memo[0] = prev & ~_BV(6);
    if (prev != memo[0]) {
        relaxed = false;
    }

// Eq. 11
    local0 = memo[0];
    local1 = memo[1];
    local2 = inputs[0];
    local3 = inputs[1];
    val = (((((local0 & _BV(3)) || (local0 & _BV(4)) || ((local1 & _BV(3)) && (((local2 & _BV(1)) && (local0 & _BV(0))) || ((local2 & _BV(4)) && (local0 & _BV(1))) || ((local2 & _BV(5)) && (local0 & _BV(2)))))) && (local1 & _BV(6))) && (((!(local1 & _BV(2)) && !(local3 & _BV(4))) && !(local3 & _BV(3))) || (local1 & _BV(7)))) && (local2 & _BV(0)));
    prev = memo[1];
    val ? memo[1] = prev | _BV(3) : memo[1] = prev & ~_BV(3);
    if (prev != memo[1]) {
        relaxed = false;
    }

// Eq. 12
    local0 = memo[0];
    val = ((((local0 & _BV(0)) && (local0 & _BV(3))) && !(local0 & _BV(5))) && !(local0 & _BV(6)));
    prev = memo[8];
    val ? memo[8] = prev | _BV(0) : memo[8] = prev & ~_BV(0);
    if (prev != memo[8]) {
        relaxed = false;
    }

// Eq. 13
    local0 = memo[0];
    val = ((((local0 & _BV(0)) && (local0 & _BV(4))) && !(local0 & _BV(5))) && !(local0 & _BV(6)));
    prev = memo[28];
    val ? memo[28] = prev | _BV(0) : memo[28] = prev & ~_BV(0);
    if (prev != memo[28]) {
        relaxed = false;
    }

// Eq. 14
    local0 = inputs[0];
    local1 = memo[18];
    val = (((local0 & _BV(3)) && (local1 & _BV(6))) && (local1 & _BV(5)));
    prev = memo[2];
    val ? memo[2] = prev | _BV(0) : memo[2] = prev & ~_BV(0);
    if (prev != memo[2]) {
        relaxed = false;
    }

// Eq. 15
    local0 = memo[2];
    local1 = inputs[2];
    val = ((local0 & _BV(0)) && (local1 & _BV(0)));
    prev = memo[2];
    val ? memo[2] = prev | _BV(1) : memo[2] = prev & ~_BV(1);
    if (prev != memo[2]) {
        relaxed = false;
    }

// Eq. 16
    local0 = memo[2];
    local1 = inputs[2];
    val = ((local0 & _BV(0)) && (local1 & _BV(1)));
    prev = memo[2];
    val ? memo[2] = prev | _BV(2) : memo[2] = prev & ~_BV(2);
    if (prev != memo[2]) {
        relaxed = false;
    }

// Eq. 17
    local0 = memo[2];
    local1 = inputs[2];
    val = ((local0 & _BV(0)) && (local1 & _BV(2)));
    prev = memo[6];
    val ? memo[6] = prev | _BV(0) : memo[6] = prev & ~_BV(0);
    if (prev != memo[6]) {
        relaxed = false;
    }

// Eq. 18
    local0 = inputs[0];
    local1 = memo[18];
    val = (((local0 & _BV(3)) && (local1 & _BV(7))) && (local1 & _BV(5)));
    prev = memo[2];
    val ? memo[2] = prev | _BV(3) : memo[2] = prev & ~_BV(3);
    if (prev != memo[2]) {
        relaxed = false;
    }

// Eq. 19
    local0 = memo[2];
    local1 = inputs[2];
    val = ((local0 & _BV(3)) && (local1 & _BV(3)));
    prev = memo[2];
    val ? memo[2] = prev | _BV(4) : memo[2] = prev & ~_BV(4);
    if (prev != memo[2]) {
        relaxed = false;
    }

// Eq. 20
    local0 = memo[2];
    local1 = inputs[2];
    val = ((local0 & _BV(3)) && (local1 & _BV(4)));
    prev = memo[2];
    val ? memo[2] = prev | _BV(5) : memo[2] = prev & ~_BV(5);
    if (prev != memo[2]) {
        relaxed = false;
    }

// Eq. 21
    local0 = memo[2];
    local1 = inputs[2];
    val = ((local0 & _BV(3)) && (local1 & _BV(5)));
    prev = memo[6];
    val ? memo[6] = prev | _BV(1) : memo[6] = prev & ~_BV(1);
    if (prev != memo[6]) {
        relaxed = false;
    }

// Eq. 22
    local0 = memo[18];
    local1 = inputs[2];
    val = (((local0 & _BV(5)) && (local0 & _BV(6))) && !(local1 & _BV(6)));
    prev = memo[9];
    val ? memo[9] = prev | _BV(0) : memo[9] = prev & ~_BV(0);
    if (prev != memo[9]) {
        relaxed = false;
    }

// Eq. 23
    local0 = memo[18];
    local1 = inputs[2];
    val = (((local0 & _BV(5)) && (local0 & _BV(7))) && !(local1 & _BV(7)));
    prev = memo[9];
    val ? memo[9] = prev | _BV(1) : memo[9] = prev & ~_BV(1);
    if (prev != memo[9]) {
        relaxed = false;
    }

// Eq. 24
    local0 = memo[0];
    val = ((((local0 & _BV(1)) && (local0 & _BV(3))) && !(local0 & _BV(5))) && !(local0 & _BV(6)));
    prev = memo[28];
    val ? memo[28] = prev | _BV(1) : memo[28] = prev & ~_BV(1);
    if (prev != memo[28]) {
        relaxed = false;
    }

// Eq. 25
    local0 = memo[0];
    val = ((((local0 & _BV(1)) && (local0 & _BV(4))) && !(local0 & _BV(5))) && !(local0 & _BV(6)));
    prev = memo[8];
    val ? memo[8] = prev | _BV(1) : memo[8] = prev & ~_BV(1);
    if (prev != memo[8]) {
        relaxed = false;
    }

// Eq. 26
    local0 = inputs[0];
    local1 = memo[18];
    local2 = memo[0];
    val = (((local0 & _BV(3)) && (local1 & _BV(6))) && (local2 & _BV(7)));
    prev = memo[3];
    val ? memo[3] = prev | _BV(0) : memo[3] = prev & ~_BV(0);
    if (prev != memo[3]) {
        relaxed = false;
    }

// Eq. 27
    local0 = memo[3];
    local1 = inputs[3];
    val = ((local0 & _BV(0)) && (local1 & _BV(0)));
    prev = memo[7];
    val ? memo[7] = prev | _BV(0) : memo[7] = prev & ~_BV(0);
    if (prev != memo[7]) {
        relaxed = false;
    }

// Eq. 28
    local0 = memo[3];
    local1 = inputs[3];
    val = ((local0 & _BV(0)) && (local1 & _BV(1)));
    prev = memo[7];
    val ? memo[7] = prev | _BV(1) : memo[7] = prev & ~_BV(1);
    if (prev != memo[7]) {
        relaxed = false;
    }

// Eq. 29
    local0 = memo[3];
    local1 = inputs[3];
    val = ((local0 & _BV(0)) && (local1 & _BV(2)));
    prev = memo[6];
    val ? memo[6] = prev | _BV(2) : memo[6] = prev & ~_BV(2);
    if (prev != memo[6]) {
        relaxed = false;
    }

// Eq. 30
    local0 = inputs[0];
    local1 = memo[18];
    local2 = memo[0];
    val = (((local0 & _BV(3)) && (local1 & _BV(7))) && (local2 & _BV(7)));
    prev = memo[3];
    val ? memo[3] = prev | _BV(1) : memo[3] = prev & ~_BV(1);
    if (prev != memo[3]) {
        relaxed = false;
    }

// Eq. 31
    local0 = memo[3];
    local1 = inputs[3];
    val = ((local0 & _BV(1)) && (local1 & _BV(3)));
    prev = memo[3];
    val ? memo[3] = prev | _BV(2) : memo[3] = prev & ~_BV(2);
    if (prev != memo[3]) {
        relaxed = false;
    }

// Eq. 32
    local0 = memo[3];
    local1 = inputs[3];
    val = ((local0 & _BV(1)) && (local1 & _BV(4)));
    prev = memo[3];
    val ? memo[3] = prev | _BV(3) : memo[3] = prev & ~_BV(3);
    if (prev != memo[3]) {
        relaxed = false;
    }

// Eq. 33
    local0 = memo[3];
    local1 = inputs[3];
    val = ((local0 & _BV(1)) && (local1 & _BV(5)));
    prev = memo[6];
    val ? memo[6] = prev | _BV(3) : memo[6] = prev & ~_BV(3);
    if (prev != memo[6]) {
        relaxed = false;
    }

// Eq. 34
    local0 = memo[0];
    local1 = memo[18];
    local2 = inputs[3];
    val = (((local0 & _BV(7)) && (local1 & _BV(6))) && !(local2 & _BV(6)));
    prev = memo[9];
    val ? memo[9] = prev | _BV(2) : memo[9] = prev & ~_BV(2);
    if (prev != memo[9]) {
        relaxed = false;
    }

// Eq. 35
    local0 = memo[0];
    local1 = memo[18];
    local2 = inputs[3];
    val = (((local0 & _BV(7)) && (local1 & _BV(7))) && !(local2 & _BV(7)));
    prev = memo[9];
    val ? memo[9] = prev | _BV(3) : memo[9] = prev & ~_BV(3);
    if (prev != memo[9]) {
        relaxed = false;
    }

// Eq. 36
    local0 = memo[0];
    val = ((((local0 & _BV(2)) && (local0 & _BV(4))) && !(local0 & _BV(5))) && !(local0 & _BV(6)));
    prev = memo[8];
    val ? memo[8] = prev | _BV(2) : memo[8] = prev & ~_BV(2);
    if (prev != memo[8]) {
        relaxed = false;
    }

// Eq. 37
    local0 = memo[0];
    val = ((((local0 & _BV(2)) && (local0 & _BV(3))) && !(local0 & _BV(5))) && !(local0 & _BV(6)));
    prev = memo[28];
    val ? memo[28] = prev | _BV(2) : memo[28] = prev & ~_BV(2);
    if (prev != memo[28]) {
        relaxed = false;
    }

// Eq. 38
    local0 = memo[1];
    local1 = memo[0];
    local2 = inputs[1];
    val = ((local0 & _BV(0)) && ((((local1 & _BV(3)) || (local1 & _BV(4))) && (local1 & _BV(5))) || ((local0 & _BV(3)) && (local2 & _BV(4)))));
    prev = memo[20];
    val ? memo[20] = prev | _BV(0) : memo[20] = prev & ~_BV(0);
    if (prev != memo[20]) {
        relaxed = false;
    }

// Eq. 39
    local0 = memo[1];
    local1 = memo[0];
    local2 = inputs[1];
    val = ((local0 & _BV(0)) && ((((local1 & _BV(3)) || (local1 & _BV(4))) && (local1 & _BV(6))) || (((local0 & _BV(3)) && (local2 & _BV(3))) && (local2 & _BV(7)))));
    prev = memo[25];
    val ? memo[25] = prev | _BV(0) : memo[25] = prev & ~_BV(0);
    if (prev != memo[25]) {
        relaxed = false;
    }

// Eq. 40
    local0 = inputs[0];
    local1 = memo[18];
    val = (((local0 & _BV(3)) && (local1 & _BV(6))) && (local1 & _BV(4)));
    prev = memo[4];
    val ? memo[4] = prev | _BV(0) : memo[4] = prev & ~_BV(0);
    if (prev != memo[4]) {
        relaxed = false;
    }

// Eq. 41
    local0 = memo[4];
    local1 = inputs[4];
    val = ((local0 & _BV(0)) && (local1 & _BV(0)));
    prev = memo[5];
    val ? memo[5] = prev | _BV(0) : memo[5] = prev & ~_BV(0);
    if (prev != memo[5]) {
        relaxed = false;
    }

// Eq. 42
    local0 = memo[4];
    local1 = inputs[4];
    val = ((local0 & _BV(0)) && (local1 & _BV(1)));
    prev = memo[5];
    val ? memo[5] = prev | _BV(1) : memo[5] = prev & ~_BV(1);
    if (prev != memo[5]) {
        relaxed = false;
    }

// Eq. 43
    local0 = memo[4];
    local1 = inputs[4];
    val = ((local0 & _BV(0)) && (local1 & _BV(2)));
    prev = memo[6];
    val ? memo[6] = prev | _BV(4) : memo[6] = prev & ~_BV(4);
    if (prev != memo[6]) {
        relaxed = false;
    }

// Eq. 44
    local0 = inputs[0];
    local1 = memo[18];
    val = (((local0 & _BV(3)) && (local1 & _BV(7))) && (local1 & _BV(4)));
    prev = memo[4];
    val ? memo[4] = prev | _BV(1) : memo[4] = prev & ~_BV(1);
    if (prev != memo[4]) {
        relaxed = false;
    }

// Eq. 45
    local0 = memo[4];
    local1 = inputs[4];
    val = ((local0 & _BV(1)) && (local1 & _BV(3)));
    prev = memo[5];
    val ? memo[5] = prev | _BV(2) : memo[5] = prev & ~_BV(2);
    if (prev != memo[5]) {
        relaxed = false;
    }

// Eq. 46
    local0 = memo[4];
    local1 = inputs[4];
    val = ((local0 & _BV(1)) && (local1 & _BV(4)));
    prev = memo[5];
    val ? memo[5] = prev | _BV(3) : memo[5] = prev & ~_BV(3);
    if (prev != memo[5]) {
        relaxed = false;
    }

// Eq. 47
    local0 = memo[4];
    local1 = inputs[4];
    val = ((local0 & _BV(1)) && (local1 & _BV(5)));
    prev = memo[6];
    val ? memo[6] = prev | _BV(5) : memo[6] = prev & ~_BV(5);
    if (prev != memo[6]) {
        relaxed = false;
    }

// Eq. 48
    local0 = memo[18];
    local1 = inputs[4];
    val = (((local0 & _BV(4)) && (local0 & _BV(6))) && !(local1 & _BV(6)));
    prev = memo[9];
    val ? memo[9] = prev | _BV(4) : memo[9] = prev & ~_BV(4);
    if (prev != memo[9]) {
        relaxed = false;
    }

// Eq. 49
    local0 = memo[18];
    local1 = inputs[4];
    val = (((local0 & _BV(4)) && (local0 & _BV(7))) && !(local1 & _BV(7)));
    prev = memo[9];
    val ? memo[9] = prev | _BV(5) : memo[9] = prev & ~_BV(5);
    if (prev != memo[9]) {
        relaxed = false;
    }

// Eq. 50
    local0 = inputs[6];
    local1 = memo[18];
    val = (((local0 & _BV(0)) || (local0 & _BV(1)) || (local0 & _BV(2))) && (local1 & _BV(0)));
    prev = memo[6];
    val ? memo[6] = prev | _BV(6) : memo[6] = prev & ~_BV(6);
    if (prev != memo[6]) {
        relaxed = false;
    }

// Eq. 51
    local0 = memo[6];
    local1 = memo[22];
    val = (((local0 & _BV(6)) && (local0 & _BV(0))) && (local1 & _BV(0)));
    prev = memo[7];
    val ? memo[7] = prev | _BV(2) : memo[7] = prev & ~_BV(2);
    if (prev != memo[7]) {
        relaxed = false;
    }

// Eq. 52
    local0 = memo[6];
    local1 = inputs[2];
    val = ((local0 & _BV(6)) && !(local1 & _BV(2)));
    prev = memo[22];
    val ? memo[22] = prev | _BV(0) : memo[22] = prev & ~_BV(0);
    if (prev != memo[22]) {
        relaxed = false;
    }

// Eq. 53
    local0 = memo[6];
    local1 = memo[17];
    val = (((local0 & _BV(6)) && (local0 & _BV(1))) && (local1 & _BV(0)));
    prev = memo[7];
    val ? memo[7] = prev | _BV(3) : memo[7] = prev & ~_BV(3);
    if (prev != memo[7]) {
        relaxed = false;
    }

// Eq. 54
    local0 = memo[6];
    local1 = inputs[2];
    val = ((local0 & _BV(6)) && !(local1 & _BV(5)));
    prev = memo[17];
    val ? memo[17] = prev | _BV(0) : memo[17] = prev & ~_BV(0);
    if (prev != memo[17]) {
        relaxed = false;
    }

// Eq. 55
    local0 = memo[6];
    local1 = memo[2];
    local2 = inputs[2];
    local3 = memo[7];
    val = ((local0 & _BV(6)) && ((((local1 & _BV(6)) && (local2 & _BV(2))) && !(local3 & _BV(4))) || (local3 & _BV(2))));
    prev = memo[2];
    val ? memo[2] = prev | _BV(6) : memo[2] = prev & ~_BV(6);
    if (prev != memo[2]) {
        relaxed = false;
    }

// Eq. 56
    local0 = memo[6];
    local1 = memo[2];
    local2 = inputs[2];
    local3 = memo[7];
    val = ((local0 & _BV(6)) && ((((local1 & _BV(7)) && (local2 & _BV(5))) && !(local3 & _BV(5))) || (local3 & _BV(3))));
    prev = memo[2];
    val ? memo[2] = prev | _BV(7) : memo[2] = prev & ~_BV(7);
    if (prev != memo[2]) {
        relaxed = false;
    }

// Eq. 57
    local0 = memo[2];
    local1 = memo[6];
    val = ((((local0 & _BV(1)) || (local0 & _BV(2))) && (local0 & _BV(6))) && (local1 & _BV(6)));
    prev = memo[7];
    val ? memo[7] = prev | _BV(4) : memo[7] = prev & ~_BV(4);
    if (prev != memo[7]) {
        relaxed = false;
    }

// Eq. 58
    local0 = memo[2];
    local1 = memo[6];
    val = ((((local0 & _BV(4)) || (local0 & _BV(5))) && (local0 & _BV(7))) && (local1 & _BV(6)));
    prev = memo[7];
    val ? memo[7] = prev | _BV(5) : memo[7] = prev & ~_BV(5);
    if (prev != memo[7]) {
        relaxed = false;
    }

// Eq. 59
    local0 = memo[6];
    local1 = memo[13];
    val = (((local0 & _BV(6)) && (local0 & _BV(2))) && (local1 & _BV(0)));
    prev = memo[8];
    val ? memo[8] = prev | _BV(3) : memo[8] = prev & ~_BV(3);
    if (prev != memo[8]) {
        relaxed = false;
    }

// Eq. 60
    local0 = memo[6];
    local1 = inputs[3];
    val = ((local0 & _BV(6)) && !(local1 & _BV(2)));
    prev = memo[13];
    val ? memo[13] = prev | _BV(0) : memo[13] = prev & ~_BV(0);
    if (prev != memo[13]) {
        relaxed = false;
    }

// Eq. 61
    local0 = memo[6];
    local1 = memo[12];
    val = (((local0 & _BV(6)) && (local0 & _BV(3))) && (local1 & _BV(1)));
    prev = memo[12];
    val ? memo[12] = prev | _BV(0) : memo[12] = prev & ~_BV(0);
    if (prev != memo[12]) {
        relaxed = false;
    }

// Eq. 62
    local0 = memo[6];
    local1 = inputs[3];
    val = ((local0 & _BV(6)) && !(local1 & _BV(5)));
    prev = memo[12];
    val ? memo[12] = prev | _BV(1) : memo[12] = prev & ~_BV(1);
    if (prev != memo[12]) {
        relaxed = false;
    }

// Eq. 63
    local0 = memo[6];
    local1 = inputs[3];
    local2 = memo[3];
    local3 = memo[8];
    val = ((local0 & _BV(6)) && ((((local0 & _BV(7)) && (local1 & _BV(2))) && !(local2 & _BV(5))) || (local3 & _BV(3))));
    prev = memo[6];
    val ? memo[6] = prev | _BV(7) : memo[6] = prev & ~_BV(7);
    if (prev != memo[6]) {
        relaxed = false;
    }

// Eq. 64
    local0 = memo[6];
    local1 = memo[3];
    local2 = inputs[3];
    local3 = memo[12];
    val = ((local0 & _BV(6)) && ((((local1 & _BV(4)) && (local2 & _BV(5))) && !(local1 & _BV(6))) || (local3 & _BV(0))));
    prev = memo[3];
    val ? memo[3] = prev | _BV(4) : memo[3] = prev & ~_BV(4);
    if (prev != memo[3]) {
        relaxed = false;
    }

// Eq. 65
    local0 = memo[7];
    local1 = memo[6];
    val = ((((local0 & _BV(0)) || (local0 & _BV(1))) && (local1 & _BV(7))) && (local1 & _BV(6)));
    prev = memo[3];
    val ? memo[3] = prev | _BV(5) : memo[3] = prev & ~_BV(5);
    if (prev != memo[3]) {
        relaxed = false;
    }

// Eq. 66
    local0 = memo[3];
    local1 = memo[6];
    val = ((((local0 & _BV(2)) || (local0 & _BV(3))) && (local0 & _BV(4))) && (local1 & _BV(6)));
    prev = memo[3];
    val ? memo[3] = prev | _BV(6) : memo[3] = prev & ~_BV(6);
    if (prev != memo[3]) {
        relaxed = false;
    }

// Eq. 67
    local0 = memo[6];
    local1 = memo[8];
    val = (((local0 & _BV(6)) && (local0 & _BV(4))) && (local1 & _BV(4)));
    prev = memo[4];
    val ? memo[4] = prev | _BV(2) : memo[4] = prev & ~_BV(2);
    if (prev != memo[4]) {
        relaxed = false;
    }

// Eq. 68
    local0 = memo[6];
    local1 = inputs[4];
    val = ((local0 & _BV(6)) && !(local1 & _BV(2)));
    prev = memo[8];
    val ? memo[8] = prev | _BV(4) : memo[8] = prev & ~_BV(4);
    if (prev != memo[8]) {
        relaxed = false;
    }

// Eq. 69
    local0 = memo[6];
    local1 = memo[12];
    val = (((local0 & _BV(6)) && (local0 & _BV(5))) && (local1 & _BV(2)));
    prev = memo[5];
    val ? memo[5] = prev | _BV(4) : memo[5] = prev & ~_BV(4);
    if (prev != memo[5]) {
        relaxed = false;
    }

// Eq. 70
    local0 = memo[6];
    local1 = inputs[4];
    val = ((local0 & _BV(6)) && !(local1 & _BV(5)));
    prev = memo[12];
    val ? memo[12] = prev | _BV(2) : memo[12] = prev & ~_BV(2);
    if (prev != memo[12]) {
        relaxed = false;
    }

// Eq. 71
    local0 = memo[6];
    local1 = memo[5];
    local2 = inputs[4];
    local3 = memo[4];
    val = ((local0 & _BV(6)) && ((((local1 & _BV(5)) && (local2 & _BV(2))) && !(local3 & _BV(3))) || (local3 & _BV(2))));
    prev = memo[5];
    val ? memo[5] = prev | _BV(5) : memo[5] = prev & ~_BV(5);
    if (prev != memo[5]) {
        relaxed = false;
    }

// Eq. 72
    local0 = memo[6];
    local1 = memo[5];
    local2 = inputs[4];
    local3 = memo[4];
    val = ((local0 & _BV(6)) && ((((local1 & _BV(6)) && (local2 & _BV(5))) && !(local3 & _BV(4))) || (local1 & _BV(4))));
    prev = memo[5];
    val ? memo[5] = prev | _BV(6) : memo[5] = prev & ~_BV(6);
    if (prev != memo[5]) {
        relaxed = false;
    }

// Eq. 73
    local0 = memo[5];
    local1 = memo[6];
    val = ((((local0 & _BV(0)) || (local0 & _BV(1))) && (local0 & _BV(5))) && (local1 & _BV(6)));
    prev = memo[4];
    val ? memo[4] = prev | _BV(3) : memo[4] = prev & ~_BV(3);
    if (prev != memo[4]) {
        relaxed = false;
    }

// Eq. 74
    local0 = memo[6];
    local1 = memo[5];
    val = (((local0 & _BV(6)) && ((local1 & _BV(2)) || (local1 & _BV(3)))) && (local1 & _BV(6)));
    prev = memo[4];
    val ? memo[4] = prev | _BV(4) : memo[4] = prev & ~_BV(4);
    if (prev != memo[4]) {
        relaxed = false;
    }

// Eq. 75
    local0 = memo[6];
    local1 = memo[3];
    local2 = memo[4];
    local3 = inputs[6];
    val = (((local0 & _BV(6)) && ((local1 & _BV(5)) || (local1 & _BV(6)) || (local2 & _BV(3)) || (local2 & _BV(4)))) && !(local3 & _BV(2)));
    prev = memo[8];
    val ? memo[8] = prev | _BV(5) : memo[8] = prev & ~_BV(5);
    if (prev != memo[8]) {
        relaxed = false;
    }

// Eq. 76
    local0 = memo[6];
    local1 = memo[7];
    local2 = memo[4];
    local3 = inputs[6];
    val = (((local0 & _BV(6)) && ((local1 & _BV(4)) || (local1 & _BV(5)) || (local2 & _BV(3)) || (local2 & _BV(4)))) && !(local3 & _BV(1)));
    prev = memo[1];
    val ? memo[1] = prev | _BV(4) : memo[1] = prev & ~_BV(4);
    if (prev != memo[1]) {
        relaxed = false;
    }

// Eq. 77
    local0 = memo[6];
    local1 = memo[7];
    local2 = memo[3];
    local3 = inputs[6];
    val = (((local0 & _BV(6)) && ((local1 & _BV(4)) || (local1 & _BV(5)) || (local2 & _BV(5)) || (local2 & _BV(6)))) && !(local3 & _BV(0)));
    prev = memo[1];
    val ? memo[1] = prev | _BV(5) : memo[1] = prev & ~_BV(5);
    if (prev != memo[1]) {
        relaxed = false;
    }

// Eq. 78
    local0 = memo[8];
    local1 = memo[1];
    val = (local0 & _BV(5)) || (local1 & _BV(4)) || (local1 & _BV(5));
    prev = memo[10];
    val ? memo[10] = prev | _BV(0) : memo[10] = prev & ~_BV(0);
    if (prev != memo[10]) {
        relaxed = false;
    }

// Eq. 79
    local0 = memo[10];
    local1 = inputs[0];
    val = (!(local0 & _BV(0)) && (local1 & _BV(3)));
    prev = memo[9];
    val ? memo[9] = prev | _BV(6) : memo[9] = prev & ~_BV(6);
    if (prev != memo[9]) {
        relaxed = false;
    }

// Eq. 80
    local0 = memo[9];
    local1 = inputs[2];
    val = (((local0 & _BV(6)) && (local0 & _BV(0))) && (local1 & _BV(3)));
    prev = memo[9];
    val ? memo[9] = prev | _BV(7) : memo[9] = prev & ~_BV(7);
    if (prev != memo[9]) {
        relaxed = false;
    }

// Eq. 81
    local0 = memo[9];
    local1 = inputs[2];
    val = (((local0 & _BV(6)) && (local0 & _BV(1))) && (local1 & _BV(0)));
    prev = memo[10];
    val ? memo[10] = prev | _BV(1) : memo[10] = prev & ~_BV(1);
    if (prev != memo[10]) {
        relaxed = false;
    }

// Eq. 82
    local0 = memo[9];
    local1 = inputs[3];
    val = (((local0 & _BV(6)) && (local0 & _BV(2))) && (local1 & _BV(3)));
    prev = memo[29];
    val ? memo[29] = prev | _BV(0) : memo[29] = prev & ~_BV(0);
    if (prev != memo[29]) {
        relaxed = false;
    }

// Eq. 83
    local0 = memo[9];
    local1 = inputs[3];
    val = (((local0 & _BV(6)) && (local0 & _BV(3))) && (local1 & _BV(0)));
    prev = memo[10];
    val ? memo[10] = prev | _BV(2) : memo[10] = prev & ~_BV(2);
    if (prev != memo[10]) {
        relaxed = false;
    }

// Eq. 84
    local0 = memo[9];
    local1 = inputs[4];
    val = (((local0 & _BV(6)) && (local0 & _BV(4))) && (local1 & _BV(3)));
    prev = memo[10];
    val ? memo[10] = prev | _BV(3) : memo[10] = prev & ~_BV(3);
    if (prev != memo[10]) {
        relaxed = false;
    }

// Eq. 85
    local0 = memo[9];
    local1 = inputs[4];
    val = (((local0 & _BV(6)) && (local0 & _BV(5))) && (local1 & _BV(0)));
    prev = memo[10];
    val ? memo[10] = prev | _BV(4) : memo[10] = prev & ~_BV(4);
    if (prev != memo[10]) {
        relaxed = false;
    }

// Eq. 86
    local0 = inputs[2];
    val = ((!(local0 & _BV(6)) || (local0 & _BV(2)) || (local0 & _BV(1))) && (local0 & _BV(5))) || ((!(local0 & _BV(6)) || (local0 & _BV(0)) || (local0 & _BV(1)) || (local0 & _BV(3)) || (local0 & _BV(2))) && (local0 & _BV(4))) || ((((local0 & _BV(3)) && !(local0 & _BV(5))) || !(local0 & _BV(6)) || (local0 & _BV(1)) || (local0 & _BV(2))) && !(local0 & _BV(7)));
    prev = memo[10];
    val ? memo[10] = prev | _BV(5) : memo[10] = prev & ~_BV(5);
    if (prev != memo[10]) {
        relaxed = false;
    }

// Eq. 87
    local0 = inputs[2];
    val = (((!(local0 & _BV(6)) && !(local0 & _BV(2))) || (local0 & _BV(3)) || (local0 & _BV(5)) || (local0 & _BV(1))) && (local0 & _BV(0))) || (((local0 & _BV(1)) || (local0 & _BV(2))) && (local0 & _BV(3)));
    prev = memo[10];
    val ? memo[10] = prev | _BV(6) : memo[10] = prev & ~_BV(6);
    if (prev != memo[10]) {
        relaxed = false;
    }

// Eq. 88
    local0 = inputs[3];
    val = ((!(local0 & _BV(6)) || (local0 & _BV(2)) || (local0 & _BV(1))) && (local0 & _BV(5))) || ((!(local0 & _BV(6)) || (local0 & _BV(0)) || (local0 & _BV(1)) || (local0 & _BV(3)) || (local0 & _BV(2))) && (local0 & _BV(4))) || ((((local0 & _BV(3)) && !(local0 & _BV(5))) || !(local0 & _BV(6)) || (local0 & _BV(1)) || (local0 & _BV(2))) && !(local0 & _BV(7)));
    prev = memo[10];
    val ? memo[10] = prev | _BV(7) : memo[10] = prev & ~_BV(7);
    if (prev != memo[10]) {
        relaxed = false;
    }

// Eq. 89
    local0 = inputs[3];
    val = (((!(local0 & _BV(6)) && !(local0 & _BV(2))) || (local0 & _BV(3)) || (local0 & _BV(5)) || (local0 & _BV(1))) && (local0 & _BV(0))) || (((local0 & _BV(1)) || (local0 & _BV(2))) && (local0 & _BV(3)));
    prev = memo[11];
    val ? memo[11] = prev | _BV(0) : memo[11] = prev & ~_BV(0);
    if (prev != memo[11]) {
        relaxed = false;
    }

// Eq. 90
    local0 = inputs[4];
    val = ((!(local0 & _BV(6)) || (local0 & _BV(2)) || (local0 & _BV(1))) && (local0 & _BV(5))) || ((!(local0 & _BV(6)) || (local0 & _BV(0)) || (local0 & _BV(1)) || (local0 & _BV(3)) || (local0 & _BV(2))) && (local0 & _BV(4))) || ((((local0 & _BV(3)) && !(local0 & _BV(5))) || !(local0 & _BV(6)) || (local0 & _BV(1)) || (local0 & _BV(2))) && !(local0 & _BV(7)));
    prev = memo[11];
    val ? memo[11] = prev | _BV(1) : memo[11] = prev & ~_BV(1);
    if (prev != memo[11]) {
        relaxed = false;
    }

// Eq. 91
    local0 = inputs[4];
    val = (((!(local0 & _BV(6)) && !(local0 & _BV(2))) || (local0 & _BV(3)) || (local0 & _BV(5)) || (local0 & _BV(1))) && (local0 & _BV(0))) || (((local0 & _BV(1)) || (local0 & _BV(2))) && (local0 & _BV(3)));
    prev = memo[11];
    val ? memo[11] = prev | _BV(2) : memo[11] = prev & ~_BV(2);
    if (prev != memo[11]) {
        relaxed = false;
    }

// Eq. 92
    local0 = memo[10];
    local1 = memo[11];
    local2 = memo[27];
    val = (local0 & _BV(5)) || (local0 & _BV(6)) || (local0 & _BV(7)) || (local1 & _BV(0)) || (local1 & _BV(1)) || (local1 & _BV(2)) || (local2 & _BV(4)) || (local2 & _BV(5)) || (local2 & _BV(6));
    prev = memo[11];
    val ? memo[11] = prev | _BV(3) : memo[11] = prev & ~_BV(3);
    if (prev != memo[11]) {
        relaxed = false;
    }

// Eq. 93
    local0 = memo[8];
    val = (local0 & _BV(6));
    prev = memo[17];
    val ? memo[17] = prev | _BV(1) : memo[17] = prev & ~_BV(1);
    if (prev != memo[17]) {
        relaxed = false;
    }

// Eq. 94
    local0 = inputs[2];
    local1 = memo[28];
    local2 = memo[8];
    local3 = inputs[3];
    local4 = inputs[4];
    local5 = memo[6];
    val = ((((local0 & _BV(5)) && (local1 & _BV(0))) || ((local0 & _BV(2)) && (local2 & _BV(0))) || ((local3 & _BV(5)) && (local2 & _BV(1))) || ((local3 & _BV(2)) && (local1 & _BV(1))) || ((local4 & _BV(5)) && (local1 & _BV(2))) || ((local4 & _BV(2)) && (local2 & _BV(2)))) && (local5 & _BV(6)));
    prev = memo[8];
    val ? memo[8] = prev | _BV(6) : memo[8] = prev & ~_BV(6);
    if (prev != memo[8]) {
        relaxed = false;
    }

// Eq. 95
    local0 = memo[11];
    local1 = inputs[2];
    val = ((local0 & _BV(4)) && (local1 & _BV(6))) || (((!(local1 & _BV(6)) || (local1 & _BV(1)) || (local1 & _BV(0))) && !(local1 & _BV(2))) && (local0 & _BV(5)));
    prev = memo[12];
    val ? memo[12] = prev | _BV(3) : memo[12] = prev & ~_BV(3);
    if (prev != memo[12]) {
        relaxed = false;
    }

// Eq. 96
    local0 = inputs[2];
    local1 = memo[2];
    val = (!(local0 & _BV(6)) && (local1 & _BV(6)));
    prev = memo[11];
    val ? memo[11] = prev | _BV(4) : memo[11] = prev & ~_BV(4);
    if (prev != memo[11]) {
        relaxed = false;
    }

// Eq. 97
    local0 = inputs[2];
    local1 = memo[11];
    local2 = memo[12];
    val = ((((local0 & _BV(0)) || (local0 & _BV(1))) && (local0 & _BV(2))) && !(local1 & _BV(5))) || (local2 & _BV(3));
    prev = memo[12];
    val ? memo[12] = prev | _BV(4) : memo[12] = prev & ~_BV(4);
    if (prev != memo[12]) {
        relaxed = false;
    }

// Eq. 98
    local0 = inputs[2];
    val = (local0 & _BV(2));
    prev = memo[11];
    val ? memo[11] = prev | _BV(5) : memo[11] = prev & ~_BV(5);
    if (prev != memo[11]) {
        relaxed = false;
    }

// Eq. 99
    local0 = memo[27];
    local1 = memo[7];
    local2 = inputs[2];
    val = (((local0 & _BV(0)) && !(local1 & _BV(7))) && (local2 & _BV(2))) || (local1 & _BV(4));
    prev = memo[27];
    val ? memo[27] = prev | _BV(0) : memo[27] = prev & ~_BV(0);
    if (prev != memo[27]) {
        relaxed = false;
    }

// Eq. 100
    local0 = inputs[2];
    local1 = memo[12];
    val = (((local0 & _BV(0)) || !(local0 & _BV(6)) || (local0 & _BV(1))) && (local1 & _BV(5))) || (local1 & _BV(4));
    prev = memo[14];
    val ? memo[14] = prev | _BV(0) : memo[14] = prev & ~_BV(0);
    if (prev != memo[14]) {
        relaxed = false;
    }

// Eq. 101
    local0 = memo[27];
    local1 = inputs[2];
    val = ((((local0 & _BV(0)) && !(local1 & _BV(0))) && (local1 & _BV(6))) && !(local1 & _BV(1)));
    prev = memo[12];
    val ? memo[12] = prev | _BV(5) : memo[12] = prev & ~_BV(5);
    if (prev != memo[12]) {
        relaxed = false;
    }

// Eq. 102
    local0 = memo[13];
    local1 = inputs[2];
    val = ((local0 & _BV(2)) && (local1 & _BV(7))) || (((!(local1 & _BV(7)) || (local1 & _BV(4)) || (local1 & _BV(3))) && !(local1 & _BV(5))) && (local0 & _BV(3)));
    prev = memo[13];
    val ? memo[13] = prev | _BV(1) : memo[13] = prev & ~_BV(1);
    if (prev != memo[13]) {
        relaxed = false;
    }

// Eq. 103
    local0 = inputs[2];
    local1 = memo[2];
    val = (!(local0 & _BV(7)) && (local1 & _BV(7)));
    prev = memo[13];
    val ? memo[13] = prev | _BV(2) : memo[13] = prev & ~_BV(2);
    if (prev != memo[13]) {
        relaxed = false;
    }

// Eq. 104
    local0 = inputs[2];
    local1 = memo[13];
    val = ((((local0 & _BV(3)) || (local0 & _BV(4))) && (local0 & _BV(5))) && !(local1 & _BV(3))) || (local1 & _BV(1));
    prev = memo[12];
    val ? memo[12] = prev | _BV(6) : memo[12] = prev & ~_BV(6);
    if (prev != memo[12]) {
        relaxed = false;
    }

// Eq. 105
    local0 = inputs[2];
    val = (local0 & _BV(5));
    prev = memo[13];
    val ? memo[13] = prev | _BV(3) : memo[13] = prev & ~_BV(3);
    if (prev != memo[13]) {
        relaxed = false;
    }

// Eq. 106
    local0 = memo[7];
    local1 = inputs[2];
    val = (((local0 & _BV(6)) && !(local0 & _BV(7))) && (local1 & _BV(5))) || (local0 & _BV(5));
    prev = memo[7];
    val ? memo[7] = prev | _BV(6) : memo[7] = prev & ~_BV(6);
    if (prev != memo[7]) {
        relaxed = false;
    }

// Eq. 107
    local0 = inputs[2];
    local1 = memo[12];
    val = (((local0 & _BV(3)) || !(local0 & _BV(7)) || (local0 & _BV(4))) && (local1 & _BV(7))) || (local1 & _BV(6));
    prev = memo[14];
    val ? memo[14] = prev | _BV(1) : memo[14] = prev & ~_BV(1);
    if (prev != memo[14]) {
        relaxed = false;
    }

// Eq. 108
    local0 = memo[7];
    local1 = inputs[2];
    val = ((((local0 & _BV(6)) && !(local1 & _BV(3))) && (local1 & _BV(7))) && !(local1 & _BV(4)));
    prev = memo[12];
    val ? memo[12] = prev | _BV(7) : memo[12] = prev & ~_BV(7);
    if (prev != memo[12]) {
        relaxed = false;
    }

// Eq. 109
    local0 = memo[13];
    local1 = inputs[3];
    val = ((local0 & _BV(5)) && (local1 & _BV(6))) || (((!(local1 & _BV(6)) || (local1 & _BV(0)) || (local1 & _BV(1))) && !(local1 & _BV(2))) && (local0 & _BV(6)));
    prev = memo[13];
    val ? memo[13] = prev | _BV(4) : memo[13] = prev & ~_BV(4);
    if (prev != memo[13]) {
        relaxed = false;
    }

// Eq. 110
    local0 = inputs[3];
    local1 = memo[6];
    val = (!(local0 & _BV(6)) && (local1 & _BV(7)));
    prev = memo[13];
    val ? memo[13] = prev | _BV(5) : memo[13] = prev & ~_BV(5);
    if (prev != memo[13]) {
        relaxed = false;
    }

// Eq. 111
    local0 = inputs[3];
    local1 = memo[13];
    val = ((((local0 & _BV(0)) || (local0 & _BV(1))) && (local0 & _BV(2))) && !(local1 & _BV(6))) || (local1 & _BV(4));
    prev = memo[14];
    val ? memo[14] = prev | _BV(2) : memo[14] = prev & ~_BV(2);
    if (prev != memo[14]) {
        relaxed = false;
    }

// Eq. 112
    local0 = inputs[3];
    val = (local0 & _BV(2));
    prev = memo[13];
    val ? memo[13] = prev | _BV(6) : memo[13] = prev & ~_BV(6);
    if (prev != memo[13]) {
        relaxed = false;
    }

// Eq. 113
    local0 = memo[22];
    local1 = memo[7];
    local2 = inputs[3];
    local3 = memo[3];
    val = (((local0 & _BV(1)) && !(local1 & _BV(7))) && (local2 & _BV(2))) || (local3 & _BV(5));
    prev = memo[22];
    val ? memo[22] = prev | _BV(1) : memo[22] = prev & ~_BV(1);
    if (prev != memo[22]) {
        relaxed = false;
    }

// Eq. 114
    local0 = inputs[3];
    local1 = memo[14];
    val = (((local0 & _BV(0)) || !(local0 & _BV(6)) || (local0 & _BV(1))) && (local1 & _BV(4))) || (local1 & _BV(2));
    prev = memo[14];
    val ? memo[14] = prev | _BV(3) : memo[14] = prev & ~_BV(3);
    if (prev != memo[14]) {
        relaxed = false;
    }

// Eq. 115
    local0 = memo[22];
    local1 = inputs[3];
    val = ((((local0 & _BV(1)) && !(local1 & _BV(0))) && (local1 & _BV(6))) && !(local1 & _BV(1)));
    prev = memo[14];
    val ? memo[14] = prev | _BV(4) : memo[14] = prev & ~_BV(4);
    if (prev != memo[14]) {
        relaxed = false;
    }

// Eq. 116
    local0 = memo[14];
    local1 = inputs[3];
    val = ((local0 & _BV(6)) && (local1 & _BV(7))) || (((!(local1 & _BV(7)) || (local1 & _BV(3)) || (local1 & _BV(4))) && !(local1 & _BV(5))) && (local0 & _BV(7)));
    prev = memo[14];
    val ? memo[14] = prev | _BV(5) : memo[14] = prev & ~_BV(5);
    if (prev != memo[14]) {
        relaxed = false;
    }

// Eq. 117
    local0 = inputs[3];
    local1 = memo[3];
    val = (!(local0 & _BV(7)) && (local1 & _BV(4)));
    prev = memo[14];
    val ? memo[14] = prev | _BV(6) : memo[14] = prev & ~_BV(6);
    if (prev != memo[14]) {
        relaxed = false;
    }

// Eq. 118
    local0 = inputs[3];
    local1 = memo[14];
    val = ((((local0 & _BV(3)) || (local0 & _BV(4))) && (local0 & _BV(5))) && !(local1 & _BV(7))) || (local1 & _BV(5));
    prev = memo[15];
    val ? memo[15] = prev | _BV(0) : memo[15] = prev & ~_BV(0);
    if (prev != memo[15]) {
        relaxed = false;
    }

// Eq. 119
    local0 = inputs[3];
    val = (local0 & _BV(5));
    prev = memo[14];
    val ? memo[14] = prev | _BV(7) : memo[14] = prev & ~_BV(7);
    if (prev != memo[14]) {
        relaxed = false;
    }

// Eq. 120
    local0 = memo[3];
    local1 = memo[7];
    local2 = inputs[3];
    val = (((local0 & _BV(7)) && !(local1 & _BV(7))) && (local2 & _BV(5))) || (local0 & _BV(6));
    prev = memo[3];
    val ? memo[3] = prev | _BV(7) : memo[3] = prev & ~_BV(7);
    if (prev != memo[3]) {
        relaxed = false;
    }

// Eq. 121
    local0 = inputs[3];
    local1 = memo[15];
    val = (((local0 & _BV(3)) || !(local0 & _BV(7)) || (local0 & _BV(4))) && (local1 & _BV(2))) || (local1 & _BV(0));
    prev = memo[15];
    val ? memo[15] = prev | _BV(1) : memo[15] = prev & ~_BV(1);
    if (prev != memo[15]) {
        relaxed = false;
    }

// Eq. 122
    local0 = memo[3];
    local1 = inputs[3];
    val = ((((local0 & _BV(7)) && !(local1 & _BV(3))) && (local1 & _BV(7))) && !(local1 & _BV(4)));
    prev = memo[15];
    val ? memo[15] = prev | _BV(2) : memo[15] = prev & ~_BV(2);
    if (prev != memo[15]) {
        relaxed = false;
    }

// Eq. 123
    local0 = memo[15];
    local1 = inputs[4];
    val = ((local0 & _BV(4)) && (local1 & _BV(6))) || (((!(local1 & _BV(6)) || (local1 & _BV(0)) || (local1 & _BV(1))) && !(local1 & _BV(2))) && (local0 & _BV(5)));
    prev = memo[15];
    val ? memo[15] = prev | _BV(3) : memo[15] = prev & ~_BV(3);
    if (prev != memo[15]) {
        relaxed = false;
    }

// Eq. 124
    local0 = inputs[4];
    local1 = memo[5];
    val = (!(local0 & _BV(6)) && (local1 & _BV(5)));
    prev = memo[15];
    val ? memo[15] = prev | _BV(4) : memo[15] = prev & ~_BV(4);
    if (prev != memo[15]) {
        relaxed = false;
    }

// Eq. 125
    local0 = inputs[4];
    local1 = memo[15];
    val = ((((local0 & _BV(0)) || (local0 & _BV(1))) && (local0 & _BV(2))) && !(local1 & _BV(5))) || (local1 & _BV(3));
    prev = memo[16];
    val ? memo[16] = prev | _BV(0) : memo[16] = prev & ~_BV(0);
    if (prev != memo[16]) {
        relaxed = false;
    }

// Eq. 126
    local0 = inputs[4];
    val = (local0 & _BV(2));
    prev = memo[15];
    val ? memo[15] = prev | _BV(5) : memo[15] = prev & ~_BV(5);
    if (prev != memo[15]) {
        relaxed = false;
    }

// Eq. 127
    local0 = memo[4];
    local1 = memo[7];
    local2 = inputs[4];
    val = (((local0 & _BV(5)) && !(local1 & _BV(7))) && (local2 & _BV(2))) || (local0 & _BV(3));
    prev = memo[4];
    val ? memo[4] = prev | _BV(5) : memo[4] = prev & ~_BV(5);
    if (prev != memo[4]) {
        relaxed = false;
    }

// Eq. 128
    local0 = inputs[4];
    local1 = memo[16];
    val = (((local0 & _BV(0)) || !(local0 & _BV(6)) || (local0 & _BV(1))) && (local1 & _BV(1))) || (local1 & _BV(0));
    prev = memo[15];
    val ? memo[15] = prev | _BV(6) : memo[15] = prev & ~_BV(6);
    if (prev != memo[15]) {
        relaxed = false;
    }

// Eq. 129
    local0 = memo[4];
    local1 = inputs[4];
    local2 = inputs[0];
    val = (((((local0 & _BV(5)) && !(local1 & _BV(0))) && (local1 & _BV(6))) && !(local1 & _BV(1))) && !(local2 & _BV(7)));
    prev = memo[16];
    val ? memo[16] = prev | _BV(1) : memo[16] = prev & ~_BV(1);
    if (prev != memo[16]) {
        relaxed = false;
    }

// Eq. 130
    local0 = memo[16];
    local1 = inputs[4];
    val = ((local0 & _BV(3)) && (local1 & _BV(7))) || (((!(local1 & _BV(7)) || (local1 & _BV(3)) || (local1 & _BV(4))) && !(local1 & _BV(5))) && (local0 & _BV(5)));
    prev = memo[16];
    val ? memo[16] = prev | _BV(2) : memo[16] = prev & ~_BV(2);
    if (prev != memo[16]) {
        relaxed = false;
    }

// Eq. 131
    local0 = inputs[4];
    local1 = memo[5];
    val = (!(local0 & _BV(7)) && (local1 & _BV(6)));
    prev = memo[16];
    val ? memo[16] = prev | _BV(3) : memo[16] = prev & ~_BV(3);
    if (prev != memo[16]) {
        relaxed = false;
    }

// Eq. 132
    local0 = inputs[4];
    local1 = memo[16];
    val = ((((local0 & _BV(3)) || (local0 & _BV(4))) && (local0 & _BV(5))) && !(local1 & _BV(5))) || (local1 & _BV(2));
    prev = memo[16];
    val ? memo[16] = prev | _BV(4) : memo[16] = prev & ~_BV(4);
    if (prev != memo[16]) {
        relaxed = false;
    }

// Eq. 133
    local0 = inputs[4];
    val = (local0 & _BV(5));
    prev = memo[16];
    val ? memo[16] = prev | _BV(5) : memo[16] = prev & ~_BV(5);
    if (prev != memo[16]) {
        relaxed = false;
    }

// Eq. 134
    local0 = memo[4];
    local1 = memo[7];
    local2 = inputs[4];
    val = (((local0 & _BV(6)) && !(local1 & _BV(7))) && (local2 & _BV(5))) || (local0 & _BV(4));
    prev = memo[4];
    val ? memo[4] = prev | _BV(6) : memo[4] = prev & ~_BV(6);
    if (prev != memo[4]) {
        relaxed = false;
    }

// Eq. 135
    local0 = inputs[4];
    local1 = memo[16];
    val = (((local0 & _BV(3)) || !(local0 & _BV(7)) || (local0 & _BV(4))) && (local1 & _BV(6))) || (local1 & _BV(4));
    prev = memo[11];
    val ? memo[11] = prev | _BV(6) : memo[11] = prev & ~_BV(6);
    if (prev != memo[11]) {
        relaxed = false;
    }

// Eq. 136
    local0 = memo[4];
    local1 = inputs[4];
    val = ((((local0 & _BV(6)) && !(local1 & _BV(3))) && (local1 & _BV(7))) && !(local1 & _BV(4)));
    prev = memo[16];
    val ? memo[16] = prev | _BV(6) : memo[16] = prev & ~_BV(6);
    if (prev != memo[16]) {
        relaxed = false;
    }

// Eq. 137
    local0 = memo[7];
    local1 = inputs[5];
    local2 = memo[14];
    local3 = memo[15];
    local4 = memo[11];
    local5 = inputs[0];
    val = ((((local0 & _BV(7)) && (local1 & _BV(0))) || (local2 & _BV(0)) || (local2 & _BV(1)) || (local2 & _BV(3)) || (local3 & _BV(1)) || (local3 & _BV(6)) || (local4 & _BV(6)) || (local4 & _BV(3)) || (local4 & _BV(7))) && (local5 & _BV(0)));
    prev = memo[7];
    val ? memo[7] = prev | _BV(7) : memo[7] = prev & ~_BV(7);
    if (prev != memo[7]) {
        relaxed = false;
    }

// Eq. 138
    local0 = inputs[6];
    local1 = memo[17];
    val = ((((local0 & _BV(0)) || (local1 & _BV(3))) && ((local0 & _BV(1)) || (local1 & _BV(4)))) && ((local0 & _BV(2)) || (local1 & _BV(5))));
    prev = memo[17];
    val ? memo[17] = prev | _BV(2) : memo[17] = prev & ~_BV(2);
    if (prev != memo[17]) {
        relaxed = false;
    }

// Eq. 139
    local0 = inputs[6];
    val = !(local0 & _BV(0));
    prev = memo[17];
    val ? memo[17] = prev | _BV(3) : memo[17] = prev & ~_BV(3);
    if (prev != memo[17]) {
        relaxed = false;
    }

// Eq. 140
    local0 = inputs[6];
    val = !(local0 & _BV(1));
    prev = memo[17];
    val ? memo[17] = prev | _BV(4) : memo[17] = prev & ~_BV(4);
    if (prev != memo[17]) {
        relaxed = false;
    }

// Eq. 141
    local0 = inputs[6];
    val = !(local0 & _BV(2));
    prev = memo[17];
    val ? memo[17] = prev | _BV(5) : memo[17] = prev & ~_BV(5);
    if (prev != memo[17]) {
        relaxed = false;
    }

// Eq. 142
    local0 = inputs[5];
    local1 = memo[22];
    local2 = inputs[7];
    local3 = inputs[0];
    local4 = memo[7];
    local5 = memo[5];
    local6 = inputs[6];
    local7 = outputs[0];
    val = (((((((((((local0 & _BV(0)) && (local0 & _BV(1))) || (local1 & _BV(2))) && (local2 & _BV(0))) && (local3 & _BV(0))) && (local0 & _BV(2))) && !(local0 & _BV(3))) && ((local3 & _BV(2)) || (local3 & _BV(6)) || (local3 & _BV(3)))) && !(local4 & _BV(7))) && !(local5 & _BV(7))) && ((local6 & _BV(3)) || (local6 & _BV(4)) || (local6 & _BV(5)) || (((local2 & _BV(1)) && !(local7 & _BV(1))) && !(local7 & _BV(2)))));
    prev = memo[1];
    val ? memo[1] = prev | _BV(6) : memo[1] = prev & ~_BV(6);
    if (prev != memo[1]) {
        relaxed = false;
    }

// Eq. 143
    local0 = inputs[5];
    local1 = outputs[3];
    local2 = memo[0];
    val = ((local0 & _BV(0)) && ((local0 & _BV(1)) || ((local1 & _BV(5)) && (local2 & _BV(0))) || ((local1 & _BV(6)) && (local2 & _BV(1))) || ((local1 & _BV(7)) && (local2 & _BV(2)))));
    prev = memo[22];
    val ? memo[22] = prev | _BV(2) : memo[22] = prev & ~_BV(2);
    if (prev != memo[22]) {
        relaxed = false;
    }

// Eq. 144
    local0 = memo[0];
    local1 = inputs[1];
    val = ((local0 & _BV(0)) && (local1 & _BV(2))) || ((local0 & _BV(1)) && (local1 & _BV(0))) || ((local0 & _BV(2)) && (local1 & _BV(1)));
    prev = memo[17];
    val ? memo[17] = prev | _BV(6) : memo[17] = prev & ~_BV(6);
    if (prev != memo[17]) {
        relaxed = false;
    }

// Eq. 145
    local0 = memo[18];
    local1 = memo[17];
    local2 = memo[0];
    local3 = memo[1];
    local4 = inputs[6];
    val = (((((((local0 & _BV(0)) && !(local1 & _BV(1))) || (local2 & _BV(3)) || (local2 & _BV(4))) && (local1 & _BV(2))) && !(local1 & _BV(7))) && (local3 & _BV(6))) && ((local4 & _BV(6)) || (((local4 & _BV(0)) || (local4 & _BV(1)) || (local4 & _BV(2))) && !(local4 & _BV(4)))));
    prev = memo[18];
    val ? memo[18] = prev | _BV(0) : memo[18] = prev & ~_BV(0);
    if (prev != memo[18]) {
        relaxed = false;
    }

// Eq. 146
    local0 = memo[18];
    local1 = memo[1];
    local2 = inputs[0];
    val = ((((local0 & _BV(1)) || (local0 & _BV(0))) && ((local1 & _BV(7)) || (local2 & _BV(6)))) && (local1 & _BV(6)));
    prev = memo[18];
    val ? memo[18] = prev | _BV(1) : memo[18] = prev & ~_BV(1);
    if (prev != memo[18]) {
        relaxed = false;
    }

// Eq. 147
    local0 = memo[18];
    local1 = memo[0];
    local2 = memo[1];
    local3 = memo[8];
    local4 = inputs[0];
    local5 = inputs[1];
    val = (((((((((local0 & _BV(2)) || ((local1 & _BV(0)) && (local2 & _BV(3)))) && !(local0 & _BV(4))) && !(local0 & _BV(3))) || (local3 & _BV(5))) && (local4 & _BV(1))) && (local0 & _BV(0))) && ((!(local5 & _BV(0)) && !(local5 & _BV(1))) || (local2 & _BV(7)))) && (local4 & _BV(0)));
    prev = memo[18];
    val ? memo[18] = prev | _BV(2) : memo[18] = prev & ~_BV(2);
    if (prev != memo[18]) {
        relaxed = false;
    }

// Eq. 148
    local0 = memo[18];
    local1 = memo[0];
    local2 = memo[1];
    local3 = inputs[0];
    local4 = inputs[1];
    val = (((((((((local0 & _BV(3)) || ((local1 & _BV(1)) && (local2 & _BV(3)))) && !(local0 & _BV(2))) && !(local0 & _BV(4))) || (local2 & _BV(4))) && (local3 & _BV(4))) && (local0 & _BV(0))) && ((!(local4 & _BV(2)) && !(local4 & _BV(1))) || (local2 & _BV(7)))) && (local3 & _BV(0)));
    prev = memo[18];
    val ? memo[18] = prev | _BV(3) : memo[18] = prev & ~_BV(3);
    if (prev != memo[18]) {
        relaxed = false;
    }

// Eq. 149
    local0 = memo[0];
    local1 = memo[1];
    local2 = memo[18];
    local3 = inputs[0];
    local4 = inputs[1];
    val = ((((((((((local0 & _BV(2)) && (local1 & _BV(3))) || (local2 & _BV(4))) && !(local2 & _BV(2))) && !(local2 & _BV(3))) || (local1 & _BV(5))) && (local3 & _BV(5))) && (local2 & _BV(0))) && ((!(local4 & _BV(2)) && !(local4 & _BV(0))) || (local1 & _BV(7)))) && (local3 & _BV(0)));
    prev = memo[18];
    val ? memo[18] = prev | _BV(4) : memo[18] = prev & ~_BV(4);
    if (prev != memo[18]) {
        relaxed = false;
    }

// Eq. 150
    local0 = memo[18];
    val = ((((local0 & _BV(2)) && !(local0 & _BV(3))) && !(local0 & _BV(4))) && (local0 & _BV(0)));
    prev = memo[18];
    val ? memo[18] = prev | _BV(5) : memo[18] = prev & ~_BV(5);
    if (prev != memo[18]) {
        relaxed = false;
    }

// Eq. 151
    local0 = memo[18];
    val = (((local0 & _BV(3)) && !(local0 & _BV(4))) && (local0 & _BV(0)));
    prev = memo[0];
    val ? memo[0] = prev | _BV(7) : memo[0] = prev & ~_BV(7);
    if (prev != memo[0]) {
        relaxed = false;
    }

// Eq. 152
    local0 = memo[6];
    val = (local0 & _BV(0)) || (local0 & _BV(2)) || (local0 & _BV(4)) || (local0 & _BV(1)) || (local0 & _BV(3)) || (local0 & _BV(5));
    prev = memo[25];
    val ? memo[25] = prev | _BV(1) : memo[25] = prev & ~_BV(1);
    if (prev != memo[25]) {
        relaxed = false;
    }

// Eq. 153
    local0 = inputs[6];
    local1 = memo[6];
    local2 = memo[9];
    val = (((local0 & _BV(6)) || !(local1 & _BV(0))) && (local2 & _BV(0)));
    prev = memo[19];
    val ? memo[19] = prev | _BV(0) : memo[19] = prev & ~_BV(0);
    if (prev != memo[19]) {
        relaxed = false;
    }

// Eq. 154
    local0 = memo[9];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(1)) && ((local1 & _BV(6)) || !(local2 & _BV(1))));
    prev = memo[19];
    val ? memo[19] = prev | _BV(1) : memo[19] = prev & ~_BV(1);
    if (prev != memo[19]) {
        relaxed = false;
    }

// Eq. 155
    local0 = inputs[6];
    local1 = memo[6];
    local2 = memo[9];
    val = (((local0 & _BV(6)) || !(local1 & _BV(2))) && (local2 & _BV(2)));
    prev = memo[19];
    val ? memo[19] = prev | _BV(2) : memo[19] = prev & ~_BV(2);
    if (prev != memo[19]) {
        relaxed = false;
    }

// Eq. 156
    local0 = inputs[6];
    local1 = memo[6];
    local2 = memo[9];
    val = (((local0 & _BV(6)) || !(local1 & _BV(3))) && (local2 & _BV(3)));
    prev = memo[19];
    val ? memo[19] = prev | _BV(3) : memo[19] = prev & ~_BV(3);
    if (prev != memo[19]) {
        relaxed = false;
    }

// Eq. 157
    local0 = inputs[6];
    local1 = memo[6];
    local2 = memo[9];
    val = (((local0 & _BV(6)) || !(local1 & _BV(4))) && (local2 & _BV(4)));
    prev = memo[19];
    val ? memo[19] = prev | _BV(4) : memo[19] = prev & ~_BV(4);
    if (prev != memo[19]) {
        relaxed = false;
    }

// Eq. 158
    local0 = inputs[6];
    local1 = memo[6];
    local2 = memo[9];
    val = (((local0 & _BV(6)) || !(local1 & _BV(5))) && (local2 & _BV(5)));
    prev = memo[19];
    val ? memo[19] = prev | _BV(5) : memo[19] = prev & ~_BV(5);
    if (prev != memo[19]) {
        relaxed = false;
    }

// Eq. 159
    local0 = memo[9];
    local1 = memo[10];
    local2 = inputs[1];
    local3 = memo[0];
    local4 = memo[6];
    local5 = memo[18];
    local6 = inputs[0];
    val = (((((local0 & _BV(1)) || (local0 & _BV(3)) || (local0 & _BV(5))) && (local1 & _BV(0))) || (local1 & _BV(1)) || (local1 & _BV(2)) || (local1 & _BV(4)) || ((local2 & _BV(5)) && ((local3 & _BV(0)) || (local3 & _BV(1)) || ((local4 & _BV(6)) && ((local5 & _BV(2)) || (local5 & _BV(3)))))) || ((local2 & _BV(6)) && ((local3 & _BV(2)) || ((local4 & _BV(6)) && (local5 & _BV(4)))))) && ((local6 & _BV(2)) || (local6 & _BV(3))));
    prev = memo[19];
    val ? memo[19] = prev | _BV(6) : memo[19] = prev & ~_BV(6);
    if (prev != memo[19]) {
        relaxed = false;
    }

// Eq. 160
    local0 = memo[9];
    local1 = memo[10];
    local2 = memo[29];
    local3 = inputs[1];
    local4 = memo[0];
    local5 = memo[6];
    local6 = memo[18];
    local7 = inputs[0];
    val = (((((local0 & _BV(0)) || (local0 & _BV(2)) || (local0 & _BV(4))) && (local1 & _BV(0))) || (local0 & _BV(7)) || (local2 & _BV(0)) || (local1 & _BV(3)) || ((local3 & _BV(6)) && ((local4 & _BV(0)) || (local4 & _BV(1)) || ((local5 & _BV(6)) && ((local6 & _BV(3)) || (local6 & _BV(2)))))) || ((local3 & _BV(5)) && ((local4 & _BV(2)) || ((local5 & _BV(6)) && (local6 & _BV(4)))))) && ((local7 & _BV(2)) || (local7 & _BV(3))));
    prev = memo[19];
    val ? memo[19] = prev | _BV(7) : memo[19] = prev & ~_BV(7);
    if (prev != memo[19]) {
        relaxed = false;
    }

// Eq. 161
    local0 = memo[18];
    local1 = memo[19];
    val = (((((((local0 & _BV(6)) && !(local1 & _BV(7))) || (local1 & _BV(6))) && (local0 & _BV(0))) && !(local1 & _BV(0))) && !(local1 & _BV(2))) && !(local1 & _BV(4)));
    prev = memo[18];
    val ? memo[18] = prev | _BV(6) : memo[18] = prev & ~_BV(6);
    if (prev != memo[18]) {
        relaxed = false;
    }

// Eq. 162
    local0 = memo[18];
    local1 = memo[19];
    val = (((((((local0 & _BV(7)) && !(local1 & _BV(6))) || (local1 & _BV(7))) && (local0 & _BV(0))) && !(local1 & _BV(1))) && !(local1 & _BV(3))) && !(local1 & _BV(5)));
    prev = memo[18];
    val ? memo[18] = prev | _BV(7) : memo[18] = prev & ~_BV(7);
    if (prev != memo[18]) {
        relaxed = false;
    }

// Eq. 163
    local0 = memo[18];
    local1 = memo[27];
    val = ((local0 & _BV(6)) && (local1 & _BV(1))) || ((local0 & _BV(7)) && (local1 & _BV(2)));
    prev = memo[1];
    val ? memo[1] = prev | _BV(7) : memo[1] = prev & ~_BV(7);
    if (prev != memo[1]) {
        relaxed = false;
    }

// Eq. 164
    local0 = inputs[0];
    local1 = inputs[6];
    local2 = memo[6];
    val = (((!(local0 & _BV(5)) && !(local1 & _BV(0))) || (!(local0 & _BV(1)) && !(local1 & _BV(2))) || (!(local0 & _BV(4)) && !(local1 & _BV(1)))) && (local2 & _BV(6)));
    prev = memo[17];
    val ? memo[17] = prev | _BV(7) : memo[17] = prev & ~_BV(7);
    if (prev != memo[17]) {
        relaxed = false;
    }

// Eq. 165
    local0 = memo[25];
    local1 = inputs[6];
    local2 = memo[2];
    local3 = memo[6];
    local4 = memo[3];
    local5 = memo[5];
    val = ((local0 & _BV(1)) && (local1 & _BV(6))) || (local2 & _BV(6)) || (local2 & _BV(7)) || (local3 & _BV(7)) || (local4 & _BV(4)) || (local5 & _BV(5)) || (local5 & _BV(6));
    prev = memo[20];
    val ? memo[20] = prev | _BV(1) : memo[20] = prev & ~_BV(1);
    if (prev != memo[20]) {
        relaxed = false;
    }

// Eq. 166
    local0 = memo[1];
    local1 = memo[20];
    local2 = inputs[0];
    local3 = memo[25];
    local4 = inputs[1];
    val = ((((local0 & _BV(7)) && (((((local1 & _BV(2)) && !(local2 & _BV(2))) && !(local1 & _BV(4))) && !(local1 & _BV(3))) || (local3 & _BV(0)) || (local1 & _BV(1)))) && !(local2 & _BV(6))) && (local4 & _BV(7)));
    prev = memo[20];
    val ? memo[20] = prev | _BV(2) : memo[20] = prev & ~_BV(2);
    if (prev != memo[20]) {
        relaxed = false;
    }

// Eq. 167
    local0 = memo[1];
    local1 = memo[20];
    local2 = inputs[0];
    local3 = memo[25];
    local4 = memo[0];
    val = ((((local0 & _BV(7)) && (((((local1 & _BV(3)) && !(local2 & _BV(2))) && !(local1 & _BV(4))) && !(local1 & _BV(2))) || (((local3 & _BV(6)) && !(local4 & _BV(3))) && !(local4 & _BV(4))) || (((local1 & _BV(0)) && !(local3 & _BV(7))) && !(local3 & _BV(1))))) && !(local2 & _BV(6))) && !(local3 & _BV(0)));
    prev = memo[20];
    val ? memo[20] = prev | _BV(3) : memo[20] = prev & ~_BV(3);
    if (prev != memo[20]) {
        relaxed = false;
    }

// Eq. 168
    local0 = memo[1];
    local1 = memo[20];
    local2 = inputs[0];
    local3 = memo[24];
    local4 = memo[0];
    val = ((local0 & _BV(7)) && (((((local1 & _BV(4)) && !(local1 & _BV(3))) && !(local1 & _BV(2))) && !(local2 & _BV(2))) || (local3 & _BV(7)) || (local3 & _BV(0)) || (local3 & _BV(3)) || (local3 & _BV(6)) || (local3 & _BV(2)) || (local3 & _BV(4)) || (((local4 & _BV(3)) || (local4 & _BV(4))) && !(local4 & _BV(5)))));
    prev = memo[20];
    val ? memo[20] = prev | _BV(4) : memo[20] = prev & ~_BV(4);
    if (prev != memo[20]) {
        relaxed = false;
    }

// Eq. 169
    local0 = memo[0];
    local1 = memo[18];
    local2 = inputs[0];
    val = (((local0 & _BV(0)) || (local1 & _BV(5))) && !(local2 & _BV(6)));
    prev = outputs[3];
    val ? outputs[3] = prev | _BV(0) : outputs[3] = prev & ~_BV(0);
    if (prev != outputs[3]) {
        relaxed = false;
    }

// Eq. 170
    local0 = memo[0];
    local1 = inputs[0];
    val = (((local0 & _BV(1)) || (local0 & _BV(7))) && !(local1 & _BV(6)));
    prev = outputs[0];
    val ? outputs[0] = prev | _BV(0) : outputs[0] = prev & ~_BV(0);
    if (prev != outputs[0]) {
        relaxed = false;
    }

// Eq. 171
    local0 = memo[0];
    local1 = memo[18];
    local2 = inputs[0];
    val = (((local0 & _BV(2)) || (local1 & _BV(4))) && !(local2 & _BV(6)));
    prev = outputs[1];
    val ? outputs[1] = prev | _BV(0) : outputs[1] = prev & ~_BV(0);
    if (prev != outputs[1]) {
        relaxed = false;
    }

// Eq. 172
    local0 = memo[18];
    local1 = memo[0];
    local2 = memo[1];
    local3 = inputs[0];
    local4 = outputs[1];
    local5 = memo[20];
    local6 = inputs[6];
    val = (((((((local0 & _BV(6)) && ((local0 & _BV(5)) || (local1 & _BV(7)))) || ((local0 & _BV(7)) && (local0 & _BV(4)))) && (local2 & _BV(7))) && (local3 & _BV(3))) || (((local1 & _BV(3)) && (local3 & _BV(2))) && (local2 & _BV(7)))) && ((local4 & _BV(7)) || (local5 & _BV(7)) || (local5 & _BV(6)))) || (((((local3 & _BV(6)) && ((local1 & _BV(3)) || (local4 & _BV(1)))) && (local2 & _BV(6))) && !(local4 & _BV(2))) && ((local6 & _BV(3)) || (local6 & _BV(5))));
    prev = outputs[1];
    val ? outputs[1] = prev | _BV(1) : outputs[1] = prev & ~_BV(1);
    if (prev != outputs[1]) {
        relaxed = false;
    }

// Eq. 173
    local0 = memo[18];
    local1 = memo[0];
    local2 = memo[1];
    local3 = inputs[0];
    local4 = outputs[1];
    local5 = memo[20];
    local6 = inputs[6];
    val = (((((((local0 & _BV(7)) && ((local0 & _BV(5)) || (local1 & _BV(7)))) || ((local0 & _BV(6)) && (local0 & _BV(4)))) && (local2 & _BV(7))) && (local3 & _BV(3))) || (((local1 & _BV(4)) && (local3 & _BV(2))) && (local2 & _BV(7)))) && ((local4 & _BV(7)) || (local5 & _BV(7)) || (local5 & _BV(6)))) || (((((local3 & _BV(6)) && ((local1 & _BV(4)) || (local4 & _BV(2)))) && (local2 & _BV(6))) && !(local4 & _BV(1))) && ((local6 & _BV(3)) || (local6 & _BV(5))));
    prev = outputs[1];
    val ? outputs[1] = prev | _BV(2) : outputs[1] = prev & ~_BV(2);
    if (prev != outputs[1]) {
        relaxed = false;
    }

// Eq. 174
    local0 = memo[0];
    local1 = memo[20];
    val = (local0 & _BV(5)) || (local1 & _BV(3));
    prev = outputs[3];
    val ? outputs[3] = prev | _BV(1) : outputs[3] = prev & ~_BV(1);
    if (prev != outputs[3]) {
        relaxed = false;
    }

// Eq. 175
    local0 = memo[0];
    local1 = memo[20];
    local2 = inputs[1];
    val = (((local0 & _BV(6)) || (local1 & _BV(2))) && (local2 & _BV(7)));
    prev = outputs[2];
    val ? outputs[2] = prev | _BV(0) : outputs[2] = prev & ~_BV(0);
    if (prev != outputs[2]) {
        relaxed = false;
    }

// Eq. 176
    local0 = inputs[6];
    local1 = memo[18];
    val = (((!(local0 & _BV(7)) && (local1 & _BV(7))) || ((local0 & _BV(7)) && (local1 & _BV(6)))) && (local0 & _BV(4))) || (local0 & _BV(3));
    prev = memo[21];
    val ? memo[21] = prev | _BV(0) : memo[21] = prev & ~_BV(0);
    if (prev != memo[21]) {
        relaxed = false;
    }

// Eq. 177
    local0 = inputs[6];
    local1 = memo[18];
    val = ((((local0 & _BV(7)) && (local1 & _BV(7))) || (!(local0 & _BV(7)) && (local1 & _BV(6)))) && (local0 & _BV(4))) || (local0 & _BV(5));
    prev = memo[21];
    val ? memo[21] = prev | _BV(1) : memo[21] = prev & ~_BV(1);
    if (prev != memo[21]) {
        relaxed = false;
    }

// Eq. 178
    local0 = memo[23];
    local1 = memo[20];
    local2 = inputs[0];
    local3 = inputs[1];
    local4 = inputs[6];
    local5 = memo[0];
    local6 = memo[18];
    local7 = outputs[1];
    val = ((((local0 & _BV(6)) || (local1 & _BV(4)) || (((local2 & _BV(2)) && (local0 & _BV(0))) && !(local3 & _BV(4))) || (((local2 & _BV(6)) && ((local4 & _BV(3)) || (local4 & _BV(5)))) && ((local5 & _BV(3)) || (local5 & _BV(4)) || (local0 & _BV(0))))) && (local6 & _BV(1))) && !(local7 & _BV(3)));
    prev = memo[23];
    val ? memo[23] = prev | _BV(0) : memo[23] = prev & ~_BV(0);
    if (prev != memo[23]) {
        relaxed = false;
    }

// Eq. 179
    local0 = memo[23];
    local1 = inputs[6];
    val = ((((local0 & _BV(1)) && !(local1 & _BV(4))) || (local0 & _BV(4)) || (local0 & _BV(5))) && (local0 & _BV(0)));
    prev = memo[23];
    val ? memo[23] = prev | _BV(1) : memo[23] = prev & ~_BV(1);
    if (prev != memo[23]) {
        relaxed = false;
    }

// Eq. 180
    local0 = memo[23];
    local1 = memo[28];
    val = ((!(local0 & _BV(1)) && (local1 & _BV(5))) && !(local1 & _BV(3)));
    prev = memo[23];
    val ? memo[23] = prev | _BV(2) : memo[23] = prev & ~_BV(2);
    if (prev != memo[23]) {
        relaxed = false;
    }

// Eq. 181
    local0 = memo[23];
    local1 = inputs[0];
    val = ((((local0 & _BV(1)) || (local0 & _BV(2))) && !(local0 & _BV(7))) && (local1 & _BV(0)));
    prev = memo[21];
    val ? memo[21] = prev | _BV(2) : memo[21] = prev & ~_BV(2);
    if (prev != memo[21]) {
        relaxed = false;
    }

// Eq. 182
    local0 = memo[23];
    local1 = inputs[5];
    local2 = inputs[7];
    local3 = inputs[0];
    local4 = inputs[6];
    local5 = outputs[1];
    val = ((local0 & _BV(0)) && (local0 & _BV(3))) || ((((((((local1 & _BV(2)) && (local1 & _BV(4))) && (local2 & _BV(0))) && !(local2 & _BV(2))) && !(local2 & _BV(3))) && ((local3 & _BV(2)) || (local3 & _BV(6)) || (local3 & _BV(3)))) && ((local4 & _BV(6)) || (local4 & _BV(0)) || (local4 & _BV(1)) || (local4 & _BV(2)))) && ((local2 & _BV(4)) || !(local5 & _BV(7))));
    prev = memo[21];
    val ? memo[21] = prev | _BV(3) : memo[21] = prev & ~_BV(3);
    if (prev != memo[21]) {
        relaxed = false;
    }

// Eq. 183
    local0 = memo[21];
    val = ((local0 & _BV(4)) && (local0 & _BV(2))) || ((local0 & _BV(3)) && (local0 & _BV(0)));
    prev = memo[21];
    val ? memo[21] = prev | _BV(4) : memo[21] = prev & ~_BV(4);
    if (prev != memo[21]) {
        relaxed = false;
    }

// Eq. 184
    local0 = memo[21];
    val = ((local0 & _BV(5)) && (local0 & _BV(2))) || ((local0 & _BV(3)) && (local0 & _BV(1)));
    prev = memo[21];
    val ? memo[21] = prev | _BV(5) : memo[21] = prev & ~_BV(5);
    if (prev != memo[21]) {
        relaxed = false;
    }

// Eq. 185
    local0 = memo[21];
    local1 = outputs[0];
    local2 = inputs[0];
    val = ((((local0 & _BV(6)) && (local0 & _BV(4))) && !(local1 & _BV(2))) && (local2 & _BV(0)));
    prev = outputs[0];
    val ? outputs[0] = prev | _BV(1) : outputs[0] = prev & ~_BV(1);
    if (prev != outputs[0]) {
        relaxed = false;
    }

// Eq. 186
    local0 = memo[21];
    local1 = inputs[0];
    local2 = outputs[0];
    val = ((((local0 & _BV(6)) && (local0 & _BV(5))) && (local1 & _BV(0))) && !(local2 & _BV(1)));
    prev = outputs[0];
    val ? outputs[0] = prev | _BV(2) : outputs[0] = prev & ~_BV(2);
    if (prev != outputs[0]) {
        relaxed = false;
    }

// Eq. 187
    local0 = memo[22];
    local1 = outputs[2];
    val = ((local0 & _BV(7)) && (local1 & _BV(1))) || (local0 & _BV(5));
    prev = memo[21];
    val ? memo[21] = prev | _BV(6) : memo[21] = prev & ~_BV(6);
    if (prev != memo[21]) {
        relaxed = false;
    }

// Eq. 188
    local0 = memo[28];
    local1 = inputs[5];
    local2 = memo[23];
    val = ((((local0 & _BV(3)) || (local1 & _BV(4))) && !(local2 & _BV(0))) && (local1 & _BV(2)));
    prev = memo[28];
    val ? memo[28] = prev | _BV(3) : memo[28] = prev & ~_BV(3);
    if (prev != memo[28]) {
        relaxed = false;
    }

// Eq. 189
    local0 = memo[21];
    val = (local0 & _BV(4)) || (local0 & _BV(5));
    prev = memo[22];
    val ? memo[22] = prev | _BV(3) : memo[22] = prev & ~_BV(3);
    if (prev != memo[22]) {
        relaxed = false;
    }

// Eq. 190
    local0 = memo[28];
    val = ((local0 & _BV(5)) && !(local0 & _BV(3)));
    prev = memo[22];
    val ? memo[22] = prev | _BV(7) : memo[22] = prev & ~_BV(7);
    if (prev != memo[22]) {
        relaxed = false;
    }

// Eq. 191
    local0 = memo[22];
    local1 = outputs[2];
    local2 = inputs[7];
    val = (!(local0 & _BV(5)) && (local0 & _BV(7))) || ((local1 & _BV(1)) && (local2 & _BV(1)));
    prev = outputs[2];
    val ? outputs[2] = prev | _BV(1) : outputs[2] = prev & ~_BV(1);
    if (prev != outputs[2]) {
        relaxed = false;
    }

// Eq. 192
    local0 = outputs[2];
    local1 = memo[22];
    local2 = inputs[7];
    val = ((!(local0 & _BV(1)) && !(local1 & _BV(7))) && (local1 & _BV(3))) || ((local2 & _BV(1)) && (local1 & _BV(5)));
    prev = memo[22];
    val ? memo[22] = prev | _BV(5) : memo[22] = prev & ~_BV(5);
    if (prev != memo[22]) {
        relaxed = false;
    }

// Eq. 193
    local0 = inputs[7];
    val = !(local0 & _BV(1));
    prev = memo[28];
    val ? memo[28] = prev | _BV(4) : memo[28] = prev & ~_BV(4);
    if (prev != memo[28]) {
        relaxed = false;
    }

// Eq. 194
    local0 = memo[28];
    val = ((local0 & _BV(4)) && !(local0 & _BV(6)));
    prev = memo[22];
    val ? memo[22] = prev | _BV(6) : memo[22] = prev & ~_BV(6);
    if (prev != memo[22]) {
        relaxed = false;
    }

// Eq. 195
    local0 = inputs[7];
    local1 = memo[28];
    local2 = memo[22];
    val = (((!(local0 & _BV(1)) && !(local1 & _BV(6))) && !(local1 & _BV(3))) && !(local2 & _BV(3))) || (((local0 & _BV(2)) || (local0 & _BV(3))) && !(local0 & _BV(1)));
    prev = outputs[1];
    val ? outputs[1] = prev | _BV(3) : outputs[1] = prev & ~_BV(3);
    if (prev != outputs[1]) {
        relaxed = false;
    }

// Eq. 196
    local0 = memo[28];
    local1 = outputs[1];
    val = ((local0 & _BV(6)) && !(local1 & _BV(3)));
    prev = memo[23];
    val ? memo[23] = prev | _BV(3) : memo[23] = prev & ~_BV(3);
    if (prev != memo[23]) {
        relaxed = false;
    }

// Eq. 197
    local0 = memo[21];
    val = ((local0 & _BV(0)) && !(local0 & _BV(5)));
    prev = memo[23];
    val ? memo[23] = prev | _BV(4) : memo[23] = prev & ~_BV(4);
    if (prev != memo[23]) {
        relaxed = false;
    }

// Eq. 198
    local0 = memo[21];
    val = ((local0 & _BV(1)) && !(local0 & _BV(4)));
    prev = memo[23];
    val ? memo[23] = prev | _BV(5) : memo[23] = prev & ~_BV(5);
    if (prev != memo[23]) {
        relaxed = false;
    }

// Eq. 199
    local0 = inputs[7];
    local1 = memo[28];
    val = ((((local0 & _BV(2)) && (local0 & _BV(5))) && !(local0 & _BV(0))) && (local1 & _BV(7)));
    prev = outputs[1];
    val ? outputs[1] = prev | _BV(4) : outputs[1] = prev & ~_BV(4);
    if (prev != outputs[1]) {
        relaxed = false;
    }

// Eq. 200
    local0 = inputs[7];
    local1 = memo[28];
    val = ((((local0 & _BV(3)) && (local0 & _BV(5))) && (local1 & _BV(6))) && (local1 & _BV(7)));
    prev = outputs[1];
    val ? outputs[1] = prev | _BV(5) : outputs[1] = prev & ~_BV(5);
    if (prev != outputs[1]) {
        relaxed = false;
    }

// Eq. 201
    local0 = inputs[7];
    val = !(local0 & _BV(0));
    prev = outputs[2];
    val ? outputs[2] = prev | _BV(2) : outputs[2] = prev & ~_BV(2);
    if (prev != outputs[2]) {
        relaxed = false;
    }

// Eq. 202
    local0 = memo[28];
    local1 = inputs[7];
    local2 = memo[22];
    local3 = outputs[0];
    local4 = memo[0];
    local5 = outputs[1];
    local6 = memo[1];
    local7 = inputs[0];
    val = ((!(local0 & _BV(3)) && (local1 & _BV(6))) && ((local2 & _BV(3)) || ((((((local1 & _BV(1)) && !(local3 & _BV(1))) && !(local3 & _BV(2))) && ((local4 & _BV(3)) || (local4 & _BV(4)) || (local5 & _BV(6)))) && (local6 & _BV(6))) && (local7 & _BV(6)))));
    prev = outputs[1];
    val ? outputs[1] = prev | _BV(6) : outputs[1] = prev & ~_BV(6);
    if (prev != outputs[1]) {
        relaxed = false;
    }

// Eq. 203
    local0 = memo[18];
    local1 = memo[20];
    local2 = memo[27];
    local3 = inputs[7];
    local4 = outputs[0];
    local5 = inputs[0];
    local6 = memo[1];
    local7 = inputs[5];
    local8 = memo[5];
    val = (((((((local0 & _BV(4)) || (local1 & _BV(4)) || (!(local0 & _BV(1)) && (local2 & _BV(7))) || (((local1 & _BV(2)) && (local3 & _BV(1))) && ((local4 & _BV(1)) || (local4 & _BV(2))))) && (local5 & _BV(7))) && (local6 & _BV(6))) || ((local7 & _BV(3)) && (local7 & _BV(5)))) && !(local4 & _BV(4))) && !(local8 & _BV(7)));
    prev = outputs[0];
    val ? outputs[0] = prev | _BV(3) : outputs[0] = prev & ~_BV(3);
    if (prev != outputs[0]) {
        relaxed = false;
    }

// Eq. 204
    local0 = memo[20];
    local1 = memo[18];
    local2 = inputs[5];
    local3 = memo[1];
    local4 = inputs[1];
    local5 = outputs[0];
    local6 = inputs[0];
    local7 = memo[5];
    val = (((((((((local0 & _BV(3)) && !(local1 & _BV(4))) && (local2 & _BV(6))) && (local2 & _BV(7))) && (local3 & _BV(6))) || ((local2 & _BV(3)) && (local4 & _BV(4)))) && !(local5 & _BV(3))) && !(local6 & _BV(6))) && !(local7 & _BV(7)));
    prev = outputs[0];
    val ? outputs[0] = prev | _BV(4) : outputs[0] = prev & ~_BV(4);
    if (prev != outputs[0]) {
        relaxed = false;
    }

// Eq. 205
    local0 = outputs[0];
    local1 = memo[29];
    local2 = inputs[0];
    local3 = inputs[5];
    val = (((local0 & _BV(3)) || (local0 & _BV(4)) || (((local1 & _BV(1)) && (local2 & _BV(7))) && (local3 & _BV(7)))) && !(local3 & _BV(3)));
    prev = memo[29];
    val ? memo[29] = prev | _BV(1) : memo[29] = prev & ~_BV(1);
    if (prev != memo[29]) {
        relaxed = false;
    }

// Eq. 206
    local0 = memo[23];
    local1 = memo[20];
    val = (((local0 & _BV(3)) || (local0 & _BV(1))) && (local1 & _BV(3)));
    prev = memo[26];
    val ? memo[26] = prev | _BV(0) : memo[26] = prev & ~_BV(0);
    if (prev != memo[26]) {
        relaxed = false;
    }

// Eq. 207
    local0 = memo[23];
    local1 = memo[22];
    val = ((local0 & _BV(1)) && (local1 & _BV(7)));
    prev = memo[20];
    val ? memo[20] = prev | _BV(5) : memo[20] = prev & ~_BV(5);
    if (prev != memo[20]) {
        relaxed = false;
    }

// Eq. 208
    local0 = inputs[6];
    local1 = memo[2];
    local2 = memo[6];
    val = (((local0 & _BV(6)) || (local1 & _BV(6)) || !(local2 & _BV(0))) && (local1 & _BV(1)));
    prev = memo[25];
    val ? memo[25] = prev | _BV(2) : memo[25] = prev & ~_BV(2);
    if (prev != memo[25]) {
        relaxed = false;
    }

// Eq. 209
    local0 = memo[2];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(2)) && ((local1 & _BV(6)) || (local0 & _BV(6)) || !(local2 & _BV(0))));
    prev = memo[24];
    val ? memo[24] = prev | _BV(0) : memo[24] = prev & ~_BV(0);
    if (prev != memo[24]) {
        relaxed = false;
    }

// Eq. 210
    local0 = memo[2];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(4)) && ((local1 & _BV(6)) || (local0 & _BV(7)) || !(local2 & _BV(1))));
    prev = memo[24];
    val ? memo[24] = prev | _BV(1) : memo[24] = prev & ~_BV(1);
    if (prev != memo[24]) {
        relaxed = false;
    }

// Eq. 211
    local0 = memo[2];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(5)) && ((local1 & _BV(6)) || (local0 & _BV(7)) || !(local2 & _BV(1))));
    prev = memo[24];
    val ? memo[24] = prev | _BV(2) : memo[24] = prev & ~_BV(2);
    if (prev != memo[24]) {
        relaxed = false;
    }

// Eq. 212
    local0 = memo[7];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(0)) && ((local1 & _BV(6)) || (local2 & _BV(7)) || !(local2 & _BV(2))));
    prev = memo[25];
    val ? memo[25] = prev | _BV(3) : memo[25] = prev & ~_BV(3);
    if (prev != memo[25]) {
        relaxed = false;
    }

// Eq. 213
    local0 = memo[7];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(1)) && ((local1 & _BV(6)) || (local2 & _BV(7)) || !(local2 & _BV(2))));
    prev = memo[24];
    val ? memo[24] = prev | _BV(3) : memo[24] = prev & ~_BV(3);
    if (prev != memo[24]) {
        relaxed = false;
    }

// Eq. 214
    local0 = memo[3];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(2)) && ((local1 & _BV(6)) || (local0 & _BV(4)) || !(local2 & _BV(3))));
    prev = memo[25];
    val ? memo[25] = prev | _BV(4) : memo[25] = prev & ~_BV(4);
    if (prev != memo[25]) {
        relaxed = false;
    }

// Eq. 215
    local0 = memo[3];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(3)) && ((local1 & _BV(6)) || (local0 & _BV(4)) || !(local2 & _BV(3))));
    prev = memo[24];
    val ? memo[24] = prev | _BV(4) : memo[24] = prev & ~_BV(4);
    if (prev != memo[24]) {
        relaxed = false;
    }

// Eq. 216
    local0 = memo[5];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(0)) && ((local1 & _BV(6)) || (local0 & _BV(5)) || !(local2 & _BV(4))));
    prev = memo[24];
    val ? memo[24] = prev | _BV(5) : memo[24] = prev & ~_BV(5);
    if (prev != memo[24]) {
        relaxed = false;
    }

// Eq. 217
    local0 = memo[5];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(1)) && ((local1 & _BV(6)) || (local0 & _BV(5)) || !(local2 & _BV(4))));
    prev = memo[24];
    val ? memo[24] = prev | _BV(6) : memo[24] = prev & ~_BV(6);
    if (prev != memo[24]) {
        relaxed = false;
    }

// Eq. 218
    local0 = memo[5];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(2)) && ((local1 & _BV(6)) || (local0 & _BV(6)) || !(local2 & _BV(5))));
    prev = memo[25];
    val ? memo[25] = prev | _BV(5) : memo[25] = prev & ~_BV(5);
    if (prev != memo[25]) {
        relaxed = false;
    }

// Eq. 219
    local0 = memo[5];
    local1 = inputs[6];
    local2 = memo[6];
    val = ((local0 & _BV(3)) && ((local1 & _BV(6)) || (local0 & _BV(6)) || !(local2 & _BV(5))));
    prev = memo[24];
    val ? memo[24] = prev | _BV(7) : memo[24] = prev & ~_BV(7);
    if (prev != memo[24]) {
        relaxed = false;
    }

// Eq. 220
    local0 = memo[25];
    local1 = memo[24];
    val = (local0 & _BV(2)) || (local1 & _BV(1)) || (local0 & _BV(3)) || (local0 & _BV(4)) || (local1 & _BV(5)) || (local0 & _BV(5));
    prev = memo[25];
    val ? memo[25] = prev | _BV(6) : memo[25] = prev & ~_BV(6);
    if (prev != memo[25]) {
        relaxed = false;
    }

// Eq. 221
    local0 = memo[24];
    val = (local0 & _BV(0)) || (local0 & _BV(2)) || (local0 & _BV(3)) || (local0 & _BV(4)) || (local0 & _BV(6)) || (local0 & _BV(7));
    prev = memo[25];
    val ? memo[25] = prev | _BV(7) : memo[25] = prev & ~_BV(7);
    if (prev != memo[25]) {
        relaxed = false;
    }

// Eq. 222
    local0 = memo[26];
    local1 = inputs[0];
    local2 = outputs[0];
    val = ((((((local0 & _BV(4)) || (local0 & _BV(0))) && ((local1 & _BV(2)) || (local1 & _BV(3)))) && !(local0 & _BV(2))) && !(local2 & _BV(3))) && !(local2 & _BV(4)));
    prev = memo[16];
    val ? memo[16] = prev | _BV(7) : memo[16] = prev & ~_BV(7);
    if (prev != memo[16]) {
        relaxed = false;
    }

// Eq. 223
    local0 = memo[20];
    local1 = memo[26];
    local2 = inputs[7];
    local3 = inputs[6];
    local4 = memo[1];
    local5 = inputs[0];
    local6 = outputs[0];
    val = ((((((local0 & _BV(6)) || ((local1 & _BV(3)) && !(local0 & _BV(4))) || (((((local2 & _BV(1)) && !(local3 & _BV(3))) && !(local3 & _BV(4))) && !(local3 & _BV(5))) && (local4 & _BV(7)))) && ((local5 & _BV(2)) || (local5 & _BV(3)))) && !(local1 & _BV(2))) && !(local6 & _BV(3))) && !(local6 & _BV(4)));
    prev = memo[26];
    val ? memo[26] = prev | _BV(1) : memo[26] = prev & ~_BV(1);
    if (prev != memo[26]) {
        relaxed = false;
    }

// Eq. 224
    local0 = outputs[2];
    val = (!(local0 & _BV(3)) && !(local0 & _BV(4)));
    prev = memo[26];
    val ? memo[26] = prev | _BV(2) : memo[26] = prev & ~_BV(2);
    if (prev != memo[26]) {
        relaxed = false;
    }

// Eq. 225
    local0 = outputs[2];
    local1 = inputs[5];
    local2 = memo[18];
    local3 = memo[27];
    val = ((!(local0 & _BV(4)) && (((local1 & _BV(5)) && !(local1 & _BV(3))) || (((local2 & _BV(7)) && (local3 & _BV(2))) && ((local0 & _BV(3)) || (local3 & _BV(7)))))) && (local1 & _BV(2)));
    prev = outputs[2];
    val ? outputs[2] = prev | _BV(3) : outputs[2] = prev & ~_BV(3);
    if (prev != outputs[2]) {
        relaxed = false;
    }

// Eq. 226
    local0 = outputs[2];
    local1 = memo[27];
    local2 = memo[18];
    val = (((((local0 & _BV(4)) || (local1 & _BV(7))) && (local2 & _BV(6))) && !(local0 & _BV(3))) && (local1 & _BV(1)));
    prev = outputs[2];
    val ? outputs[2] = prev | _BV(4) : outputs[2] = prev & ~_BV(4);
    if (prev != outputs[2]) {
        relaxed = false;
    }

// Eq. 227
    local0 = memo[18];
    val = (local0 & _BV(6));
    prev = memo[27];
    val ? memo[27] = prev | _BV(1) : memo[27] = prev & ~_BV(1);
    if (prev != memo[27]) {
        relaxed = false;
    }

// Eq. 228
    local0 = memo[18];
    val = (local0 & _BV(7));
    prev = memo[27];
    val ? memo[27] = prev | _BV(2) : memo[27] = prev & ~_BV(2);
    if (prev != memo[27]) {
        relaxed = false;
    }

// Eq. 229
    local0 = memo[27];
    val = (local0 & _BV(3));
    prev = memo[26];
    val ? memo[26] = prev | _BV(3) : memo[26] = prev & ~_BV(3);
    if (prev != memo[26]) {
        relaxed = false;
    }

// Eq. 230
    local0 = memo[26];
    local1 = inputs[5];
    local2 = memo[0];
    val = ((local0 & _BV(0)) && (!(local1 & _BV(7)) || !(local1 & _BV(6)) || (local2 & _BV(2))));
    prev = memo[27];
    val ? memo[27] = prev | _BV(3) : memo[27] = prev & ~_BV(3);
    if (prev != memo[27]) {
        relaxed = false;
    }

// Eq. 231
    local0 = memo[20];
    val = (local0 & _BV(6));
    prev = memo[26];
    val ? memo[26] = prev | _BV(4) : memo[26] = prev & ~_BV(4);
    if (prev != memo[26]) {
        relaxed = false;
    }

// Eq. 232
    local0 = memo[20];
    local1 = inputs[1];
    local2 = memo[6];
    local3 = inputs[6];
    local4 = inputs[7];
    local5 = outputs[0];
    local6 = inputs[0];
    val = ((((local0 & _BV(2)) && !(local1 & _BV(7))) && (local2 & _BV(6))) && (local0 & _BV(1))) || (((((local0 & _BV(2)) && ((local3 & _BV(6)) || ((local4 & _BV(1)) && ((local5 & _BV(1)) || (local5 & _BV(2)))))) || (local0 & _BV(4))) && (local0 & _BV(5))) && !(local6 & _BV(7)));
    prev = memo[20];
    val ? memo[20] = prev | _BV(6) : memo[20] = prev & ~_BV(6);
    if (prev != memo[20]) {
        relaxed = false;
    }

// Eq. 233
    local0 = inputs[1];
    local1 = memo[20];
    local2 = outputs[2];
    local3 = outputs[1];
    val = ((((!(local0 & _BV(7)) || !(local1 & _BV(2))) && !(local2 & _BV(6))) && !(local1 & _BV(3))) && !(local3 & _BV(7)));
    prev = outputs[2];
    val ? outputs[2] = prev | _BV(5) : outputs[2] = prev & ~_BV(5);
    if (prev != outputs[2]) {
        relaxed = false;
    }

// Eq. 234
    local0 = memo[20];
    local1 = outputs[2];
    val = (((local0 & _BV(3)) && !(local1 & _BV(6))) && !(local1 & _BV(5)));
    prev = outputs[1];
    val ? outputs[1] = prev | _BV(7) : outputs[1] = prev & ~_BV(7);
    if (prev != outputs[1]) {
        relaxed = false;
    }

// Eq. 235
    local0 = inputs[1];
    local1 = outputs[2];
    local2 = outputs[1];
    local3 = memo[20];
    local4 = memo[0];
    val = ((((local0 & _BV(7)) && !(local1 & _BV(5))) && !(local2 & _BV(7))) && ((local3 & _BV(2)) || (local4 & _BV(6))));
    prev = outputs[2];
    val ? outputs[2] = prev | _BV(6) : outputs[2] = prev & ~_BV(6);
    if (prev != outputs[2]) {
        relaxed = false;
    }

// Eq. 236
    local0 = inputs[6];
    local1 = memo[18];
    local2 = memo[20];
    local3 = memo[26];
    val = ((((local0 & _BV(7)) && (local1 & _BV(5))) && !(local2 & _BV(3))) && !(local3 & _BV(2)));
    prev = outputs[2];
    val ? outputs[2] = prev | _BV(7) : outputs[2] = prev & ~_BV(7);
    if (prev != outputs[2]) {
        relaxed = false;
    }

// Eq. 237
    local0 = inputs[1];
    local1 = inputs[0];
    val = (((local0 & _BV(5)) || (local0 & _BV(6))) && (local1 & _BV(2))) || (local1 & _BV(3));
    prev = memo[26];
    val ? memo[26] = prev | _BV(5) : memo[26] = prev & ~_BV(5);
    if (prev != memo[26]) {
        relaxed = false;
    }

// Eq. 238
    local0 = memo[26];
    local1 = outputs[0];
    local2 = memo[18];
    val = ((((((local0 & _BV(5)) && ((local1 & _BV(5)) || (local0 & _BV(7)))) && (local0 & _BV(1))) && (local2 & _BV(5))) && !(local1 & _BV(6))) && !(local1 & _BV(7)));
    prev = outputs[0];
    val ? outputs[0] = prev | _BV(5) : outputs[0] = prev & ~_BV(5);
    if (prev != outputs[0]) {
        relaxed = false;
    }

// Eq. 239
    local0 = memo[26];
    local1 = outputs[0];
    local2 = memo[0];
    val = ((((((local0 & _BV(5)) && ((local1 & _BV(6)) || (local0 & _BV(7)))) && (local0 & _BV(1))) && (local2 & _BV(7))) && !(local1 & _BV(7))) && !(local1 & _BV(5)));
    prev = outputs[0];
    val ? outputs[0] = prev | _BV(6) : outputs[0] = prev & ~_BV(6);
    if (prev != outputs[0]) {
        relaxed = false;
    }

// Eq. 240
    local0 = memo[26];
    local1 = outputs[0];
    local2 = memo[18];
    val = ((((((local0 & _BV(5)) && ((local1 & _BV(7)) || (local0 & _BV(7)))) && (local0 & _BV(1))) && (local2 & _BV(4))) && !(local1 & _BV(5))) && !(local1 & _BV(6)));
    prev = outputs[0];
    val ? outputs[0] = prev | _BV(7) : outputs[0] = prev & ~_BV(7);
    if (prev != outputs[0]) {
        relaxed = false;
    }

// Eq. 241
    local0 = outputs[0];
    local1 = inputs[0];
    val = ((!(local0 & _BV(5)) || !(local1 & _BV(0))) && (local1 & _BV(1)));
    prev = outputs[3];
    val ? outputs[3] = prev | _BV(2) : outputs[3] = prev & ~_BV(2);
    if (prev != outputs[3]) {
        relaxed = false;
    }

// Eq. 242
    local0 = outputs[0];
    local1 = inputs[0];
    val = ((!(local0 & _BV(6)) || !(local1 & _BV(0))) && (local1 & _BV(4)));
    prev = outputs[3];
    val ? outputs[3] = prev | _BV(3) : outputs[3] = prev & ~_BV(3);
    if (prev != outputs[3]) {
        relaxed = false;
    }

// Eq. 243
    local0 = outputs[0];
    local1 = inputs[0];
    val = ((!(local0 & _BV(7)) || !(local1 & _BV(0))) && (local1 & _BV(5)));
    prev = outputs[3];
    val ? outputs[3] = prev | _BV(4) : outputs[3] = prev & ~_BV(4);
    if (prev != outputs[3]) {
        relaxed = false;
    }

// Eq. 244
    local0 = outputs[0];
    local1 = memo[26];
    local2 = inputs[5];
    val = ((((!(local0 & _BV(7)) && !(local0 & _BV(6))) && !(local0 & _BV(5))) && !(local1 & _BV(2))) && !(local2 & _BV(5)));
    prev = memo[26];
    val ? memo[26] = prev | _BV(6) : memo[26] = prev & ~_BV(6);
    if (prev != memo[26]) {
        relaxed = false;
    }

// Eq. 245
    local0 = memo[0];
    local1 = inputs[2];
    local2 = inputs[0];
    local3 = memo[18];
    val = ((((local0 & _BV(4)) && !(local1 & _BV(6))) || ((local0 & _BV(3)) && !(local1 & _BV(7)))) && (local2 & _BV(2))) || (!(local3 & _BV(5)) && !(local0 & _BV(0)));
    prev = outputs[3];
    val ? outputs[3] = prev | _BV(5) : outputs[3] = prev & ~_BV(5);
    if (prev != outputs[3]) {
        relaxed = false;
    }

// Eq. 246
    local0 = memo[0];
    local1 = inputs[3];
    local2 = inputs[0];
    val = ((((local0 & _BV(4)) && !(local1 & _BV(6))) || ((local0 & _BV(3)) && !(local1 & _BV(7)))) && (local2 & _BV(2))) || (!(local0 & _BV(7)) && !(local0 & _BV(1)));
    prev = outputs[3];
    val ? outputs[3] = prev | _BV(6) : outputs[3] = prev & ~_BV(6);
    if (prev != outputs[3]) {
        relaxed = false;
    }

// Eq. 247
    local0 = memo[0];
    local1 = inputs[4];
    local2 = inputs[0];
    local3 = memo[18];
    val = ((((local0 & _BV(3)) && !(local1 & _BV(6))) || ((local0 & _BV(4)) && !(local1 & _BV(7)))) && (local2 & _BV(2))) || (!(local3 & _BV(4)) && !(local0 & _BV(2)));
    prev = outputs[3];
    val ? outputs[3] = prev | _BV(7) : outputs[3] = prev & ~_BV(7);
    if (prev != outputs[3]) {
        relaxed = false;
    }

// Eq. 248
    local0 = inputs[6];
    local1 = memo[6];
    local2 = memo[2];
    local3 = inputs[0];
    val = (((local0 & _BV(6)) && (((local1 & _BV(0)) && ((local2 & _BV(1)) || (local2 & _BV(2)))) || ((local1 & _BV(1)) && ((local2 & _BV(4)) || (local2 & _BV(5)))))) && (local3 & _BV(3)));
    prev = memo[27];
    val ? memo[27] = prev | _BV(4) : memo[27] = prev & ~_BV(4);
    if (prev != memo[27]) {
        relaxed = false;
    }

// Eq. 249
    local0 = inputs[6];
    local1 = memo[6];
    local2 = memo[7];
    local3 = memo[3];
    local4 = inputs[0];
    val = (((local0 & _BV(6)) && (((local1 & _BV(2)) && ((local2 & _BV(0)) || (local2 & _BV(1)))) || ((local1 & _BV(3)) && ((local3 & _BV(2)) || (local3 & _BV(3)))))) && (local4 & _BV(3)));
    prev = memo[27];
    val ? memo[27] = prev | _BV(5) : memo[27] = prev & ~_BV(5);
    if (prev != memo[27]) {
        relaxed = false;
    }

// Eq. 250
    local0 = inputs[6];
    local1 = memo[6];
    local2 = memo[5];
    local3 = inputs[0];
    val = (((local0 & _BV(6)) && (((local1 & _BV(4)) && ((local2 & _BV(0)) || (local2 & _BV(1)))) || ((local1 & _BV(5)) && ((local2 & _BV(2)) || (local2 & _BV(3)))))) && (local3 & _BV(3)));
    prev = memo[27];
    val ? memo[27] = prev | _BV(6) : memo[27] = prev & ~_BV(6);
    if (prev != memo[27]) {
        relaxed = false;
    }

// Eq. 251
    local0 = memo[8];
    val = !(local0 & _BV(7));
    prev = memo[21];
    val ? memo[21] = prev | _BV(7) : memo[21] = prev & ~_BV(7);
    if (prev != memo[21]) {
        relaxed = false;
    }

// Eq. 252
    local0 = memo[20];
    val = (local0 & _BV(7));
    prev = memo[15];
    val ? memo[15] = prev | _BV(7) : memo[15] = prev & ~_BV(7);
    if (prev != memo[15]) {
        relaxed = false;
    }

// Eq. 253
    local0 = inputs[6];
    local1 = memo[18];
    local2 = memo[21];
    local3 = memo[0];
    local4 = memo[26];
    val = !(local0 & _BV(4)) || ((!(local0 & _BV(7)) || ((!(local1 & _BV(7)) || !(local2 & _BV(4))) && (!(local1 & _BV(6)) || !(local2 & _BV(5))))) && ((local0 & _BV(7)) || ((!(local1 & _BV(6)) || !(local2 & _BV(4))) && (!(local1 & _BV(7)) || !(local2 & _BV(5)))))) || (local3 & _BV(3)) || (local3 & _BV(4)) || (local4 & _BV(2));
    prev = memo[4];
    val ? memo[4] = prev | _BV(7) : memo[4] = prev & ~_BV(7);
    if (prev != memo[4]) {
        relaxed = false;
    }

// Eq. 254
    local0 = memo[4];
    local1 = inputs[2];
    local2 = memo[0];
    local3 = inputs[3];
    local4 = inputs[4];
    local5 = inputs[0];
    local6 = inputs[6];
    val = (((!(local0 & _BV(7)) && ((((local1 & _BV(7)) && (local1 & _BV(6))) && (local2 & _BV(0))) || (((local3 & _BV(7)) && (local3 & _BV(6))) && (local2 & _BV(1))) || (((local4 & _BV(7)) && (local4 & _BV(6))) && (local2 & _BV(2))))) && (local5 & _BV(0))) && (local6 & _BV(6)));
    prev = memo[11];
    val ? memo[11] = prev | _BV(7) : memo[11] = prev & ~_BV(7);
    if (prev != memo[11]) {
        relaxed = false;
    }

// Eq. 255
    local0 = inputs[7];
    local1 = outputs[1];
    val = ((local0 & _BV(3)) && !(local1 & _BV(4))) || ((local0 & _BV(2)) && !(local1 & _BV(5)));
    prev = memo[13];
    val ? memo[13] = prev | _BV(7) : memo[13] = prev & ~_BV(7);
    if (prev != memo[13]) {
        relaxed = false;
    }

// Eq. 256
    local0 = inputs[0];
    local1 = inputs[7];
    local2 = memo[26];
    local3 = memo[20];
    local4 = memo[23];
    local5 = inputs[6];
    local6 = outputs[0];
    val = (((local0 & _BV(2)) || (local0 & _BV(3))) && (((local1 & _BV(4)) && ((((local2 & _BV(6)) || (local2 & _BV(2))) && (local3 & _BV(3))) || (local3 & _BV(0)))) || ((local4 & _BV(0)) && !(local2 & _BV(6))))) || ((local3 & _BV(2)) && ((local5 & _BV(6)) || ((local1 & _BV(1)) && ((local6 & _BV(1)) || (local6 & _BV(2))))));
    prev = memo[23];
    val ? memo[23] = prev | _BV(6) : memo[23] = prev & ~_BV(6);
    if (prev != memo[23]) {
        relaxed = false;
    }

    if (UpdateTimers(1))
        relaxed = false;
    return !relaxed;
}

void UpdateMachine(uint8_t* inputBytes, uint8_t* outputBytes, uint8_t* memoBytes)
{
    memcpy(&inputs[0], inputBytes, sizeof(inputs));

    int iterations = 0;
    while (Update()) {
//         if (iterations == 5)
//             break;
        iterations++;
    }

    memcpy(outputBytes, &outputs[0], sizeof(outputs));
	
	if (memoBytes != NULL)
	{
		memcpy(memoBytes, &memo[0], sizeof(memo));
	}
}