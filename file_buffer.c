#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint16_t block_t;
#define MAX_BIT_SIZE 16

typedef struct Node {
    block_t data;
    struct Node* next;
} Node;

typedef struct LinkedList {
    Node* head;
    Node* tail;
    uint8_t current_bit;
    uint8_t last_block_length;
} LinkedList;


bool readBytesFromFile(const char* filename, LinkedList* list) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    block_t buffer;
    while (fread(&buffer, sizeof(block_t), 1, file)) {
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->data = buffer;
        newNode->next = NULL;

        if (!list->head) {
            list->head = newNode;
            list->tail = newNode;
        } else {
            list->tail->next = newNode;
            list->tail = newNode;
        }
    }

    // Determine the last byte length
    long fileSize = ftell(file);
    list->last_block_length = fileSize % (sizeof(block_t));
    if (list->last_block_length == 0) {
        list->last_block_length = sizeof(block_t);
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
    block_t buffer;

    while (current) {
        block_t buffer = current->data;
        fwrite(&buffer, sizeof(block_t), 1, file);
        current = current->next;
    }
    fwrite(&buffer, list->last_block_length/8, list->last_block_length, file); 

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

