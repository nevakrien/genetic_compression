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
   head->next=NULL;
   LinkedList list= {head,head,0,0};
   return list;
}

bc_t pop_bits(LinkedList* list, const bc_t num, uint8_t* out, bool free_list){
    bc_t size = 0;

    while(size < num && list->tail){
        bc_t available_bits = MAX_BIT_SIZE - list->current_bit;
        bc_t bits_to_pop = available_bits < (num - size) ? available_bits : (num - size);
        
        // Create a mask to isolate the bits we want
        uint8_t mask = ((1 << bits_to_pop) - 1) << list->current_bit;
        
        // Extract the bits
        uint8_t extracted_bits = (list->tail->data[list->current_bit / 8] & mask) >> list->current_bit;

        // Assign bits to the out array
        out[size / 8] |= extracted_bits << (size % 8);
        
        // Move pointers forward
        size += bits_to_pop;
        list->current_bit += bits_to_pop;
        
        // If we've consumed all bits in this node
        if(list->current_bit >= MAX_BIT_SIZE){
            list->current_bit = 0;

            if(free_list){
                Node* temp = list->tail->next;
                free(list->tail);
                list->tail = temp;
            }
            else{
                list->tail = list->tail->next;
            }
        }
    }

    return size;
}

bool append_bits(LinkedList* list, const bc_t num, uint8_t* in) {
    bc_t pos = 0;

    while (pos < num) {
        if (!list->tail) {
            list->tail = (Node*)malloc(sizeof(Node));
            if (!list->tail) {
                return false;
            }
            //memset(list->tail->data, 0, sizeof(list->tail->data)); // Initialize to 0
            list->tail->next = NULL;
        }

        bc_t space_left = MAX_BIT_SIZE - list->last_block_length;
        bc_t bits_to_append = space_left < (num - pos) ? space_left : (num - pos);

        // Create a mask to isolate the bits we want from 'in'
        uint8_t mask = (1 << bits_to_append) - 1;
        uint8_t bits = (in[pos / 8] >> (pos % 8)) & mask;

        // Insert the bits into the list's tail data
        list->tail->data[list->last_block_length / 8] |= bits << list->last_block_length;

        // Move pointers forward
        pos += bits_to_append;
        list->last_block_length += bits_to_append;

        // If we've filled up the current node
        if (list->last_block_length >= MAX_BIT_SIZE) {
            // if (list->tail->next) {
            //     list->tail = list->tail->next;             
            // } 
            // else {
                list->tail->next = (Node*)malloc(sizeof(Node));
                if (!list->tail->next) {
                    return false;  // Memory allocation failure
                }
                //memset(list->tail->next->data, 0, sizeof(list->tail->next->data)); // Initialize to 0
                list->tail->next->next = NULL;
                list->tail = list->tail->next;
            // }
            list->last_block_length = 0;
        }
    }

    return true;
}

//untested:

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

