/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: C:\Sources\UAVCAN-dsdl\uavcan\kplc\202.IOState_float16x2.uavcan
 */
#include "kplc\IOState_float16x2.h"
#include "canard.h"

#ifndef CANARD_INTERNAL_SATURATE
#define CANARD_INTERNAL_SATURATE(x, max) ( ((x) > max) ? max : ( (-(x) > max) ? (-max) : (x) ) );
#endif

#ifndef CANARD_INTERNAL_SATURATE_UNSIGNED
#define CANARD_INTERNAL_SATURATE_UNSIGNED(x, max) ( ((x) >= max) ? max : (x) );
#endif

#if defined(__GNUC__)
# define CANARD_MAYBE_UNUSED(x) x __attribute__((unused))
#else
# define CANARD_MAYBE_UNUSED(x) x
#endif

/**
  * @brief kplc_IOState_float16x2Request_encode_internal
  * @param source : pointer to source data struct
  * @param msg_buf: pointer to msg storage
  * @param offset: bit offset to msg storage
  * @param root_item: for detecting if TAO should be used
  * @retval returns new offset
  */
uint32_t kplc_IOState_float16x2Request_encode_internal(kplc_IOState_float16x2Request* source,
  void* msg_buf,
  uint32_t offset,
  uint8_t CANARD_MAYBE_UNUSED(root_item))
{
#ifndef CANARD_USE_FLOAT16_CAST
    uint16_t tmp_float = 0;
#else
    CANARD_USE_FLOAT16_CAST tmp_float = 0;
#endif

    // float16 special handling
#ifndef CANARD_USE_FLOAT16_CAST
    tmp_float = canardConvertNativeFloatToFloat16(source->field1);
#else
    tmp_float = (CANARD_USE_FLOAT16_CAST)source->field1;
#endif
    canardEncodeScalar(msg_buf, offset, 16, (void*)&tmp_float); // 32767
    offset += 16;

    // float16 special handling
#ifndef CANARD_USE_FLOAT16_CAST
    tmp_float = canardConvertNativeFloatToFloat16(source->field2);
#else
    tmp_float = (CANARD_USE_FLOAT16_CAST)source->field2;
#endif
    canardEncodeScalar(msg_buf, offset, 16, (void*)&tmp_float); // 32767
    offset += 16;

    return offset;
}

/**
  * @brief kplc_IOState_float16x2Request_encode
  * @param source : Pointer to source data struct
  * @param msg_buf: Pointer to msg storage
  * @retval returns message length as bytes
  */
uint32_t kplc_IOState_float16x2Request_encode(kplc_IOState_float16x2Request* source, void* msg_buf)
{
    uint32_t offset = 0;

    offset = kplc_IOState_float16x2Request_encode_internal(source, msg_buf, offset, 1);

    return (offset + 7 ) / 8;
}

/**
  * @brief kplc_IOState_float16x2Request_decode_internal
  * @param transfer: Pointer to CanardRxTransfer transfer
  * @param payload_len: Payload message length
  * @param dest: Pointer to destination struct
  * @param dyn_arr_buf: NULL or Pointer to memory storage to be used for dynamic arrays
  *                     kplc_IOState_float16x2Request dyn memory will point to dyn_arr_buf memory.
  *                     NULL will ignore dynamic arrays decoding.
  * @param offset: Call with 0, bit offset to msg storage
  * @retval new offset or ERROR value if < 0
  */
int32_t kplc_IOState_float16x2Request_decode_internal(
  const CanardRxTransfer* transfer,
  uint16_t CANARD_MAYBE_UNUSED(payload_len),
  kplc_IOState_float16x2Request* dest,
  uint8_t** CANARD_MAYBE_UNUSED(dyn_arr_buf),
  int32_t offset)
{
    int32_t ret = 0;
#ifndef CANARD_USE_FLOAT16_CAST
    uint16_t tmp_float = 0;
#else
    CANARD_USE_FLOAT16_CAST tmp_float = 0;
#endif

    // float16 special handling
    ret = canardDecodeScalar(transfer, (uint32_t)offset, 16, false, (void*)&tmp_float);

    if (ret != 16)
    {
        goto kplc_IOState_float16x2Request_error_exit;
    }
#ifndef CANARD_USE_FLOAT16_CAST
    dest->field1 = canardConvertFloat16ToNativeFloat(tmp_float);
#else
    dest->field1 = (float)tmp_float;
#endif
    offset += 16;

    // float16 special handling
    ret = canardDecodeScalar(transfer, (uint32_t)offset, 16, false, (void*)&tmp_float);

    if (ret != 16)
    {
        goto kplc_IOState_float16x2Request_error_exit;
    }
#ifndef CANARD_USE_FLOAT16_CAST
    dest->field2 = canardConvertFloat16ToNativeFloat(tmp_float);
#else
    dest->field2 = (float)tmp_float;
#endif
    offset += 16;
    return offset;

kplc_IOState_float16x2Request_error_exit:
    if (ret < 0)
    {
        return ret;
    }
    else
    {
        return -CANARD_ERROR_INTERNAL;
    }
}

