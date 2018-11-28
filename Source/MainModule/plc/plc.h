#pragma once

#include "main_config.h"
#include <avr/io.h>
#include "uavcan/kplc/IOState.h"
#include "mill.h"

#define PLC_SLAVE_NODES_LEN 2
#define PLC_PULL_ITEMS_MAX 1

struct PlcSlaveNodeState
{
	uint8_t NodeId;
	uint32_t LastStatusUpdateTime;
};

struct PlcPullItem
{
	uint8_t NodeId;
	uavcan_kplc_IOStateRequest State;
};

extern PlcSlaveNodeState g_plcSlaveNodeState[PLC_SLAVE_NODES_LEN];

uint8_t PlcInitialize();

int8_t PlcPush(uint8_t nodeId, uavcan_kplc_IOStateRequest state);

int8_t PlcUpdate();

int8_t PlcPull(PlcPullItem* buffer, uint8_t* length);