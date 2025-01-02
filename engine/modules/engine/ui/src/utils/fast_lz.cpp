#include "ui/utils/fast_lz.h"
#include <vcruntime_string.h>
#define MAX_L2_DISTANCE 8191
#define MAGIC 0x4b50534e
namespace api {
    struct Header
    {
        uint32_t magic;
        uint32_t size;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    static void FastLZMove(uint8_t* dest, const uint8_t* src, uint32_t count)
    {
        if ((count > 4) && (dest >= src + count))
        {
            memmove(dest, src, count);
        }
        else
        {
            switch (count)
            {
            default:
                do { *dest++ = *src++; } while (--count);
                break;
            case 3:
                *dest++ = *src++;
            case 2:
                *dest++ = *src++;
            case 1:
                *dest++ = *src++;
            case 0:
                break;
            }
        }
    }
	uint32_t FastLZ::DecompressBufferSize(const void* buffer)
	{
        Header* header = (Header*)buffer;
        return header->size;
	}
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void FastLZ::Decompress(const void* input, uint32_t length, void* output)
    {
        Header* header = (Header*)input;
        int maxout = header->size;

        const uint8_t* ip = (const uint8_t*)input + sizeof(Header);
        const uint8_t* ip_limit = ip + length - sizeof(Header);
        const uint8_t* ip_bound = ip_limit - 2;
        uint8_t* op = (uint8_t*)output;
        uint8_t* op_limit = op + maxout;
        uint32_t ctrl = (*ip++) & 31;

        while (1)
        {
            if (ctrl >= 32)
            {
                uint32_t len = (ctrl >> 5) - 1;
                uint32_t ofs = (ctrl & 31) << 8;
                const uint8_t* ref = op - ofs - 1;

                uint8_t code;
                if (len == 7 - 1)
                {
                    do
                    {
                        code = *ip++;
                        len += code;
                    } while (code == 255);
                }

                code = *ip++;
                ref -= code;
                len += 3;

                /* match from 16-bit distance */
                if (code == 255)
                {
                    if (ofs == (31 << 8))
                    {
                        ofs = (*ip++) << 8;
                        ofs += *ip++;
                        ref = op - ofs - MAX_L2_DISTANCE - 1;
                    }
                }

                FastLZMove(op, ref, len);
                op += len;
            }
            else
            {
                ctrl++;
                memcpy(op, ip, ctrl);
                ip += ctrl;
                op += ctrl;
            }

            if (ip >= ip_limit) break;
            ctrl = *ip++;
        }
    }
}
