#include <windows.h>
#include <vfw.h>
//#pragma hdrstop

#ifdef DXT5
static const DWORD FOURCC_S3TC = mmioFOURCC('D','X','T','5');   // our compressed format
#endif

#ifdef DXT3
static const DWORD FOURCC_S3TC = mmioFOURCC('D','X','T','3');   // our compressed format
#endif

extern HMODULE hmoduleS3TC;

#define ALIGN4(V) (((V + 3) >> 2) << 2)

struct CodecInst
{
    unsigned char **rows;

    // methods
    CodecInst() : rows(0) {}

    BOOL QueryAbout();
    DWORD About(HWND hwnd);

    BOOL QueryConfigure();
    DWORD Configure(HWND hwnd);

    DWORD GetState(LPVOID pv, DWORD dwSize);
    DWORD SetState(LPVOID pv, DWORD dwSize);

    DWORD GetInfo(ICINFO* icinfo, DWORD dwSize);

    DWORD CompressQuery(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
    DWORD CompressGetFormat(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
    DWORD CompressBegin(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
    DWORD CompressGetSize(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
    DWORD Compress(ICCOMPRESS* icinfo, DWORD dwSize);
    DWORD CompressEnd();

    DWORD DecompressQuery(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
    DWORD DecompressGetFormat(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
    DWORD DecompressBegin(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
    DWORD Decompress(ICDECOMPRESS* icinfo, DWORD dwSize);
    DWORD DecompressGetPalette(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
    DWORD DecompressEnd();

    void log_message(const char fmt[], ...);
};

CodecInst* Open(ICOPEN* icinfo);
DWORD Close(CodecInst* pinst);
