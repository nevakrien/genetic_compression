#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_buffer.h"



bool readBytesFromFile(const char* filename, LinkedList* list) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    list->head = (Node*)malloc(sizeof(Node));

    if (!list->head) { // Memory allocation check
        fclose(file);
        return false;
    } 

    list->tail = list->head;

    bc_t read_size=fread(&(list->tail->data), 1, BLOCK_BYTES, file);

    //check for small file edge case
    if(read_size<BLOCK_BYTES){
            list->tail->next=NULL;
            list->last_block_length=8*read_size;
            fclose(file);
            return true;
        }    

    //main loop
    while (true) {
        list->tail->next = (Node*)malloc(sizeof(Node));
        if (!list->tail->next) { // Memory allocation check
            fclose(file);
            return false;
        }
        read_size=fread(&(list->tail->next->data), 1, BLOCK_BYTES, file);

        if(!read_size){
            free(list->tail->next);
            list->tail->next=NULL;
            list->last_block_length=MAX_BIT_SIZE;
            break;
        }
        list->tail = list->tail->next; 
        if(read_size<BLOCK_BYTES){
            list->tail->next=NULL;
            list->last_block_length=8*read_size;
            break;
        }    
    }

    fclose(file);
    return true;
}

bool writeBytesToFile(const char* filename, const LinkedList* list) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        return false;
    }

    Node* current = list->head;

    while (current->next) {
        fwrite(&current->data, 1, BLOCK_BYTES, file);
        current = current->next;
    }
    fwrite(&current->data, 1, list->last_block_length/8, file); 

    fclose(file);
    return true;
}

void cleanupLinkedList(LinkedList* list) {
    Node* current = list->head;
    while (current) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->tail = NULL;
}




LinkedList create_empty_list(){
   Node* head = (Node*)malloc(sizeof(Node));
   memset(head->data,0,BLOCK_BYTES);
   head->next=NULL;
   LinkedList list = {
       .head = head,
       .tail = head,
       .current_bit = 0,
       .last_block_length = 0
   };
   return list;
}

static bc_t room_finder(bc_t a, bc_t b, bc_t max) {
    a = 8 - a % 8;
    b = 8 - b % 8;
    
    bc_t ans = (a < b) ? a : b;
    ans = (ans < max) ? ans : max;

    return ans;
}


static void bitCopy(uint8_t *destByte, uint8_t srcByte, int destPos, int srcPos, int size) {
    // Create a mask for the source bits.
    uint8_t src_mask = ((0xFF >> (8 - size)) & 0xFF) << srcPos;
    uint8_t isolated_bits = srcByte & src_mask;

    // Shift to align with destination.
    int diff = destPos - srcPos;
    if(diff > 0) {
        isolated_bits <<= diff;
    } else {
        isolated_bits >>= (-diff);
    }

    // Create a mask for the destination bits.
    uint8_t dest_mask = ((0xFF >> (8 - size)) & 0xFF) << destPos;

    // Clear the destination bits first.
    *destByte &= ~dest_mask;

    // Now, combine the bits.
    *destByte |= isolated_bits;
}



bc_t pop_bits(LinkedList* list, const bc_t num, uint8_t* out, bool free_list){
    bc_t size = 0;

    while(size < num && list->tail){
        // If we've consumed all bits in this node
        if(list->current_bit == MAX_BIT_SIZE){
            list->current_bit = 0;

            if(free_list){
                Node* temp = list->tail->next;
                free(list->tail);
                //temp->next=NULL;
                list->tail = temp;
            }
            else{
                if(!list->tail->next){return size;}
                list->tail = list->tail->next;
            }
        }

        bc_t bits_to_pop=room_finder(list->current_bit,size,num - size);

        bitCopy(out+(size / 8),list->tail->data[list->current_bit / 8],(size % 8),(list->current_bit % 8),bits_to_pop);
        
        // Move pointers forward
        size += bits_to_pop;
        list->current_bit += bits_to_pop;
        
    }

    return size;
}

bool append_bits(LinkedList* list, const bc_t num, uint8_t* in) {
    if(num==0){return true;}
    bc_t pos = 0;

    while (pos < num) {

        // If we've filled up the current node
        if (list->last_block_length >= MAX_BIT_SIZE) {
            
            list->tail->next = (Node*)malloc(sizeof(Node));
            if (!list->tail->next) {
                return false;  // Memory allocation failure
            }
            memset(list->tail->next->data, 0, sizeof(list->tail->next->data)); // Initialize to 0
            list->tail = list->tail->next;
            list->tail->next= NULL;
   
            list->last_block_length = 0;
        } 

        bc_t bits_to_append=room_finder(list->last_block_length,pos,num - pos);
        bitCopy(list->tail->data+(list->last_block_length / 8),in[pos / 8],list->last_block_length%8,(pos % 8),bits_to_append);

        // Move pointers forward
        pos += bits_to_append;
        list->last_block_length += bits_to_append;

        
    }

    return true;
}

//untested:
bool lists_are_equal(LinkedList* A,LinkedList* B){
    if(A->last_block_length!=B->last_block_length){
        return false;
    }
    Node* a=A->head;
    Node* b=B->head;

    if(!a && !b){return true;}
    if(!a || !b){return false;}

    while(a->next && b-> next){
        if(memcmp(a->data,b->data,BLOCK_BYTES)!=0){
            return false;
        }
        a=a->next;
        b=b->next;
    }
    if(!a->next != !b->next){return false;}
    if(memcmp(a->data,b->data,BLOCK_BYTES)!=0){return false;}
    return true;
}

LinkedList copy_list(LinkedList a){
    LinkedList b=a;
    Node* cur=b.head;
    Node* n = (Node*)malloc(sizeof(Node));
    memcpy(n->data,cur->data,BLOCK_BYTES);
    b.head=n;
    b.tail=n;
    while(cur->next){
        n = (Node*)malloc(sizeof(Node));
        memcpy(n->data,cur->next->data,BLOCK_BYTES);
        b.tail->next=n;
        b.tail=n;

        cur=cur->next;
    }
    b.tail->next=NULL;
    return b;
    
}

// void pad_tail(LinkedList* list) {
//     if (!list || !list->tail) return;

//     bc_t current_byte = list->last_block_length / 8;
//     bc_t current_bit_in_byte = list->last_block_length % 8;

//     // Set the remaining bits in the current byte to 0
//     uint8_t mask = 0xFF << (8 - current_bit_in_byte);
//     list->tail->data[current_byte] &= mask;

//     // If there were remaining bits, move to the next byte
//     if (current_bit_in_byte) current_byte++;

//     // Set the rest of the bytes in the block to 0
//     memset(&list->tail->data[current_byte], 0, BLOCK_BYTES - current_byte);
// }

// // Helper function to create a new node with left-padding
// static Node* createNodeWithLeftPaddedCopy(uint8_t* data, bc_t num_bits) {
//     Node* newNode = (Node*)malloc(sizeof(Node));
//     // Left-padded copy
//     memcpy(newNode->data + (BLOCK_BYTES - (num_bits + 7) / 8), data, (num_bits + 7) / 8); 
//     newNode->next = NULL;
//     return newNode;
// }

// // Helper function to create a new node with right-padding
// static Node* createNodeWithRightPaddedCopy(uint8_t* data, bc_t num_bits) {
//     Node* newNode = (Node*)malloc(sizeof(Node));
//     // Right-padded copy
//     memcpy(newNode->data, data, (num_bits + 7) / 8);  
//     newNode->next = NULL;
//     return newNode;
// }

