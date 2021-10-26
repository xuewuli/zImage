#pragma once

#include <cstdint>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned char * convertCCZBuffer( const unsigned char *buffer, size_t bufferLen, size_t &outSize );

#ifdef __cplusplus
}
#endif