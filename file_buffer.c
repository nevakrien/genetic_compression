#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// typedef uint8_t[] block_t;
#define BLOCK_BYTES 9
#define MAX_BIT_SIZE 8*BLOCK_BYTES

typedef uint16_t bc_t; //bit counter type

typedef struct Node {
    uint8_t data[BLOCK_BYTES];
    struct Node* next;
} Node;

typedef struct LinkedList {
    Node* head;
    Node* tail;
    bc_t current_bit;
    bc_t last_block_length;
} LinkedList;


bool readBytesFromFile(const char* filename, LinkedList* list) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    list->head = (Node*)malloc(sizeof(Node));;
    list->tail = list->head;

    bc_t read_size=fread(&(list->tail->data), 1, BLOCK_BYTES, file);

    while (read_size) {
        list->tail->next = (Node*)malloc(sizeof(Node));
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

