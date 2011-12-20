#include "CodecInst.h"

/* we accept RGB or RGBA input */
static bool CanCompress(LPBITMAPINFOHEADER lpbiIn)
{
    if(0 == lpbiIn->biCompression || mmioFOURCC('D','I','B',' ') == lpbiIn->biCompression)
    {
        if(24 == lpbiIn->biBitCount || 32 == lpbiIn->biBitCount)
            return true;
    };
    return false;
};

static bool CanCompress(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    if(!lpbiOut)
        return CanCompress(lpbiIn);

    if(FOURCC_S3TC == lpbiOut->biCompression)
        return (lpbiIn->biBitCount == lpbiOut->biBitCount);

    return false;
};

DWORD CodecInst::CompressGetSize(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    return ALIGN4(lpbiIn->biWidth) * lpbiIn->biHeight * (lpbiIn->biBitCount / 8);
};

DWORD CodecInst::CompressQuery(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    bool r = CanCompress(lpbiIn, lpbiOut);

    log_message
    (
       "CodecInst::CompressQuery()=%s  lpbiIn->biCompression=[%.8X], lpbiIn->biBitCount=%d\n\n",
       r ? "ICERR_OK":"ICERR_BADFORMAT",
       lpbiIn->biCompression,
       lpbiIn->biBitCount
    );

    return r ? ICERR_OK : ICERR_BADFORMAT;
};

DWORD CodecInst::CompressGetFormat(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    if (lpbiOut == 0)
        return sizeof(BITMAPINFOHEADER);

    bool r = CanCompress(lpbiIn);

    log_message
    (
       "CodecInst::CompressGetFormat()=%s lpbiIn->biCompression=[%.8X], lpbiIn->biBitCount=%d lpbiOut->biCompression=[%.8X], lpbiOut->biBitCount=%d\n\n",
       r ? "ICERR_OK":"ICERR_BADFORMAT",
       lpbiIn->biCompression,
       lpbiIn->biBitCount,
       (lpbiOut)?lpbiOut->biCompression:0xFFFFFFFF,
       (lpbiOut)?lpbiOut->biBitCount:0
    );

    *lpbiOut = *lpbiIn;
    lpbiOut->biCompression = FOURCC_S3TC;
    return ICERR_OK;
};

DWORD CodecInst::CompressBegin(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    log_message("CodecInst::CompressBegin()\n\n");

    CompressEnd();  // free resources if necessary

    // allocate buffers
    rows = (unsigned char**)malloc(sizeof(unsigned char*) * lpbiIn->biHeight);

    return CanCompress(lpbiIn, lpbiOut) ? ICERR_OK : ICERR_BADFORMAT;
};

DWORD CodecInst::CompressEnd()
{
    // release buffers
    if(rows)
    {
        log_message("CodecInst::CompressEnd()\n\n");
        free(rows);
        rows = NULL;
    };

    return ICERR_OK;
};

