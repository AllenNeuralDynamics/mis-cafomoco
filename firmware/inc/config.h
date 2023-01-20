#ifndef CONFIG_H
#define CONFIG_H

#define NUM_BMCS (6)

// Encoder GPIOS are enumerated contiguously starting from this address:
#define ENCODER_BASE_OFFSET (12)
// i.e: ENC_0_A, ENC_0_B, ENC_1_A, ENC_1_B, .... ENC_N_A, ENC_N_B
// EXCEPT the last (sixth) encoder, which is on GPIOS 26 and 27.

#endif // CONFIG_H
