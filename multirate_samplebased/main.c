#include "xlaudio.h"
#include "xlaudio_armdsp.h"
#include <stdlib.h>

#define NUMCOEF 97
#define RATE    16

// rcosdesign(0, 6, 16, 'normal')'

float32_t coef[NUMCOEF] = {
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

int nextSymbol() {
    return rand() % 2;
}

static int phase = 0;

// the input is 'upsampled' to RATE, by inserting N-1 zeroes every N samples
float32_t nextSample() {
    phase = (phase + 1) % RATE;
    if (phase == 0) {
        xlaudio_debugpinhigh();
        return (nextSymbol() ? 1.f : -1.f);
    } else {
        xlaudio_debugpinlow();
        return 0.0f;
    }
}

// traditional implementation
float32_t taps[NUMCOEF];
uint16_t processSample(uint16_t x) {
    taps[0] = nextSample();

    float32_t q = 0.0;
    uint16_t i;
    for (i = 0; i<NUMCOEF; i++)
        q += taps[i] * coef[i];

    for (i = NUMCOEF-1; i>0; i--)
        taps[i] = taps[i-1];

    return xlaudio_f32_to_dac14(0.1*q);
}

float32_t symboltaps[NUMCOEF/RATE + 1];

uint16_t processSampleMultirate(uint16_t x) {
    float32_t input = nextSample();

    uint16_t i;
    if (phase == 0) {
        symboltaps[0] = input;
        for (i=NUMCOEF/RATE; i>0; i--)
            symboltaps[i] = symboltaps[i-1];
    }

    float32_t q = 0.0;
    uint16_t limit = (phase == 0) ? NUMCOEF/RATE + 1 : NUMCOEF/RATE;
    for (i=0; i<limit; i++)
        q += symboltaps[i] * coef[i * RATE + phase + 1];

    return xlaudio_f32_to_dac14(0.1*q);
}


#include <stdio.h>

int main(void) {
    WDT_A_hold(WDT_A_BASE);

    int c = xlaudio_measurePerfSample(processSample);
    printf("Cycles Direct %d\n", c);

    c = xlaudio_measurePerfSample(processSampleMultirate);
    printf("Cycles Multirate %d\n", c);

    xlaudio_init_intr(FS_8000_HZ, XLAUDIO_J1_2_IN, processSampleMultirate);
    xlaudio_run();

    return 1;
}

