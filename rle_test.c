#include <stdio.h>
#include <assert.h>
#include <string.h> // for memcmp
#include "file_buffer.h"
#include "random.h"
#include "funcs.c"
#include "debug.c"

// ... Your byte_to_binary, show_Node, and random_list functions go here ...

#define CHUNK_COUNT 3
#define MAX_TEST_LENGTH MAX_BIT_SIZE*10

l_t random_length(pcg32_random_t* rng) {
    return (l_t)(1 + pcg32_random_r(rng) % MAX_TEST_LENGTH );
}

uint8_t random_window(pcg32_random_t* rng) {
    l_t ans = (l_t)(2 + pcg32_random_r(rng) %(MAX_WINDOW-1));
    assert((ans+7)/8<BLOCK_BYTES);
    return ans;
}

void print_lengths(l_t* lengths) {
    printf("Chunk lengths: [ ");
    for (int i = 0; i < CHUNK_COUNT; i++) {
        printf("%u", lengths[i]);
        if (i != CHUNK_COUNT - 1) {
            printf(", ");
        }
    }
    printf(" ]\n");
}

void print_windows(uint8_t* windows) {
    printf("Chunk windows: [ ");
    for (int i = 0; i < CHUNK_COUNT; i++) {
        printf("%u", windows[i]);
        if (i != CHUNK_COUNT - 1) {
            printf(", ");
        }
    }
    printf(" ]\n");
}



int main() {
    pcg32_random_t rng = get_rng();
    LinkedList original;
    LinkedList copy;
    LinkedList encoded;
    LinkedList decoded;

    l_t lengths[CHUNK_COUNT];
    uint8_t windows[CHUNK_COUNT];
    
    uint8_t j; //num encodes
    //int lengths[CHUNK_COUNT] = {3, 5, 7}; // Define the list of lengths you want to test

    for (int i = 13; i <= MAX_BIT_SIZE * 4; i++) {

        for (int j = 0; j < CHUNK_COUNT; j++) {
            lengths[j] = random_length(&rng); // you might want to adjust the range
        }
        print_lengths(lengths);

        for (int j = 0; j < CHUNK_COUNT; j++) {
            windows[j] = random_window(&rng); // you might want to adjust the range
        }
        print_windows(windows);

        printf("Original (%d bits):\n", i);
        original = random_list(&rng, i);
        // show_Node(original.head);

        copy=copy_list(original);
        show_Node(copy.head);

        encoded = create_empty_list();
        decoded = create_empty_list();


        // num_encodes=0;
        // Encoding
        j=0;
        for ( j = 0; j < CHUNK_COUNT; j++) {
            printf("Encoding chunk %d...\n", j);
            // curent_length=lengths[j];

            uint8_t temp=RLE_encoding(&copy, lengths+j, &encoded,windows[j], true);
            printf("length encoded: %u\n",temp);
            if(lengths[j]){
                lengths[j]=temp;
                show_Node(encoded.head);
                break;
            }
            lengths[j]=temp;
            show_Node(encoded.head);
            // if(curent_length!=lengths[j]){
            //     break;
            // }
        }
        j+=1;
        printf("j: %u\n",j);

        // Decoding
        encoded.tail=encoded.head;
        encoded.current_bit=0;
        for (int k = 0; k < j; k++) {
            printf("Decoding chunk %d...\n", k + 1);
            
            RLE_decoding(&encoded, lengths+k, &decoded, windows[k], true);
            show_Node(decoded.head);
        }

        // Verification
        printf("Verifying...\n");
        if (!lists_are_equal(&original, &decoded)) {
            printf("Test failed at %d bits!\n", i);
            printf("decoded last block %u\n",decoded.last_block_length);
            printf("original last block %u\n",original.last_block_length);
            return 1;
        } else {
            printf("Test passed for %d bits!\n", i);
        }

        // Clean up
        cleanupLinkedList(&original);
        cleanupLinkedList(&decoded);
    }

    printf("All tests are done!!!\n");
    return 0;
}
