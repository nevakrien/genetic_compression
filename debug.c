#ifndef DEBUG_C
#define DEBUG_C

#include "random.h"
char* byte_to_binary(uint8_t byte) {
    static char bit_string[9];
    bit_string[8] = '\0';
    
    for (int i = 7; i >= 0; i--) {
        bit_string[7 - i] = (byte & (1 << i)) ? '1' : '0';
    }
    
    return bit_string;
}

void show_arr(uint8_t* arr,uint64_t size){
	for(int i=0;i<size;i++){
            printf("%s\n",byte_to_binary(arr[i]));
        }
}

void show_Block(uint8_t* block){
	for(int i=0;i<BLOCK_BYTES;i++){
            printf("%s\n",byte_to_binary(block[i]));
        }
}

void show_Node(Node* node){
    // return;
    printf("\n");
    while(node){
        show_Block(node->data);
        node=node->next;
    }
    printf("\n");
}

// Create a function to initialize a list with specified data and length
LinkedList random_list(pcg32_random_t* rng,int length) {
    LinkedList list = create_empty_list();
    for (int i = 0; i < length/8; i += 1) {
        uint8_t byte = pcg32_random_r(rng)%256;
        //printf("rng: %d\n",byte);
        append_bits(&list, 8, &byte);
    }
    uint8_t extra=(length%8);
    if (extra){
        uint8_t byte = pcg32_random_r(rng)%(1<<extra); //
        append_bits(&list, extra, &byte);
    }

    return list;
}

#endif //DEBUG_C