/**
  * @brief kplc_IOState_float16x2Request_decode
  * @param transfer: Pointer to CanardRxTransfer transfer
  * @param payload_len: Payload message length
  * @param dest: Pointer to destination struct
  * @param dyn_arr_buf: NULL or Pointer to memory storage to be used for dynamic arrays
  *                     kplc_IOState_float16x2Request dyn memory will point to dyn_arr_buf memory.
  *                     NULL will ignore dynamic arrays decoding.
  * @retval offset or ERROR value if < 0
  */
int32_t kplc_IOState_float16x2Request_decode(const CanardRxTransfer* transfer,
  uint16_t payload_len,
  kplc_IOState_float16x2Request* dest,
  uint8_t** dyn_arr_buf)
{
    const int32_t offset = 0;
    int32_t ret = 0;

    // Clear the destination struct
    for (uint32_t c = 0; c < sizeof(kplc_IOState_float16x2Request); c++)
    {
        ((uint8_t*)dest)[c] = 0x00;
    }

    ret = kplc_IOState_float16x2Request_decode_internal(transfer, payload_len, dest, dyn_arr_buf, offset);

    return ret;
}

/**
  * @brief kplc_IOState_float16x2Response_encode_internal
  * @param source : pointer to source data struct
  * @param msg_buf: pointer to msg storage
  * @param offset: bit offset to msg storage
  * @param root_item: for detecting if TAO should be used
  * @retval returns new offset
  */
uint32_t kplc_IOState_float16x2Response_encode_internal(kplc_IOState_float16x2Response* source,
  void* msg_buf,
  uint32_t offset,
  uint8_t CANARD_MAYBE_UNUSED(root_item))
{
    canardEncodeScalar(msg_buf, offset, 8, (void*)&source->status); // 255
    offset += 8;

    return offset;
}

/**
  * @brief kplc_IOState_float16x2Response_encode
  * @param source : Pointer to source data struct
  * @param msg_buf: Pointer to msg storage
  * @retval returns message length as bytes
  */
uint32_t kplc_IOState_float16x2Response_encode(kplc_IOState_float16x2Response* source, void* msg_buf)
{
    uint32_t offset = 0;

    offset = kplc_IOState_float16x2Response_encode_internal(source, msg_buf, offset, 1);

    return (offset + 7 ) / 8;
}

/**
  * @brief kplc_IOState_float16x2Response_decode_internal
  * @param transfer: Pointer to CanardRxTransfer transfer
  * @param payload_len: Payload message length
  * @param dest: Pointer to destination struct
  * @param dyn_arr_buf: NULL or Pointer to memory storage to be used for dynamic arrays
  *                     kplc_IOState_float16x2Response dyn memory will point to dyn_arr_buf memory.
  *                     NULL will ignore dynamic arrays decoding.
  * @param offset: Call with 0, bit offset to msg storage
  * @retval new offset or ERROR value if < 0
  */
int32_t kplc_IOState_float16x2Response_decode_internal(
  const CanardRxTransfer* transfer,
  uint16_t CANARD_MAYBE_UNUSED(payload_len),
  kplc_IOState_float16x2Response* dest,
  uint8_t** CANARD_MAYBE_UNUSED(dyn_arr_buf),
  int32_t offset)
{
    int32_t ret = 0;

    ret = canardDecodeScalar(transfer, (uint32_t)offset, 8, false, (void*)&dest->status);
    if (ret != 8)
    {
        goto kplc_IOState_float16x2Response_error_exit;
    }
    offset += 8;
    return offset;

kplc_IOState_float16x2Response_error_exit:
    if (ret < 0)
    {
        return ret;
    }
    else
    {
        return -CANARD_ERROR_INTERNAL;
    }
}

/**
  * @brief kplc_IOState_float16x2Response_decode
  * @param transfer: Pointer to CanardRxTransfer transfer
  * @param payload_len: Payload message length
  * @param dest: Pointer to destination struct
  * @param dyn_arr_buf: NULL or Pointer to memory storage to be used for dynamic arrays
  *                     kplc_IOState_float16x2Response dyn memory will point to dyn_arr_buf memory.
  *                     NULL will ignore dynamic arrays decoding.
  * @retval offset or ERROR value if < 0
  */
int32_t kplc_IOState_float16x2Response_decode(const CanardRxTransfer* transfer,
  uint16_t payload_len,
  kplc_IOState_float16x2Response* dest,
  uint8_t** dyn_arr_buf)
{
    const int32_t offset = 0;
    int32_t ret = 0;

    // Clear the destination struct
    for (uint32_t c = 0; c < sizeof(kplc_IOState_float16x2Response); c++)
    {
        ((uint8_t*)dest)[c] = 0x00;
    }

    ret = kplc_IOState_float16x2Response_decode_internal(transfer, payload_len, dest, dyn_arr_buf, offset);

    return ret;
}
