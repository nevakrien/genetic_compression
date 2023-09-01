#ifndef FILE_BUFFER_H
#define FILE_BUFFER_H

#include <stdint.h>
#include <stdbool.h> 

//we are going to be using some padding 
//note that pointers are 8 bytes so its actuly still saving space
#define BLOCK_BYTES 9 //at least 2
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


bool readBytesFromFile(const char* filename, LinkedList* list); 
bool writeBytesToFile(const char* filename, const LinkedList* list);
void cleanupLinkedList(LinkedList* list);
LinkedList create_empty_list();
//void pad_tail(LinkedList* list);
bc_t pop_bits(LinkedList* list, const bc_t num, uint8_t* out, bool free_list);
bool append_bits(LinkedList* list, const bc_t num, uint8_t* in);

//untested
bool lists_are_equal(LinkedList* A,LinkedList* B);
LinkedList copy_list(LinkedList a);

#endif //FILE_BUFFER_H