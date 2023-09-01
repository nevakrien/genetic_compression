#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "random.h"
#include "file_buffer.c"

//#define TEST_SIZE 10000
#define CHUNK 11

char* byte_to_binary(uint8_t byte) {
    static char bit_string[9];
    bit_string[8] = '\0';
    
    for (int i = 7; i >= 0; i--) {
        bit_string[7 - i] = (byte & (1 << i)) ? '1' : '0';
    }
    
    return bit_string;
}
void show_Node(Node* node){
    // return;
    printf("\n");
    while(node){
        for(int i=0;i<BLOCK_BYTES;i++){
            printf("%s\n",byte_to_binary(node->data[i]));
        }
        node=node->next;
    }
    printf("\n");
}

int clutter_random(LinkedList* list,pcg32_random_t* rng){
    printf("appending\n");
    int sum=0;
    for(int i=0;i<CHUNK;i++){
        uint8_t data[BLOCK_BYTES];
        for(int m=0;m<BLOCK_BYTES;m++){
            data[m]=(uint8_t)pcg32_random_r(rng);
        }
        bc_t num=(bc_t)(pcg32_random_r(rng)%MAX_BIT_SIZE);
        printf("%d,  ",num);
        sum+=num;
        append_bits(list,num,data);
    }
    printf("\n");
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

int main() {
    // Initialize random generator
    pcg32_random_t rng = get_rng();

    //filling alist with junk payload junk
    LinkedList list = create_empty_list();
    printf("first clutter\n");
    fflush(stdout);
    int junk=clutter_random(&list,&rng);
    printf("junk:%d\n",junk);
    show_Node(list.head);
    
    printf("payload\n");
    uint8_t target=1;
    uint8_t payload=target;
    append_bits(&list,8,&payload);
    show_Node(list.head);
    
    printf("second clutter\n");
    fflush(stdout);
    clutter_random(&list,&rng);
    show_Node(list.head); 

    list.tail=list.head; 
    printf("trashing: %d\n",junk/MAX_BIT_SIZE);
    fflush(stdout);
    int trashed=trash_random(&list,&rng,junk);
    assert(trashed==junk);
    //assert(!list.tail);
    show_Node(list.tail);
    
    printf("poping payload\n");
    fflush(stdout);
    payload = 0;  // Reset before reusing
    bc_t poped=pop_bits(&list,8,&payload,true);
    assert(poped==8);
    assert(payload==target);

    printf("cleanup\n");
    list.head=list.tail;
    cleanupLinkedList(&list);
    printf("all test passed!!!\n");

    return 0;
}
