#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

typedef struct {
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

uint32_t pcg32_random_r(pcg32_random_t* rng);
pcg32_random_t initialize_rng_with_seed(uint64_t stream_id, uint64_t seed);
pcg32_random_t initialize_rng(uint64_t stream_id);
pcg32_random_t get_rng();

void inject_randomness_to_state(pcg32_random_t* rng, uint64_t randomness);

#endif // RANDOM_H
