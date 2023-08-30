#include <stdio.h>
#include "random.h"

int main() {
    // Initialize RNG with a stream ID and default seed
    pcg32_random_t rng = initialize_rng(8);

    // Generate and print 10 random numbers
    printf("Generating 10 random numbers:\n");
    for (int i = 0; i < 10; i++) {
        printf("%u\n", pcg32_random_r(&rng));
    }

    // Re-initialize RNG with a stream ID and a specific seed
    rng = initialize_rng_with_seed(22, 42);

    // Generate and print another 10 random numbers
    printf("\nGenerating another 10 random numbers with a new seed:\n");
    for (int i = 0; i < 10; i++) {
        printf("%u\n", pcg32_random_r(&rng));
    }

     rng = initialize_rng_with_seed(22, 42);

    // Generate and print another 10 random numbers
    printf("\nGenerating another 10 random numbers with extra step:\n");
    for (int i = 0; i < 10; i++) {
        //no change
        inject_randomness_to_state(&rng,0);
        printf("%u\n", pcg32_random_r(&rng));
    }

    rng = initialize_rng_with_seed(22, 42);
    // Generate and print another 10 random numbers
    printf("\nnow with added 'randomnes':\n");
    for (int i = 0; i < 10; i++) {
        inject_randomness_to_state(&rng,1);
        printf("%u\n", pcg32_random_r(&rng));
    }

    rng = initialize_rng_with_seed(22, 42);
    // Generate and print another 10 random numbers
    printf("\nnow with added better randomnes:\n");
    for (int i = 0; i < 10; i++) {
        inject_randomness_to_state(&rng,3+i);
        printf("%u\n", pcg32_random_r(&rng));
    }

    return 0;
}
