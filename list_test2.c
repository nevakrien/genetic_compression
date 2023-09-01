#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "random.h"
#include "file_buffer.c"

#define TEST_SIZE 10000
#define CHUNK 11

uint8_t zero[BLOCK_BYTES]={0};

char* byte_to_binary(uint8_t byte) {
    static char bit_string[9];
    bit_string[8] = '\0';
    
    for (int i = 7; i >= 0; i--) {
        bit_string[7 - i] = (byte & (1 << i)) ? '1' : '0';
    }
    
    return bit_string;
}

int clutter_random(LinkedList* list,pcg32_random_t* rng){
    int sum=0;
    for(int i=0;i<CHUNK;i++){
        uint8_t data[BLOCK_BYTES];
        for(int m=0;m<BLOCK_BYTES;m++){
            data[m]=(uint8_t)pcg32_random_r(rng);
        }
        bc_t num=(bc_t)(pcg32_random_r(rng)%MAX_BIT_SIZE);
    
        sum+=num;
        append_bits(list,num,data);
    }
    return sum;
}

int trash_random(LinkedList* list,pcg32_random_t* rng,int num){
    int sum=0;
    while(sum-num){
        uint8_t dump[BLOCK_BYTES] = {0};

        bc_t to_pop=(bc_t)pcg32_random_r(rng)%MAX_BIT_SIZE+1;
        to_pop = to_pop>(num-sum)? num-sum :to_pop;
        sum+=pop_bits(list,to_pop,dump,true);;
    }
    return sum;
}

void random_target(uint8_t* target,bc_t amount,pcg32_random_t* rng){
    //uint8_t target[BLOCK_BYTES]={0};
    int i=0;
    for(i;i<amount/8;i++){
        target[i]=pcg32_random_r(rng);
    }
    if(i<BLOCK_BYTES){
        bitCopy(target+i, pcg32_random_r(rng),0,0, amount%8);
    }
}

void test(bc_t i,pcg32_random_t* rng){
    //assert(i<=MAX_BIT_SIZE);
    bc_t amount=(i%MAX_BIT_SIZE)+1;
    //filling alist with junk payload junk
    LinkedList list = create_empty_list();
    int junk=clutter_random(&list,rng);
    

    uint8_t target[BLOCK_BYTES]={0};
    random_target(target,amount,rng);
    uint8_t payload[BLOCK_BYTES];//=target;
    memcpy(payload,target,BLOCK_BYTES);
    append_bits(&list,amount,payload);
    
    clutter_random(&list,rng);

    list.tail=list.head; 

    int trashed=trash_random(&list,rng,junk);
    assert(trashed==junk);

    memcpy(payload,zero,BLOCK_BYTES);
    bc_t poped=pop_bits(&list,amount,payload,true);
    assert(poped==amount);
    assert(memcmp(payload,target,BLOCK_BYTES)==0);

    list.head=list.tail;
    cleanupLinkedList(&list);
}

int main() {
    // Initialize random generator
    pcg32_random_t rng = get_rng();

    for(int i=0;i<TEST_SIZE;i++){
        printf("testing: %d\n",i);
        test(i,&rng);
    }
    
    printf("all test passed!!!\n");

    return 0;
}
