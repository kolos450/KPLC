#include "plc.h"

#define NODEID_DI 102
#define NODEID_DO 101

PlcSlaveNodeState g_plcSlaveNodeState[PLC_SLAVE_NODES_LEN]
{
	{
		.NodeId = NODEID_DI,
		.LastStatusUpdateTime = 0,
	},
	{
		.NodeId = NODEID_DO,
		.LastStatusUpdateTime = 0,
	},
};

uint8_t _state[3];
uint8_t _isDirty = false;

uint8_t _inputs[8];
uint8_t _outputs[4];

uint8_t PlcInitialize()
{
	_inputs[5] |= 2; // Set E077 true: no limit switches S10, S20, S30.
	return 0;
}

int8_t PlcUpdate()
{
	UpdateMachine(&_inputs[0], &_outputs[0]);
	return 0;
}

int8_t PlcPush(uint8_t nodeId, uavcan_kplc_IOStateRequest state)
{
	if (nodeId == NODEID_DI)
	{
		for (uint8_t i = 0; i < ARRAY_SIZE(_state); i++)
		{
			uint8_t val = state.state[i];
			uint8_t inv = ~val;
			if (inv != state.state_inv[i])
				return -1;
			_state[i] = val;
		}
		_isDirty = true;
	}
	
	return 0;
}

int8_t PlcPull(PlcPullItem* buffer, uint8_t* length)
{	
	if (_isDirty)
	{
		*length = 1;
		
		if (buffer != NULL)
		{
			buffer[0].NodeId = NODEID_DO;
			for (uint8_t i = 0; i < ARRAY_SIZE(_state); i++)
			{
				buffer[0].State.state[i] = _state[i];
				buffer[0].State.state_inv[i] = ~_state[i];
			}
			
			_isDirty = false;
		}
	}
	
	return 0;
}