#include "CodecInst.h"

DWORD CodecInst::Compress(ICCOMPRESS* icinfo, DWORD dwSize)
{
    log_message("CodecInst::Compress(%d) INPUT\n\n", icinfo->lFrameNum);

    if(ICERR_OK != CompressQuery(icinfo->lpbiInput, icinfo->lpbiOutput))
        return ICERR_BADFORMAT;
}
