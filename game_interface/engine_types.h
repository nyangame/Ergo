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

#ifdef __cplusplus
}
#endif
