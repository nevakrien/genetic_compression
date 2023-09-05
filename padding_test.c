#include <stdio.h>
#include <assert.h>
#include "file_buffer.h"
#include "random.h"
#include "funcs.c"
#include "debug.c"

int main() {
    pcg32_random_t rng= get_rng();
    LinkedList list;
    LinkedList list2;
    for(int i=1;i<=MAX_BIT_SIZE*4;i++){
        
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
            printf("end A:%d end B:%d\n",list.last_block_length,list2.last_block_length);
            printf("start A:%d start B:%d\n",list.current_bit,list2.current_bit);
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
