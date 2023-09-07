#include "funcs.c"
#include "bit_buffer.c"
#include "random.h"

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#define BUFFER_SIZE_BITS  BLOCK_BITS*7 + 3
#define WINDOW_SIZE 64*1000 //uint 16 max = 65535

bit_buffer_t* random_buffer(bit_c_t size,pcg32_random_t* rng){
	bit_buffer_t* buffer =init_buffer(size);
	for(bit_c_t i=0;i<size;i++){
		write_to(buffer,i,(bool)(pcg32_random_r(rng)%2));
	}

	return buffer;
}

void show_buffer(bit_buffer_t* buffer,bit_c_t size){
	for(bit_c_t i=0;i<size;i++){
		if(read_from(buffer,i)){
			printf("1");
		}
		else{
			printf("0");
		}
	}
	printf("\n");
}

void test_rle(bit_c_t N,pcg32_random_t* rng){
	bit_buffer_t* original=random_buffer(N,rng);
	//show_buffer(original,N);
	bit_buffer_t* encoded =init_buffer(N);
	bit_buffer_t* decoded =init_buffer(N);

	for(window_t window=1;window<N+2;window++){
		printf("window: %d\n",window);

		resize_buffer(encoded,N+N/window);
		bit_c_t encoded_size =RLE_encode(original,0,N,encoded,0, window);
		bit_c_t decoded_size =RLE_decode(encoded, 0,encoded_size,decoded, 0,window);
		
		//show_buffer(original,N);
		//show_buffer(encoded, encoded_size);
		//show_buffer(decoded,decoded_size);

		assert(decoded_size==N);

		for(bit_c_t i=0;i<N;i++){
			assert(read_from(original,i)==read_from(decoded,i));
		}
	}
	free_buffer(original);
	free_buffer(encoded);
	free_buffer(decoded);
}

int main() {

    pcg32_random_t rng = get_rng();
    for (int i = 0; i < BUFFER_SIZE_BITS; i++) {
    	printf("testing: %d\n",i);
    	//fflush(stdout);
        test_rle(i,&rng);
    }

    printf("All tests passed successfully!\n");
    return 0;
}