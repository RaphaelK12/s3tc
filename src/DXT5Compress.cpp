#include "CodecInst.h"

static int power(int a, int b, int c)
{
    return a*a + b*b + c*c;
};

DWORD CodecInst::Compress(ICCOMPRESS* icinfo, DWORD dwSize)
{
    int c, i, j, d, p, b, l;
    int src_pitch, pix_size;
    int block_h, block_w;
    int color_codebook[4][3];
    int alpha_codebook[8];
    long long v, COLOR, ALPHA, *dst;

    log_message("CodecInst::Compress(%d) INPUT\n\n", icinfo->lFrameNum);

    if(ICERR_OK != CompressQuery(icinfo->lpbiInput, icinfo->lpbiOutput))
        return ICERR_BADFORMAT;

    if(32 == icinfo->lpbiInput->biBitCount)
        pix_size = 4;
    else if(24 == icinfo->lpbiInput->biBitCount)
        pix_size = 3;
    else
    {
        log_message("CodecInst::Compress error value of icinfo->lpbiInput->biBitCount=%d\n",
            icinfo->lpbiInput->biBitCount);
        return ICERR_BADFORMAT;
    };

    dst = (long long*)icinfo->lpOutput;

    /* calc line size */
    src_pitch = ALIGN4(icinfo->lpbiInput->biWidth) * (icinfo->lpbiInput->biBitCount / 8);

    /* calc rows ptrs */
    for(i = 0; i < icinfo->lpbiInput->biHeight; i++)
        rows[i] = (unsigned char*)icinfo->lpInput + src_pitch * i;

#define PIX(N) rows[block_h + j][(block_w + i) * pix_size + N]
#define vec_max color_codebook[0]
#define vec_min color_codebook[1]

    /* check 4x4 squares */
    for(block_h = 0; block_h < icinfo->lpbiInput->biHeight; block_h += 4)
    {
        for(block_w = 0; block_w < icinfo->lpbiInput->biWidth; block_w += 4)
        {
            int power_min, power_max;

//__debugbreak();

//            log_message("CodecInst::Compress block_h=%d, block_w=%d\n", block_h, block_w);

            if(4 == pix_size)
            {
                /* find a maximum and min alpha */
                for(j = 0; j < 4; j++) for(i = 0; i < 4; i++)
                {
                    p = PIX(3);
                    if(!j && !i)
                        power_min = power_max = p;
                    else
                    {
                        if(p < power_min) power_min = p;
                        if(power_max < p) power_max = p;
                    };
                };

                /* build alpha codebook */
                alpha_codebook[0] = power_max;
                alpha_codebook[1] = power_min;
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

                for(ALPHA = 0, b = 0, j = 0; j < 4; j++) for(i = 0; i < 4; i++, b++)
                {
                    for(l = 0, v = 0; l < 8; l++)
                    {
                        p = power(alpha_codebook[l] - (int)PIX(3), 0, 0);
                        if(!l)
                            d = p;
                        else
                        {
                            if(p < d)
                            {
                                d = p;
                                v = l;
                            };
                        };
                    };
                    ALPHA |= l << (b * 3);
                };

                /* save alpha components */
                ALPHA <<= 8;
                ALPHA |= alpha_codebook[1];
                ALPHA <<= 8;
                ALPHA |= alpha_codebook[0];
            }
            else
                ALPHA = 0x00FF;

            *dst = ALPHA;
            dst++;

            /* find max min colors */
            for(j = 0; j < 4; j++) for(i = 0; i < 4; i++)
            {
                p = power(PIX(0), PIX(1), PIX(2));

//                log_message("CodecInst::Compress j=%d, i=%d, [%d,%d,%d]\n",
//                    i, j, (unsigned int)PIX(0), (unsigned int)PIX(1), (unsigned int)PIX(2));

                if(!j && !i)
                {
                    power_min = power_max = p;
                    for(c = 0; c < 3; c++) vec_min[c] = vec_max[c] = PIX(c);
                }
                else
                {
                    if(p < power_min)
                    {
                        power_min = p;
                        for(c = 0; c < 3; c++) vec_min[c] = PIX(c);
                    };

                    if(power_max < p)
                    {
                        power_max = p;
                        for(c = 0; c < 3; c++) vec_max[c] = PIX(c);
                    };
                };
            };

            /* build color codebook */
            for(c = 0; c < 3; c++)
            {
                color_codebook[2][c] = (2 * color_codebook[0][c] + color_codebook[1][c] + 1) / 3;
                color_codebook[3][c] = (color_codebook[0][c] + 2*color_codebook[1][c] + 1) / 3;
            }

            /* find code book color */
            for(COLOR = 0, b = 0, j = 0; j < 4; j++) for(i = 0; i < 4; i++, b++)
            {
                for(l = 0, v = 0; l < 4; l++)
                {
                    p = power
                    (
                        color_codebook[l][0] - (int)PIX(0),
                        color_codebook[l][1] - (int)PIX(1),
                        color_codebook[l][2] - (int)PIX(2)
                    );

                    if(!l)
                        d = p;
                    else
                    {
                        if(p < d)
                        {
                            d = p;
                            v = l;
                        };
                    };
                };

                COLOR |= v << (b * 2);
            };

            /* save color component */
            COLOR <<= 5;
            COLOR |= color_codebook[1][2] >> 3;
            COLOR <<= 6;
            COLOR |= color_codebook[1][1] >> 2;
            COLOR <<= 5;
            COLOR |= color_codebook[1][0] >> 3;

            COLOR <<= 5;
            COLOR |= color_codebook[0][2] >> 3;
            COLOR <<= 6;
            COLOR |= color_codebook[0][1] >> 2;
            COLOR <<= 5;
            COLOR |= color_codebook[0][0] >> 3;

            *dst = COLOR;
            dst++;

//            log_message("CodecInst::Compress COLOR=0x%X,0x%X\n",
//                (unsigned int)(COLOR >> 32), (unsigned int)(COLOR));
        };
    };

    /* setup compressed inmage size */
    icinfo->lpbiOutput->biSizeImage = (DWORD)((unsigned char*)dst - (unsigned char*)icinfo->lpOutput);
    log_message("CodecInst::Compress(%d) size=%d bytes\n\n", icinfo->lFrameNum,
        icinfo->lpbiOutput->biSizeImage);
    if (icinfo->lpckid)
        *icinfo->lpckid = FOURCC_S3TC;
    *icinfo->lpdwFlags = AVIIF_KEYFRAME;

    return ICERR_OK;
}
