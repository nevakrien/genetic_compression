#include <stdint.h> 
#include <stdlib.h>
#include <string.h>
#include "file_buffer.h"


//made to be used with new lists and before outputing
uint32_t RLE_encoding(LinkedList* in,uint32_t* length,LinkedList* out,uint8_t window,bool free_in){
	uint8_t zero=0;
	uint8_t one=1;

	uint8_t size=(window+7)/8;

	uint8_t next[size];
	uint8_t curent[size];
	memset(next,0,size);
	memset(curent,0,size);

	
	bc_t poped=pop_bits(in,window,next,free_in);
	*length-=poped;
	memcpy(curent,next,size); 


	while((poped==window) && window<=*length){
		poped=pop_bits(in,window,next,free_in);
		*length-=poped;
		if(memcmp(next,curent,size)==0){
			append_bits(out, 1, &zero);
		}

		else{
			
			append_bits(out, 1, &one);
			append_bits(out,window,curent);
			memcpy(curent,next,size); 
		}
	}
	//wright the last run token
	append_bits(out, 1, &one);
	append_bits(out,window,curent); 

	//remainder
	append_bits(out,poped,next);
	poped=pop_bits(in,*length,next,free_in);
	*length-=poped;
	append_bits(out,poped,next);

	return 0; 
} 

uint32_t RLE_decoding(LinkedList* in,uint32_t* length,LinkedList* out,uint8_t window,bool free_in){
	uint8_t size=(window+7)/8;

	uint8_t data[size];
	memset(data,0,size);

	uint8_t keep=0;
	uint64_t run=0;

	bc_t poped; 

	while(!keep){
		poped=pop_bits(in,1,&keep,free_in);
		*length-=poped;
		if(!poped){
			return 0;
		}
		run+=1;
	}
	if(*length<window){
		//do something
	}

	else{
		//should always be window
		poped=pop_bits(in,window,data,free_in);
		*length-=poped;
		for(int i=0;i<run;i++){
			append_bits(out,poped,data);
		}
	}
	
}