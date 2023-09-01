#include <stdint.h> 
#include <stdlib.h>
#include <string.h>
#include "file_buffer.h"

//untested:

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

void PAD(LinkedList* in){
	LinkedList copy=*in;
	while(copy.tail->next){
		copy.tail=copy.tail->next;
	} 
	printf("last block:%d   ",copy.last_block_length);
	int padding=8-(copy.last_block_length%8)-3;
	printf("padding number:%d",padding);

	uint8_t stock=0;
	if(padding<0){
		padding+=8;
		printf("  changed padding number:%d",padding);
		// append_bits(&copy,8, &stock);
	}
	printf("\n");
	append_bits(&copy,padding, &stock);
	stock=padding;
	append_bits(&copy,3, &stock);
	in->last_block_length=copy.last_block_length;
}
//mpt done:
static Node* get_prelast(LinkedList* copy){
	Node* prelast=copy->head;
	copy->tail=prelast;

	while(copy->tail->next){
		prelast=copy->tail;
		copy->tail=copy->tail->next;
	}
	return prelast;
}

//should return a zero padded list instead keeps the original data as it was...
//WARNING!!! this function depends on the specific implementation of append_bits
bool UNPAD(LinkedList* in){
	LinkedList copy=*in;
	
	//exceptions
	if(copy.last_block_length%8)return false;
	
	Node* prelast=NULL;
	while(copy.tail->next){
		prelast=copy.tail;
		copy.tail=copy.tail->next;
	}

	//finding the padding
	int idx=copy.last_block_length-3;
	
	if(idx<0) return false; //idx should always be at least 4 in all valid blocks
	printf("initial idx:%d",idx);

	copy.current_bit=idx;
	uint8_t padding=0;
	pop_bits(&copy,3,&padding ,false);

	//setting
	idx-=padding;
	printf("   minus padding:%d",idx);
	if(idx<0){
		if(!prelast){
			prelast=get_prelast(&copy);
		}
		copy.tail=prelast;
		in->tail=prelast;
		free(prelast->next);
		prelast->next=NULL;
		idx+=8;//MAX_BIT_SIZE;
		printf("  fixed overflow:%d",idx);
	}
	printf("\n");
	in->last_block_length=idx;
	//reseting the pad to 0 
	copy.current_bit=idx; 
	padding+=3; //acounting for the 3 bits that right the pading
	uint8_t zero=255;
	printf("num zeros:%d\n",padding);
	append_bits(&copy, padding, &zero);
	return true;
}