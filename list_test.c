#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "random.h"
#include "file_buffer.c"

//#define TEST_SIZE 10000
#define CHUNK 10

int clutter_random(LinkedList* list,pcg32_random_t* rng){
    int sum=0;
    for(int i=0;i<CHUNK;i++){
        uint8_t data[BLOCK_BYTES];
        for(int m=0;m<BLOCK_BYTES;m++){
            data[m]=(uint8_t)pcg32_random_r(rng);
        }
        bc_t num=(bc_t)(pcg32_random_r(rng)%MAX_BIT_SIZE);
        sum+=append_bits(list,num,data);
    }
    return sum;
}

int trash_random(LinkedList* list,pcg32_random_t* rng,int num){
    int sum=0;
    for(int i=0;i<CHUNK;i++){
        uint8_t dump[BLOCK_BYTES];

        bc_t to_pop=(bc_t)pcg32_random_r(rng);
        to_pop = to_pop>(num-sum)? num-sum :to_pop;
        pop_bits(list,to_pop,dump,false);
        sum+=to_pop;
    }
    return sum;
}

int main() {
    // Initialize random generator
    pcg32_random_t rng = get_rng();

    //filling alist with junk payload junk
    LinkedList list = create_empty_list();
    printf("first clutter\n");
    fflush(stdout);
    int junk=clutter_random(&list,&rng);
    
    uint8_t target=1;
    uint8_t payload=target;
    append_bits(&list,8,&payload);
    
    printf("second clutter\n");
    fflush(stdout);
    clutter_random(&list,&rng); 

    list.tail=list.head; 
    printf("trashing\n");
    fflush(stdout);
    int trashed=trash_random(&list,&rng,junk);
    assert(trashed==junk);
    
    printf("poping payload\n");
    fflush(stdout);
    bc_t poped=pop_bits(&list,8,&payload,false);
    assert(poped==8);
    assert(payload==target);

    cleanupLinkedList(&list);

    return 0;
}
