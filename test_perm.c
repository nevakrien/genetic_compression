#include "funcs.c"
#include "bit_buffer.c"
#include "random.h"

#include <stdbool.h>
#include <stdio.h>
#include <assert.h> 

#define TEST_SIZE 1000

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

void test_xor(bit_c_t N,window_t window,pcg32_random_t* rng){
	bit_buffer_t* original=random_buffer(N,rng);
	//show_buffer(original,N);
	bit_buffer_t* encoded =init_buffer(N);
	bit_buffer_t* decoded =init_buffer(N);

	window_t max=pcg32_random_r(rng);
	window_t min=pcg32_random_r(rng); 
	//window_t window=1+(pcg32_random_r(rng)%100); 
	//printf("window %u\n",window);

	CONDITIONAL_PERMUTE(original,0,N,encoded,0,window, max, min);
	CONDITIONAL_PERMUTE(encoded,0,N,decoded,0,window, max, min);

	// show_buffer(original,N);
	// show_buffer(encoded, N);
	// show_buffer(decoded,N);

	for(bit_c_t i=0;i<N;i++){
			assert(read_from(original,i)==read_from(decoded,i));
		}

	free_buffer(original);
	free_buffer(encoded);
	free_buffer(decoded);
}

int main() {

    pcg32_random_t rng = get_rng();
    for (int i = 10; i < TEST_SIZE; i++) {
    	printf("testing: %d\n",i);
    	//fflush(stdout);
    	for(window_t w=1; w<=i+2;w++){
    		printf("window: %u\n",w);
    		test_xor(i,w,&rng);
    	}
        
    }

    printf("All tests passed successfully!\n");
    return 0;
}