#include "funcs.c"
#include "bit_buffer.c"
#include "random.h"

#include <stdbool.h>
#include <stdio.h>
#include <assert.h> 

#define TEST_SIZE 64*1000

bit_buffer_t* random_buffer(bit_c_t size,pcg32_random_t* rng){
	bit_buffer_t* buffer =init_buffer(size);
	for(bit_c_t i=0;i<size;i++){
		write_to(buffer,i,(bool)(pcg32_random_r(rng)%2));
	}

	return buffer;
}

void test_xor(bit_c_t N,pcg32_random_t* rng){
	bit_buffer_t* original=random_buffer(N,rng);
	//show_buffer(original,N);
	bit_buffer_t* encoded =init_buffer(N);
	bit_buffer_t* decoded =init_buffer(N);

	window_t state=pcg32_random_r(rng);
	window_t mul=pcg32_random_r(rng);
	window_t increment=pcg32_random_r(rng); 
	window_t period=1+(pcg32_random_r(rng)%100); 

	XOR(original,0,N,encoded,0,state,increment,mul,period);
	XOR(encoded,0,N,decoded,0,state,increment,mul,period);

	for(bit_c_t i=0;i<N;i++){
			assert(read_from(original,i)==read_from(decoded,i));
		}

	free_buffer(original);
	free_buffer(encoded);
	free_buffer(decoded);
}

int main() {

    pcg32_random_t rng = get_rng();
    for (int i = 0; i < TEST_SIZE; i++) {
    	printf("testing: %d\n",i);
    	//fflush(stdout);
        test_xor(i,&rng);
    }

    printf("All tests passed successfully!\n");
    return 0;
}