#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { float x, y; } ErgoVec2;
typedef struct { float w, h; } ErgoSize2;
typedef struct { uint8_t r, g, b, a; } ErgoColor;
typedef struct { ErgoVec2 position; float rotation; ErgoSize2 size; } ErgoTransform2D;
typedef struct { uint64_t id; } ErgoTaskHandle;
typedef struct { uint64_t id; } ErgoColliderHandle;
typedef struct { uint64_t id; } ErgoTextureHandle;

// Network handles
typedef struct { uint64_t id; } ErgoNetHandle;

// Network message (C API)
typedef struct {
    uint16_t type;
    const uint8_t* payload;
    uint32_t payload_len;
} ErgoNetMessage;

// Network event types
typedef enum {
    ERGO_NET_CONNECTED    = 0,
    ERGO_NET_DISCONNECTED = 1,
    ERGO_NET_ERROR        = 2,
} ErgoNetEvent;

// HTTP response (C API)
typedef struct {
    int status_code;
    const char* body;
    uint32_t body_len;
} ErgoHttpResponse;

#ifdef __cplusplus
}
#endif
