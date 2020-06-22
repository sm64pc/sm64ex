#include "libultra_internal.h"
#include <string.h>

void guMtxF2L(float mf[4][4], Mtx *m) {
    memcpy(m->m, mf, sizeof(Mtx));
}

void guMtxIdentF(float mf[4][4]) {
    int r, c;
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            if (r == c) {
                mf[r][c] = 1.0f;
            } else {
                mf[r][c] = 0.0f;
            }
        }
    }
}
void guMtxIdent(Mtx *m) {
    guMtxIdentF(m->m);
}

