#include "zlib.h"
#include "zstd/zstd.h"
#include <cstring>
#include <stdlib.h>

struct CCZHeader {
    unsigned char   sig[4];             /** Signature. Should be 'CCZ!' 4 bytes. */
    unsigned short  compression_type;   /** Should be 0. */
    unsigned short  version;            /** Should be 2 (although version type==1 is also supported). */
    unsigned int    reserved;           /** Reserved for users. */
    unsigned int    len;                /** Size of the uncompressed file. */
};

#define CC_HOST_IS_BIG_ENDIAN           (bool)(*(unsigned short *)"\0\xff" < 0x100)
#define CC_SWAP32(i)                    ((i & 0x000000ff) << 24 | (i & 0x0000ff00) << 8 | (i & 0x00ff0000) >> 8 | (i & 0xff000000) >> 24)
#define CC_SWAP16(i)                    ((i & 0x00ff) << 8 | (i & 0xff00) >> 8)
#define CC_SWAP_INT32_LITTLE_TO_HOST(i) ((CC_HOST_IS_BIG_ENDIAN == true) ? CC_SWAP32(i) : (i))
#define CC_SWAP_INT16_LITTLE_TO_HOST(i) ((CC_HOST_IS_BIG_ENDIAN == true) ? CC_SWAP16(i) : (i))
#define CC_SWAP_INT32_BIG_TO_HOST(i)    ((CC_HOST_IS_BIG_ENDIAN == true) ? (i) : CC_SWAP32(i))
#define CC_SWAP_INT16_BIG_TO_HOST(i)    ((CC_HOST_IS_BIG_ENDIAN == true) ? (i) : CC_SWAP16(i))

#ifdef __cplusplus
extern "C" {
#endif

unsigned char * convertCCZBuffer( const unsigned char *buffer, size_t bufferLen, size_t &outSize )
{
    if (bufferLen < sizeof(struct CCZHeader)) {
        return nullptr;
    }

    auto *header = reinterpret_cast<const struct CCZHeader*>(buffer);

    if( header->sig[0] == 'C' && header->sig[1] == 'C' && header->sig[2] == 'Z' && header->sig[3] == '!' ) {
        unsigned int version = CC_SWAP_INT16_BIG_TO_HOST( header->version );
        if( version > 2 ) {
            return nullptr;
        }

        if( CC_SWAP_INT16_BIG_TO_HOST(header->compression_type) != 0 ) {
            return nullptr;
        }

        uLong len = CC_SWAP_INT32_BIG_TO_HOST( header->len );
        auto out = (unsigned char*)malloc( len );
        if(! out ) {
            return nullptr;
        }

        int ret = uncompress(out, &len, (Bytef*)buffer + sizeof(struct CCZHeader), bufferLen - sizeof(struct CCZHeader) );

        if( ret != Z_OK ) {
            free(out);
            out = nullptr;
            return nullptr;
        }

        const size_t maxOutputBytes = ZSTD_compressBound(len + sizeof(struct CCZHeader));

        auto output = (uint8_t*)calloc(1, maxOutputBytes);

        auto *outHeader = reinterpret_cast<struct CCZHeader*>(output);
        outHeader->sig[0] = 'C';
        outHeader->sig[1] = 'C';
        outHeader->sig[2] = 'Z';
        outHeader->sig[3] = 'z';
        outHeader->version = 0;
        outHeader->compression_type = 0x0300;
        outHeader->len = header->len;
        outHeader->reserved = 0;

        const size_t result = ZSTD_compress(
                output + sizeof(struct CCZHeader),
                maxOutputBytes,
                out,
                len,
                ZSTD_maxCLevel());

        if (ZSTD_isError(result)) {
            free(out);
            out = nullptr;
            free(output);
            output = nullptr;
        }

        outSize = result + sizeof(struct CCZHeader);
        return output;
    }

    return nullptr;
}

#ifdef __cplusplus
}
#endif