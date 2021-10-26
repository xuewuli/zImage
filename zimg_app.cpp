#include "zpng.h"
#include "zpvr.h"

#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#define STB_IMAGE_IMPLEMENTATION /* compile it here */
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"


#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#elif __MACH__
    #include <mach/mach_time.h>
    #include <mach/mach.h>
    #include <mach/clock.h>

    extern mach_port_t clock_port;
#else
    #include <time.h>
    #include <sys/time.h>
#endif


//------------------------------------------------------------------------------
// Timing

#ifdef _WIN32
static double PerfFrequencyInverse = 0.;

static void InitPerfFrequencyInverse() {
    LARGE_INTEGER freq = {};
    if (!::QueryPerformanceFrequency(&freq) || freq.QuadPart == 0)
        return;
    PerfFrequencyInverse = 1000000. / (double)freq.QuadPart;
}
#elif __MACH__
static bool m_clock_serv_init = false;
static clock_serv_t m_clock_serv = 0;

static void InitClockServ() {
    m_clock_serv_init = true;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &m_clock_serv);
}
#endif // _WIN32

static uint64_t GetTimeUsec() {
#ifdef _WIN32
    LARGE_INTEGER timeStamp = {};
    if (!::QueryPerformanceCounter(&timeStamp))
        return 0;
    if (PerfFrequencyInverse == 0.)
        InitPerfFrequencyInverse();
    return (uint64_t)(PerfFrequencyInverse * timeStamp.QuadPart);
#elif __MACH__
    if (!m_clock_serv_init)
        InitClockServ();

    mach_timespec_t tv;
    clock_get_time(m_clock_serv, &tv);

    return 1000000 * tv.tv_sec + tv.tv_nsec / 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return 1000000 * tv.tv_sec + tv.tv_usec;
#endif
}

static void usage() {
    cout << "Usage: zimg -c InputFileName OutputFileName" << endl;
    cout << "Usage: zimg -d InputFileName OutputFileName" << endl;
}

int main(int argc, char** argv)
{
    bool compress = false;
    bool decompress = false;
    const char* sourceFile = nullptr;
    const char* destFile = nullptr;

    if (argc >= 4)
    {
        if (0 == strcmp(argv[1], "-c")) {
            compress = true;
        }
        else if (0 == strcmp(argv[1], "-d")) {
            decompress = true;
        } else {
            usage();
            return 0;
        }
        sourceFile = argv[2];
        destFile = argv[3];
    } else {
        usage();
        return 0;
    }

    std::ifstream input(sourceFile, std::ios::binary);
    if (!input)
    {
        cout << "Could not open input file" << endl;
        return -4;
    }

    // copies all data into buffer
    std::vector<char> fileData((
        std::istreambuf_iterator<char>(input)),
        (std::istreambuf_iterator<char>()));

    if (compress)
    {
        cout << "Compressing " << sourceFile << " to " << destFile << endl;

        int x, y, comp;
        stbi_uc* data = stbi_load_from_memory((uint8_t*)&fileData[0], (unsigned)fileData.size(), &x, &y, &comp, 0);
        if (data)
        {
            ZPNG_ImageData image;
            image.Buffer.Data = data;
            image.Buffer.Bytes = x * y * comp;
            image.BytesPerChannel = 1;
            image.Channels = comp;
            image.HeightPixels = y;
            image.WidthPixels = x;
            image.StrideBytes = x * image.Channels;

            Z_Buffer buffer = ZPNG_Compress(&image);

            if (!buffer.Data)
            {
                cout << "ZPNG compression failed" << endl;
                return -2;
            }

            cout << "ZPNG compression size: " << buffer.Bytes << " bytes" << endl;

            std::ofstream output(destFile, std::ios::binary);
            if (!output)
            {
                cout << "Could not open output file" << endl;
                return -3;
            }
            output.write((char*)buffer.Data, buffer.Bytes);
            output.close();

            stbi_image_free(data);

            ZPNG_Free(&buffer);

            return 0;
        }

        size_t len = 0;
        data = convertCCZBuffer((uint8_t*)&fileData[0], fileData.size(), len);
        if (data) {

            cout << "ZPVR compression size: " << len << " bytes" << endl;

            std::ofstream output(destFile, std::ios::binary);
            if (!output)
            {
                cout << "Could not open output file" << endl;
                return -3;
            }
            output.write((char*)data, len);
            output.close();

            free(data);

            return 0;
        }


        cout << "Unable to load file: " << sourceFile << endl;
        return -1;

    }
    else if (decompress)
    {
        cout << "Decompressing " << sourceFile << " to " << destFile << " (output will be PNG format)" << endl;

        Z_Buffer buffer{(uint8_t*)&fileData[0], (unsigned)fileData.size()};

        uint64_t t0 = GetTimeUsec();

        ZPNG_ImageData decompressResult = ZPNG_Decompress(buffer);

        uint64_t t1 = GetTimeUsec();

        if (!decompressResult.Buffer.Data)
        {
            cout << "Decompression failed" << endl;
            return -5;
        }

        cout << "Decompressed ZPNG in " << (t1 - t0) / 1000.f << " msec" << endl;

        t0 = GetTimeUsec();

        int writeResult = stbi_write_png(
            destFile,
            decompressResult.WidthPixels,
            decompressResult.HeightPixels,
            decompressResult.Channels,
            decompressResult.Buffer.Data,
            decompressResult.StrideBytes);

        t1 = GetTimeUsec();

        if (!writeResult)
        {
            cout << "Failed to compress PNG" << endl;
            return -6;
        }

        cout << "Compressed PNG in " << (t1 - t0) / 1000.f << " msec" << endl;

        cout << "Wrote decompressed PNG file: " << destFile << endl;

        ZPNG_Free(&decompressResult.Buffer);
    }

    return 0;
}
