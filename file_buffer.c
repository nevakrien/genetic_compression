#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//we are going to be using some padding 
//note that pointers are 8 bytes so its actuly still saving space
#define BLOCK_BYTES 9
#define MAX_BIT_SIZE 8*BLOCK_BYTES

typedef uint16_t bc_t; //bit counter type

typedef struct Node {
    uint8_t data[BLOCK_BYTES];
    struct Node* next;
} Node;

typedef struct LinkedList {
    Node* head;
    Node* tail; //this is not necesserly the end of the list but the curent area we r working from
    bc_t current_bit;
    bc_t last_block_length;
} LinkedList;



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


//untested:

void pad_tail(LinkedList* list) {
    if (!list || !list->tail) return;

    bc_t current_byte = list->last_block_length / 8;
    bc_t current_bit_in_byte = list->last_block_length % 8;

    // Set the remaining bits in the current byte to 0
    uint8_t mask = 0xFF << (8 - current_bit_in_byte);
    list->tail->data[current_byte] &= mask;

    // If there were remaining bits, move to the next byte
    if (current_bit_in_byte) current_byte++;

    // Set the rest of the bytes in the block to 0
    memset(&list->tail->data[current_byte], 0, BLOCK_BYTES - current_byte);
}


bc_t pop_bits(LinkedList* list,const bc_t num,uint8_t* out,bool free_list){
    bc_t size=0;
    while(size<num){
        while(list->current_bit<MAX_BIT_SIZE){
            //assining the bit to out
            uint8_t bit=((list->tail->data[list->current_bit/8]>>(list->current_bit%8))& 0x01);
            out[size/8]|=(bit*(0x01>>(size%8)));
            list->current_bit++;
            size++;

            if(size==num){
                return size;
            }
        }
        
        //moving block
        list->current_bit=0;
        if(free_list){
            Node* temp=list->tail->next;
            free(list->tail);
            list->tail =temp;
        }
        else{
            list->tail=list->tail->next;
        }

        if(!list->tail){
            return size;
        }

    }

    return size;
} 

bool append_bits(LinkedList* list,const bc_t num,uint8_t* in){
    bc_t pos=0;
    
    while(pos<num){
        while(list->current_bit<MAX_BIT_SIZE){
            uint8_t bit=(in[pos/8]>>(pos%8))& 0x01;
            list->tail->data[list->current_bit/8]|=(bit*(0x01>>(pos%8)));
            list->current_bit++;
            pos++;

            if(pos==num){
                return true;
            }
        }

        list->current_bit=0;
        list->tail->next = (Node*)malloc(sizeof(Node));
            if (!list->tail->next) { // Memory allocation check
                return false;
            }
        list->tail = list->tail->next;
    } 

    return true;  
}

// Helper function to create a new node with left-padding
static Node* createNodeWithLeftPaddedCopy(uint8_t* data, bc_t num_bits) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    // Left-padded copy
    memcpy(newNode->data + (BLOCK_BYTES - (num_bits + 7) / 8), data, (num_bits + 7) / 8); 
    newNode->next = NULL;
    return newNode;
}

// Helper function to create a new node with right-padding
static Node* createNodeWithRightPaddedCopy(uint8_t* data, bc_t num_bits) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    // Right-padded copy
    memcpy(newNode->data, data, (num_bits + 7) / 8);  
    newNode->next = NULL;
    return newNode;
}

