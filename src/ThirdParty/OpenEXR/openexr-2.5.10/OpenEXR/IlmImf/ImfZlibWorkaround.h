#ifndef INCLUDED_IMF_ZLIB_WORKAROUND_H
#define INCLUDED_IMF_ZLIB_WORKAROUND_H

#if defined compress

namespace {

    inline int ZlibCompress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
    {
        return compress (dest, destLen, source, sourceLen);
    }

    #undef compress

    inline int compress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
    {
        return ZlibCompress (dest, destLen, source, sourceLen);
    }

} // namespace

#endif

#if defined uncompress

namespace {

    inline int ZlibUncompress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
    {
        return uncompress (dest, destLen, source, sourceLen);
    }

    #undef uncompress

    inline int uncompress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
    {
        return ZlibUncompress (dest, destLen, source, sourceLen);
    }

} // namespace

#endif

#endif
