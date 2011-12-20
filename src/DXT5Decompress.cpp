#include "CodecInst.h"

DWORD CodecInst::Decompress(ICDECOMPRESS* icinfo, DWORD dwSize)
{
    if(ICERR_OK != DecompressQuery(icinfo->lpbiInput, icinfo->lpbiOutput))
        return ICERR_BADFORMAT;

    return ICERR_OK;
};
