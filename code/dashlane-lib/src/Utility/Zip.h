#pragma once

#include <zlib/zlib.h>

namespace Utility
{

    inline std::vector<uint8_t> InflateRaw(const std::span<const uint8_t>& in)
    {
        static constexpr std::size_t Z_CHUNK_SIZE = 2048;
        static constexpr std::size_t Z_W_BITS_RAW = -MAX_WBITS;

        std::vector<uint8_t> out;

        // Scale write buffer to a rounded up multiple of CHUNK size of the read buffer
        out.resize(Z_CHUNK_SIZE + in.size() - in.size() % Z_CHUNK_SIZE);

        int ret;
        z_stream strm = {};
        strm.opaque = nullptr;
        strm.zalloc = nullptr;
        strm.zfree = nullptr;
        strm.total_in = 0;
        strm.total_out = 0;

        /* allocate inflate state */
        ret = inflateInit2(&strm, Z_W_BITS_RAW);
        if (ret != Z_OK)
            return out;

        do {
            // Either the rest of the read buffer, or CHUNK SIZE
            strm.avail_in = std::min(Z_CHUNK_SIZE, in.size() - strm.total_in);

            // Exit when nothing left to inflate
            if (strm.avail_in == 0)
                break;

            // Move read pointer to next part
            strm.next_in = in.data() + strm.total_in;

            do {
                // Resize write buffer if remaining space is not enough to hold CHUNK
                out.resize(std::max(strm.total_out + Z_CHUNK_SIZE, out.size() - strm.total_out));

                // We only write CHUNK size per loop
                strm.avail_out = Z_CHUNK_SIZE;

                // Set write pointer to next free spot in buffer
                strm.next_out = out.data() + strm.total_out;

                // Inflate, and catch errors to cleanup
                ret = inflate(&strm, Z_NO_FLUSH);
                switch (ret)
                {
                case Z_STREAM_ERROR:
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    inflateEnd(&strm);
                    return out;
                }

            } while (strm.avail_out == 0);

        } while (ret != Z_STREAM_END);

        // Shrink write buffer to written byte count
        out.resize(strm.total_out);

        inflateEnd(&strm);
        return out;
    }

}