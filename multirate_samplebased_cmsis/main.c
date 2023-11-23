#include "xlaudio.h"
#include "xlaudio_armdsp.h"
#include <stdlib.h>

// A multirate filter with an upsample factor of 16
// uses a block-based IO scheme with a block size of 16 (equal to the upsample factor)

// CMSIS requires NUMCOEF to be a multiple of RATE
// therefore, we tell ARM CMSIS the filter is one tap shorter
// than the length we get from Matlab

#define NUMCOEF 96
#define RATE    16
#define BLOCKSIZE RATE

// rcosdesign(0, 6, 16, 'normal')'

float32_t coef[NUMCOEF+1] = {
                           0.0000,
                           0.0001,
                           0.0003,
                           0.0007,
                           0.0013,
                           0.0020,
                           0.0028,
                           0.0037,
                           0.0046,
                           0.0054,
                           0.0059,
                           0.0062,
                           0.0061,
                           0.0055,
                           0.0043,
                           0.0025,
                          -0.0000,
                          -0.0031,
                          -0.0068,
                          -0.0109,
                          -0.0154,
                          -0.0200,
                          -0.0245,
                          -0.0286,
                          -0.0321,
                          -0.0345,
                          -0.0357,
                          -0.0352,
                          -0.0327,
                          -0.0282,
                          -0.0213,
                          -0.0119,
                           0.0000,
                           0.0143,
                           0.0310,
                           0.0497,
                           0.0702,
                           0.0920,
                           0.1147,
                           0.1377,
                           0.1604,
                           0.1823,
                           0.2028,
                           0.2212,
                           0.2371,
                           0.2500,
                           0.2595,
                           0.2653,
                           0.2673,
                           0.2653,
                           0.2595,
                           0.2500,
                           0.2371,
                           0.2212,
                           0.2028,
                           0.1823,
                           0.1604,
                           0.1377,
                           0.1147,
                           0.0920,
                           0.0702,
                           0.0497,
                           0.0310,
                           0.0143,
                           0.0000,
                          -0.0119,
                          -0.0213,
                          -0.0282,
                          -0.0327,
                          -0.0352,
                          -0.0357,
                          -0.0345,
                          -0.0321,
                          -0.0286,
                          -0.0245,
                          -0.0200,
                          -0.0154,
                          -0.0109,
                          -0.0068,
                          -0.0031,
                          -0.0000,
                           0.0025,
                           0.0043,
                           0.0055,
                           0.0061,
                           0.0062,
                           0.0059,
                           0.0054,
                           0.0046,
                           0.0037,
                           0.0028,
                           0.0020,
                           0.0013,
                           0.0007,
                           0.0003,
                           0.0001,
                           0.0000
};

float32_t nextSymbol() {
    return ((rand() % 2) ? 0.1f : -0.1f);
}

float32_t symbolTaps[NUMCOEF/RATE];
arm_fir_interpolate_instance_f32 F;

void processSampleMultirateCMSIS(uint16_t x[BLOCKSIZE], uint16_t y[BLOCKSIZE]) {
    float32_t input = nextSymbol();
    float32_t yf[BLOCKSIZE];
    xlaudio_debugpinhigh();
    arm_fir_interpolate_f32(&F, &input, yf, 1);
    xlaudio_debugpinlow();
    xlaudio_f32_to_dac14_vec(yf, y,BLOCKSIZE);
}

#include <stdio.h>

int main(void) {
    WDT_A_hold(WDT_A_BASE);
    arm_status k;

    k = arm_fir_interpolate_init_f32 (&F,
                                  RATE,        // rate
                                  NUMCOEF,     // number of coefficients
                                  coef,        // coefficients
                                  symbolTaps,  // filter state
                                  1);          // blocksize

    printf("%d\n", k);

    xlaudio_init_dma(FS_8000_HZ, XLAUDIO_J1_2_IN, BUFLEN_16, processSampleMultirateCMSIS);

    uint32_t c = xlaudio_measurePerfBuffer(processSampleMultirateCMSIS);
    printf("Cycles Multirate CMSIS %d\n", c);

    xlaudio_run();

    return 1;
}
