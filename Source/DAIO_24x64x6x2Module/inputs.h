#pragma once

#include "common.h"
#include "kplc/IOStateFrame.h"

int8_t ProcessInputs(bool forced = false);
int8_t handle_KPLC_IOStateFrame_Response(CanardRxTransfer* transfer);
uint8_t InitializeInputs();