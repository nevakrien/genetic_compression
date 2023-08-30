#include "random.h"

// Constants for PCG Random Number Generation
#define PCG_MULTIPLIER 6364136223846793005ULL  // Multiplier constant for the LCG portion of PCG
#define PCG_INCREMENT  987654321                // Default increment for PCG (can be any odd value)


pcg32_random_t initialize_rng_with_seed(uint64_t stream_id, uint64_t seed) {
    stream_id^=PCG_INCREMENT;
    pcg32_random_t rng;
    rng.state = seed ^ stream_id;
    rng.inc = stream_id | 1ULL;
    pcg32_random_r(&rng);
    rng.state += seed;
    pcg32_random_r(&rng);
    return rng;
}

pcg32_random_t initialize_rng(uint64_t stream_id) {
    return initialize_rng_with_seed(stream_id, 42);
}

pcg32_random_t get_rng(){
    return initialize_rng_with_seed(0,42);
}

void inject_randomness_to_state(pcg32_random_t* rng, uint64_t randomness) {
    //doing some shifts in case the value is small
    randomness ^= randomness << 27;
    randomness ^= randomness >> 13;
    randomness ^= PCG_INCREMENT;
    
    rng->state ^= randomness;
    pcg32_random_r(rng);
}


uint32_t pcg32_random_r(pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    // Advance internal state using the LCG algorithm
    rng->state = oldstate * PCG_MULTIPLIER + (rng->inc|1);
    // Calculate output function (XSH RR) for better randomness quality
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31u));
}