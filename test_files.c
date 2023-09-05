#include "bit_buffer.c"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

bool files_are_identical(const char* filename1, const char* filename2) 
{
    FILE* file1 = fopen(filename1, "rb");
    FILE* file2 = fopen(filename2, "rb");

    if (!file1 || !file2) 
    {
        if (file1) fclose(file1);
        if (file2) fclose(file2);
        return false;
    }

    bool identical = true;

    while (1) 
    {
        char byte1 = fgetc(file1);
        char byte2 = fgetc(file2);

        if (byte1 != byte2) 
        {
            identical = false;
            break;
        }

        if (byte1 == EOF || byte2 == EOF) 
            break;
    }

    fclose(file1);
    fclose(file2);
    return identical;
}

void test_read_write() 
{
    const char* original_file = "test_garbage/input.txt";
    const char* test_file = "test_garbage/output.txt";

    // 1. Read from original file
    bit_buffer_t* buffer = read_file_as_buffer(original_file);

    // 2. Write to a new file
    if (buffer) 
    {
        write_buffer_to_file(buffer, test_file);
        free_buffer(buffer); // Assuming you have a function to free the buffer
    }

    // 3. Compare the original file and the test output
    if (files_are_identical(original_file, test_file)) 
    {
        printf("Test passed: The files are identical!\n");
    } 
    else 
    {
        printf("Test failed: The files are different.\n");
    }
}

int main() 
{
    test_read_write();
    return 0;
}
