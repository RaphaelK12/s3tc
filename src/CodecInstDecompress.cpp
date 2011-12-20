#include "CodecInst.h"

static bool CanDecompress(LPBITMAPINFOHEADER lpbiIn)
{
    if(FOURCC_S3TC == lpbiIn->biCompression)
        return true;
    return false;
};

static bool CanDecompress(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    if(!lpbiOut)
        return CanDecompress(lpbiIn);

    if(0 == lpbiOut->biCompression || mmioFOURCC('D','I','B',' ') == lpbiOut->biCompression)
        return (lpbiIn->biBitCount == lpbiOut->biBitCount);

    return false;
};

DWORD CodecInst::DecompressQuery(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    bool r = CanDecompress(lpbiIn, lpbiOut);

    log_message
    (
       "CodecInst::DecompressQuery()=%s lpbiIn->biCompression=[%.8X], lpbiIn->biBitCount=%d lpbiOut->biCompression=[%.8X], lpbiOut->biBitCount=%d\n\n",
       r ? "ICERR_OK":"ICERR_BADFORMAT",
       lpbiIn->biCompression,
       lpbiIn->biBitCount,
       (lpbiOut)?lpbiOut->biCompression:0xFFFFFFFF,
       (lpbiOut)?lpbiOut->biBitCount:0
    );

    return r ? ICERR_OK : ICERR_BADFORMAT;
};

DWORD CodecInst::DecompressGetFormat(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
  // if lpbiOut == NULL, then return the size required to hold an output format struct
    if (lpbiOut == NULL)
        return lpbiIn->biSize;

    if (!CanDecompress(lpbiIn))
        return ICERR_BADFORMAT;

    memcpy(lpbiOut, lpbiIn, lpbiIn->biSize);

    lpbiOut->biCompression = 0;
    lpbiOut->biSizeImage = ALIGN4(lpbiIn->biWidth) * lpbiIn->biHeight * (lpbiIn->biBitCount / 8);

    return ICERR_OK;
};

DWORD CodecInst::DecompressBegin(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    CompressEnd();  // free resources if necessary

    // allocate buffers
    rows = (unsigned char**)malloc(sizeof(unsigned char*) * lpbiOut->biHeight);

    return CanDecompress(lpbiIn, lpbiOut) ? ICERR_OK : ICERR_BADFORMAT;
};

DWORD CodecInst::DecompressEnd()
{
    // release buffers
    if(rows)
    {
        free(rows);
        rows = NULL;
    };

    return ICERR_OK;
};

DWORD CodecInst::DecompressGetPalette(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    return ICERR_BADFORMAT;
};
