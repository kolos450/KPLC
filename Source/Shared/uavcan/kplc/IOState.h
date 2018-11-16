/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: C:\Sources\UAVCAN-dsdl\uavcan\kplc\200.IOState.uavcan
 */

#ifndef __UAVCAN_KPLC_IOSTATE
#define __UAVCAN_KPLC_IOSTATE

#include <stdint.h>
#include "canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
#
# KPLC IO State
#

uint8[3] state
uint8[3] state_inv

---

uint8 STATUS_OK = 0
uint8 STATUS_ERROR_INCONSISTENCY = 1
uint8 STATUS_ERROR_UNKNOWN = 2

uint8 status
******************************************************************************/

/********************* DSDL signature source definition ***********************
uavcan.kplc.IOState
saturated uint8[3] state
saturated uint8[3] state_inv
---
saturated uint8 status
******************************************************************************/

#define UAVCAN_KPLC_IOSTATE_ID                             200
#define UAVCAN_KPLC_IOSTATE_NAME                           "uavcan.kplc.IOState"
#define UAVCAN_KPLC_IOSTATE_SIGNATURE                      (0xF1C19C93F05888B9ULL)

#define UAVCAN_KPLC_IOSTATE_REQUEST_MAX_SIZE               ((48 + 7)/8)

// Constants

#define UAVCAN_KPLC_IOSTATE_REQUEST_STATE_LENGTH                                         3
#define UAVCAN_KPLC_IOSTATE_REQUEST_STATE_INV_LENGTH                                     3

typedef struct
{
    // FieldTypes
    uint8_t    state[3];                      // Static Array 8bit[3] max items
    uint8_t    state_inv[3];                  // Static Array 8bit[3] max items

} uavcan_kplc_IOStateRequest;

extern
uint32_t uavcan_kplc_IOStateRequest_encode(uavcan_kplc_IOStateRequest* source, void* msg_buf);

extern
int32_t uavcan_kplc_IOStateRequest_decode(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_kplc_IOStateRequest* dest, uint8_t** dyn_arr_buf);

extern
uint32_t uavcan_kplc_IOStateRequest_encode_internal(uavcan_kplc_IOStateRequest* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t uavcan_kplc_IOStateRequest_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_kplc_IOStateRequest* dest, uint8_t** dyn_arr_buf, int32_t offset, uint8_t tao);

#define UAVCAN_KPLC_IOSTATE_RESPONSE_MAX_SIZE              ((8 + 7)/8)

// Constants
#define UAVCAN_KPLC_IOSTATE_STATUS_OK                                         0 // 0
#define UAVCAN_KPLC_IOSTATE_STATUS_ERROR_INCONSISTENCY                        1 // 1
#define UAVCAN_KPLC_IOSTATE_STATUS_ERROR_UNKNOWN                              2 // 2

typedef struct
{
    // FieldTypes
    uint8_t    status;                        // bit len 8

} uavcan_kplc_IOStateResponse;

extern
uint32_t uavcan_kplc_IOStateResponse_encode(uavcan_kplc_IOStateResponse* source, void* msg_buf);

extern
int32_t uavcan_kplc_IOStateResponse_decode(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_kplc_IOStateResponse* dest, uint8_t** dyn_arr_buf);

extern
uint32_t uavcan_kplc_IOStateResponse_encode_internal(uavcan_kplc_IOStateResponse* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t uavcan_kplc_IOStateResponse_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_kplc_IOStateResponse* dest, uint8_t** dyn_arr_buf, int32_t offset, uint8_t tao);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __UAVCAN_KPLC_IOSTATE