#include "funcs.c"
#include "bit_buffer.c"
#include "random.h"

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

//#define BUFFER_SIZE_BITS  BLOCK_BITS*7 + 3
#define TEST_SIZE 1000

void test_padding_functions(bit_c_t size,pcg32_random_t* rng) {
    bit_buffer_t* buffer = init_buffer(size+10);
    if (!buffer) {
        fprintf(stderr, "Failed to initialize buffer\n");
        return;
    }

    // Save a random bit for integrity check
    bit_c_t saved_bit_index;// = pcg32_random_r(rng) % BUFFER_SIZE_BITS;
    //printf("saved_bit_index: %lu\n",saved_bit_index);
    bool saved_bit_value;// = read_from(buffer, saved_bit_index);

    // 1. Check padding with different sizes
	for(bit_c_t i = 1; i < size; i++) {
	    printf("testing size: %lu\n",i);

	    // Save a random bit for integrity check within the current size 'i'
	    bit_c_t saved_bit_index = pcg32_random_r(rng) % i;
	    printf("saved_bit_index: %lu\n",saved_bit_index);
	    fflush(stdout);
	    bool saved_bit_value = read_from(buffer, saved_bit_index);

	    uint8_t pad = PAD(buffer, i);
	    assert(pad >= 3 && pad <= 10);  // Padding should be between 3 to 10 bits

	    uint8_t unpadded = UNPAD(buffer, i + pad);
	    assert(unpadded == pad);  // Unpadding should give the same value back

	    // Check if the saved bit remains unchanged
	    bool post_unpad_bit_value = read_from(buffer, saved_bit_index);
	    assert(post_unpad_bit_value == saved_bit_value);
	}


    // 2. Random Access Padding
    printf("\n___________________\nread test\n");
    bit_c_t random_bit_position = pcg32_random_r(rng) % size;
    uint8_t pad = PAD(buffer, random_bit_position);
    assert(pad >= 3 && pad <= 10);
    uint8_t unpadded = UNPAD(buffer, random_bit_position + pad);
    assert(unpadded == pad);

    // // Check if the saved bit remains unchanged
    // bool post_unpad_bit_value = read_from(buffer, saved_bit_index);
    // assert(post_unpad_bit_value == saved_bit_value);

    free_buffer(buffer);
}

int main() {
    pcg32_random_t rng = get_rng();
    for (bit_c_t i = 1; i < TEST_SIZE; i++) {
        //bit_c_t random_bit_position = pcg32_random_r(&rng) % BUFFER_SIZE_BITS;
        test_padding_functions(i,&rng);
    }

    printf("All tests passed successfully!\n");
    return 0;
}
