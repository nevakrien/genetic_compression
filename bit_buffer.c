#ifndef BIT_BUFFER_C
#define BIT_BUFFER_C

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define BLOCK_BYTES 9
#define BLOCK_BITS (BLOCK_BYTES * 8)

typedef uint64_t bit_c_t;

typedef struct bit_buffer
{
	uint8_t** blocks;
	bit_c_t size;
} bit_buffer_t;


	bit_buffer_t* init_buffer(bit_c_t size) 
	{
		bit_buffer_t* buffer = (bit_buffer_t*) malloc(sizeof(bit_buffer_t));
		buffer->size = size;

		// Calculate the number of blocks needed.
		bit_c_t num_blocks = (size + BLOCK_BITS - 1) / BLOCK_BITS;

		buffer->blocks = (uint8_t**) malloc(num_blocks * sizeof(uint8_t*));

		for(bit_c_t i = 0; i < num_blocks; i++) 
	    {
			buffer->blocks[i] = (uint8_t*) calloc(BLOCK_BYTES, sizeof(uint8_t));
		}

		return buffer;
	}

void free_buffer(bit_buffer_t* buffer) 
{
	for(bit_c_t i = 0; i < (buffer->size + BLOCK_BITS - 1) / BLOCK_BITS; i++) 
    {
		free(buffer->blocks[i]);
	}
	free(buffer->blocks);
	free(buffer);
}

void write_to(bit_buffer_t* buffer, bit_c_t idx, bool value) 
{
	if (idx >= buffer->size) 
    {
		//fprintf(stderr, "Index out of bounds\n");
		return;
	}

	bit_c_t block_idx = idx / BLOCK_BITS;
	bit_c_t bit_idx = idx % BLOCK_BITS;

	if(value) 
    {
		buffer->blocks[block_idx][bit_idx / 8] |= (1 << (bit_idx % 8));
	} 
    else 
    {
		buffer->blocks[block_idx][bit_idx / 8] &= ~(1 << (bit_idx % 8));
	}
}
#include <stdlib.h>
bool read_from(bit_buffer_t* buffer, bit_c_t idx) 
{
	if (idx >= buffer->size) 
    {
		fprintf(stderr, "Index out of bounds operating new crash behvior\n");
		exit(0);
		return false;
	}

	bit_c_t block_idx = idx / BLOCK_BITS;
	bit_c_t bit_idx = idx % BLOCK_BITS;

	return (buffer->blocks[block_idx][bit_idx / 8] & (1 << (bit_idx % 8))) != 0;
}

bit_buffer_t* read_file_as_buffer(const char* filename) 
{
    FILE* file = fopen(filename, "rb");
    if (!file) 
    {
        fprintf(stderr, "Failed to open file for reading\n");
        return NULL;
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Calculate the number of blocks needed
    size_t num_blocks = file_size / BLOCK_BYTES;
    if (file_size % BLOCK_BYTES != 0) 
    {
        num_blocks++; // One more for the remaining bytes
    }

    bit_c_t buffer_size_bits = file_size * 8; // Size in bits

    bit_buffer_t* buffer = init_buffer(buffer_size_bits);

    size_t remaining_bytes = file_size;
    for (size_t i = 0; i < num_blocks; i++) 
    {
        size_t bytes_to_read = remaining_bytes < BLOCK_BYTES ? remaining_bytes : BLOCK_BYTES;
        fread(buffer->blocks[i], sizeof(uint8_t), bytes_to_read, file);
        remaining_bytes -= bytes_to_read;
    }

    fclose(file);
    return buffer;
}


bool write_buffer_to_file(bit_buffer_t* buffer, const char* filename) 
{
    FILE* file = fopen(filename, "wb");
    if (!file) 
    {
        fprintf(stderr, "Failed to open file for writing\n");
        return false;
    }

    // Calculate the number of full blocks and the size of the last block in bytes
    size_t full_blocks = buffer->size / BLOCK_BITS;
    size_t remaining_bits = buffer->size % BLOCK_BITS;
    size_t remaining_bytes = (remaining_bits + 7) / 8; // Convert bits to bytes, rounding up

    // Write full blocks to the file
    for(size_t i = 0; i < full_blocks; i++) 
    {
        fwrite(buffer->blocks[i], sizeof(uint8_t), BLOCK_BYTES, file);
    }

    // Write the remaining bytes, if any
    if (remaining_bytes > 0) 
    {
        fwrite(buffer->blocks[full_blocks], sizeof(uint8_t), remaining_bytes, file);
    }

    fclose(file);
    return true;
}

//untested
void reserve_buffer(bit_buffer_t* buffer,bit_c_t size) 
{	
	if(buffer->size>=size){
		return;
	}

	// Calculate the number of blocks needed.
	bit_c_t num_blocks=(size + BLOCK_BITS - 1) / BLOCK_BITS;
	bit_c_t num_existing_blocks = (buffer->size + BLOCK_BITS - 1) / BLOCK_BITS;
	bit_c_t num_new_blocks=num_blocks-num_existing_blocks;
	
	buffer->size=size;

	if(!num_new_blocks){
		return;
	}

	buffer->blocks = (uint8_t**) realloc(buffer->blocks,num_blocks * sizeof(uint8_t*));

	for(bit_c_t i = num_existing_blocks; i < num_blocks; i++) 
    {
		buffer->blocks[i] = (uint8_t*) calloc(BLOCK_BYTES, sizeof(uint8_t));
	}
}

void resize_buffer(bit_buffer_t* buffer, bit_c_t new_size) 
{
	// Calculate the number of blocks needed for the new size.
	bit_c_t num_new_blocks = (new_size + BLOCK_BITS - 1) / BLOCK_BITS;

	// If the new size is larger than the current size, we increase the size.
	if (new_size > buffer->size) 
	{
		reserve_buffer(buffer, new_size);
		return;
	}

	// Calculate the number of existing blocks.
	bit_c_t num_existing_blocks = (buffer->size + BLOCK_BITS - 1) / BLOCK_BITS;

	// Free blocks that are not needed after resizing.
	for (bit_c_t i = num_new_blocks; i < num_existing_blocks; i++) 
	{
		free(buffer->blocks[i]);
	}

	// Reallocate the blocks pointer array.
	buffer->blocks = (uint8_t**) realloc(buffer->blocks, num_new_blocks * sizeof(uint8_t*));

	// Update buffer's size.
	buffer->size = new_size;
}


#endif // BIT_BUFFER_C