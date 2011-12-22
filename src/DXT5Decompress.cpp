#include "CodecInst.h"

DWORD CodecInst::Decompress(ICDECOMPRESS* icinfo, DWORD dwSize)
{
    int c, i, j;
    int dst_pitch, pix_size = 4;
    int block_h, block_w;
    int color_codebook[4][3];
    int alpha_codebook[8];
    long long COLOR, ALPHA, *src;

    if(ICERR_OK != DecompressQuery(icinfo->lpbiInput, icinfo->lpbiOutput))
        return ICERR_BADFORMAT;

    if(32 == icinfo->lpbiOutput->biBitCount)
        pix_size = 4;
    else if(24 == icinfo->lpbiOutput->biBitCount)
        pix_size = 3;
    else
    {
        log_message("CodecInst::Decompress error value of icinfo->lpbiOutput->biBitCount=%d\n",
            icinfo->lpbiOutput->biBitCount);
        return ICERR_BADFORMAT;
    };

    src = (long long*)icinfo->lpInput;

    /* calc line size */
    dst_pitch = ALIGN4(icinfo->lpbiOutput->biWidth) * (icinfo->lpbiOutput->biBitCount / 8);

    /* calc rows ptrs */
    for(i = 0; i < icinfo->lpbiOutput->biHeight; i++)
        rows[i] = (unsigned char*)icinfo->lpOutput + dst_pitch * i;

#define PIX(N) rows[block_h + j][(block_w + i) * pix_size + N]

    /* check 4x4 squares */
    for(block_h = 0; block_h < icinfo->lpbiInput->biHeight; block_h += 4)
    {
        for(block_w = 0; block_w < icinfo->lpbiInput->biWidth; block_w += 4)
        {
            ALPHA = *src; src++;
            COLOR = *src; src++;

            /* build alpha codebook */
            if(32 == icinfo->lpbiOutput->biBitCount)
            {
                alpha_codebook[0] = ALPHA & 0x00FF; ALPHA >>= 8;
                alpha_codebook[1] = ALPHA & 0x00FF; ALPHA >>= 8;
                if(alpha_codebook[0] > alpha_codebook[1])
                {
                    alpha_codebook[2] = (6 * alpha_codebook[0] + 1 * alpha_codebook[1] + 3) / 7;
                    alpha_codebook[3] = (5 * alpha_codebook[0] + 2 * alpha_codebook[1] + 3) / 7;
                    alpha_codebook[4] = (4 * alpha_codebook[0] + 3 * alpha_codebook[1] + 3) / 7;
                    alpha_codebook[5] = (3 * alpha_codebook[0] + 4 * alpha_codebook[1] + 3) / 7;
                    alpha_codebook[6] = (2 * alpha_codebook[0] + 5 * alpha_codebook[1] + 3) / 7;
                    alpha_codebook[7] = (1 * alpha_codebook[0] + 6 * alpha_codebook[1] + 3) / 7;
                }
                else
                {
                    alpha_codebook[2] = (4 * alpha_codebook[0] + 1 * alpha_codebook[1] + 2) / 5;
                    alpha_codebook[3] = (3 * alpha_codebook[0] + 2 * alpha_codebook[1] + 2) / 5;
                    alpha_codebook[4] = (2 * alpha_codebook[0] + 3 * alpha_codebook[1] + 2) / 5;
                    alpha_codebook[5] = (1 * alpha_codebook[0] + 4 * alpha_codebook[1] + 2) / 5;
                    alpha_codebook[6] = 0;
                    alpha_codebook[7] = 255;
                };
            };

            /* build color codebook */
            color_codebook[0][0] = COLOR & 0x1F;
            color_codebook[0][0] <<= 3;
            COLOR >>= 5;
            color_codebook[0][1] = COLOR & 0x3F;
            color_codebook[0][1] <<= 2;
            COLOR >>= 6;
            color_codebook[0][2] = COLOR & 0x1F;
            color_codebook[0][2] <<= 3;
            COLOR >>= 5;
            color_codebook[1][0] = COLOR & 0x1F;
            color_codebook[1][0] <<= 3;
            COLOR >>= 5;
            color_codebook[1][1] = COLOR & 0x3F;
            color_codebook[1][1] <<= 2;
            COLOR >>= 6;
            color_codebook[1][2] = COLOR & 0x1F;
            color_codebook[1][2] <<= 3;
            COLOR >>= 5;
            for(c = 0; c < 3; c++)
            {
                color_codebook[2][c] = (2 * color_codebook[0][c] + color_codebook[1][c] + 1) / 3;
                color_codebook[3][c] = (color_codebook[0][c] + 2*color_codebook[1][c] + 1) / 3;
            }

            /* store pixels */
            for(j = 0; j < 4; j++) for(i = 0; i < 4; i++)
            {
                for(c = 0; c < 3; c++)
                    PIX(c) = color_codebook[COLOR & 0x03][c];
                COLOR >>= 2;

                if(32 == icinfo->lpbiOutput->biBitCount)
                {
                    PIX(3) = alpha_codebook[ALPHA & 0x07];
                    ALPHA >>= 3;
                };
            };

        };
    };

    /* setup compressed inmage size */
    icinfo->lpbiOutput->biSizeImage = icinfo->lpbiOutput->biHeight * dst_pitch;

    return ICERR_OK;
};
