#include <stdio.h>
#include <assert.h>
#include "file_buffer.h"
#include "random.h"
#include "funcs.c"

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
        uint8_t byte = 255%(1<<extra); //pcg32_random_r(rng)
        append_bits(&list, extra, &byte);
    }

    list.last_block_length=length;
    return list;
}



int main() {
    pcg32_random_t rng= get_rng();
    LinkedList list;
    LinkedList list2;
    for(int i=62;i<MAX_BIT_SIZE*4;i++){
        
        printf("original:%d\n",i);
        list=random_list(&rng,i);
        //printf("copy:\n");
        list2=copy_list(list);
        show_Node(list.head);
        
        printf("padded:\n");
        PAD(&list);
        //printf("padded:\n");
        show_Node(list.head);

        printf("unpaaded:\n");
        if(!UNPAD(&list)){printf("errored!\n"); return 1;}
        show_Node(list.head);

        printf("equal:\n");
        if(!lists_are_equal(&list,&list2)){
            printf("failed %d\n",i);
            // printf("padded:\n");
            // show_Node(list.head);
            // printf("unpadded:\n");
            // show_Node(list2.head);
            return 1;
        }
        else{
            printf("passed %d\n",i);
        }
        printf("clean:\n");
        cleanupLinkedList(&list);
        cleanupLinkedList(&list2);
        
    }
    printf("All tests are done!!!\n");
    return 0;
}
