#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Z_Buffer {
    unsigned char* Data;
    unsigned Bytes;
};

struct ZPNG_ImageData {
    Z_Buffer Buffer;

    unsigned BytesPerChannel;
    unsigned Channels;
    unsigned WidthPixels;
    unsigned HeightPixels;
    unsigned StrideBytes;
};


Z_Buffer ZPNG_Compress( const ZPNG_ImageData* imageData );

ZPNG_ImageData ZPNG_Decompress( Z_Buffer buffer );

void ZPNG_Free( Z_Buffer* buffer );

#ifdef __cplusplus
}
#endif
