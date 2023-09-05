#include "bit_buffer.c"
#include "random.h"

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>  // For assert()

#define BUFFER_SIZE_BITS  BLOCK_BITS*7 +3 // Sample buffer size
#define TEST_SIZE 1000

void test_functions(bit_c_t random_bit_position) {
    // Create a buffer of BUFFER_SIZE_BITS bits
    bit_buffer_t* buffer = init_buffer(BUFFER_SIZE_BITS);
    if (!buffer) 
    {
        fprintf(stderr, "Failed to initialize buffer\n");
        return;
    }

    // 1. Boundary Testing
    write_to(buffer, 0, true);  // set the first bit
    assert(read_from(buffer, 0) == true);

    write_to(buffer, BUFFER_SIZE_BITS - 1, true);  // set the last bit
    assert(read_from(buffer, BUFFER_SIZE_BITS - 1) == true);

    // 2. Random Access
    write_to(buffer, random_bit_position, true);
    assert(read_from(buffer, random_bit_position) == true);

    // 3. Writing and Overwriting
    write_to(buffer, 10, true);
    assert(read_from(buffer, 10) == true);

    write_to(buffer, 10, false);
    assert(read_from(buffer, 10) == false);

    // 4. Error Scenarios - This will just print errors but not crash
    write_to(buffer, BUFFER_SIZE_BITS, true);
    read_from(buffer, BUFFER_SIZE_BITS);

    //free 
    free_buffer(buffer);
}

int main() {
    pcg32_random_t rng = get_rng();
    for (int i = 0; i < TEST_SIZE; i++) {
        //printf("testing %d\n",i);
        bit_c_t random_bit_position = pcg32_random_r(&rng) % BUFFER_SIZE_BITS;
        test_functions(random_bit_position);
    }
    
    printf("All tests passed successfully!\n");
    return 0;
}
