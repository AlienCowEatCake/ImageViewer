#include "wels/codec_api.h"
#include "wels/codec_ver.h"

OpenH264Version WelsGetCodecVersion()
{
    return g_stCodecVersion;
}

void WelsGetCodecVersionEx(OpenH264Version* pVersion)
{
    *pVersion = g_stCodecVersion;
}
