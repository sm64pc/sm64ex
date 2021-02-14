#include <stdio.h>
#include <stdlib.h>
#include "vadpcm.h"

s32 readcodebook(FILE *fhandle, s32 ****table, s32 *order, s32 *npredictors)
{
    s32 **table_entry;
    s32 i;
    s32 j;
    s32 k;

    if (fscanf(fhandle, "%d", order) != 1) {
        printf("I/O error occurred.");
        exit(1);
    };
    if (fscanf(fhandle, "%d", npredictors) != 1) {
        printf("I/O error occurred.");
        exit(1);
    };
    *table = malloc(*npredictors * sizeof(s32 **));
    for (i = 0; i < *npredictors; i++)
    {
        (*table)[i] = malloc(8 * sizeof(s32 *));
        for (j = 0; j < 8; j++)
        {
            (*table)[i][j] = malloc((*order + 8) * sizeof(s32));
        }
    }

    for (i = 0; i < *npredictors; i++)
    {
        table_entry = (*table)[i];
        for (j = 0; j < *order; j++)
        {
            for (k = 0; k < 8; k++)
            {
                if (fscanf(fhandle, "%d", &table_entry[k][j]) != 1) {
                    printf("I/O error occurred.");
                    exit(1);
                };
            }
        }

        for (k = 1; k < 8; k++)
        {
            table_entry[k][*order] = table_entry[k - 1][*order - 1];
        }

        table_entry[0][*order] = 1 << 11;

        for (k = 1; k < 8; k++)
        {
            for (j = 0; j < k; j++)
            {
                table_entry[j][k + *order] = 0;
            }

            for (; j < 8; j++)
            {
                table_entry[j][k + *order] = table_entry[j - k][*order];
            }
        }
    }
    return 0;
}

s32 readaifccodebook(FILE *fhandle, s32 ****table, s16 *order, s16 *npredictors)
{
    s32 **table_entry;
    s32 i;
    s32 j;
    s32 k;
    s16 ts;

    if (fread(order, sizeof(s16), 1, fhandle) != 1) {
        printf("I/O error occurred.");
        exit(1);
    };
    BSWAP16(*order)
    if (fread(npredictors, sizeof(s16), 1, fhandle) != 1) {
        printf("I/O error occurred.");
        exit(1);
    };
    BSWAP16(*npredictors)
    *table = malloc(*npredictors * sizeof(s32 **));
    for (i = 0; i < *npredictors; i++)
    {
        (*table)[i] = malloc(8 * sizeof(s32 *));
        for (j = 0; j < 8; j++)
        {
            (*table)[i][j] = malloc((*order + 8) * sizeof(s32));
        }
    }

    for (i = 0; i < *npredictors; i++)
    {
        table_entry = (*table)[i];
        for (j = 0; j < *order; j++)
        {
            for (k = 0; k < 8; k++)
            {
                if (fread(&ts, sizeof(s16), 1, fhandle) != 1) {
                    printf("I/O error occurred.");
                    exit(1);
                };
                BSWAP16(ts)
                table_entry[k][j] = ts;
            }
        }

        for (k = 1; k < 8; k++)
        {
            table_entry[k][*order] = table_entry[k - 1][*order - 1];
        }

        table_entry[0][*order] = 1 << 11;

        for (k = 1; k < 8; k++)
        {
            for (j = 0; j < k; j++)
            {
                table_entry[j][k + *order] = 0;
            }

            for (; j < 8; j++)
            {
                table_entry[j][k + *order] = table_entry[j - k][*order];
            }
        }
    }
    return 0;
}

s32 inner_product(s32 length, s32 *v1, s32 *v2)
{
    s32 j;
    s32 dout;
    s32 fiout;
    s32 out;

    j = 0;
    out = 0;
    for (; j < length; j++)
    {
        out += *v1++ * *v2++;
    }

    // Compute "out / 2^11", rounded down.
    dout = out / (1 << 11);
    fiout = dout * (1 << 11);
    if (out - fiout < 0)
    {
        return dout - 1;
    }
    else
    {
        return dout;
    }
}
