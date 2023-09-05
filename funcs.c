#include <stdint.h> 
#include <stdlib.h>
#include <string.h>
#include "file_buffer.h"
#include "debug.c"

//untested:
typedef uint32_t l_t;
#define MAX_WINDOW 30//(256-7) max
//made to be used with new lists and before outputing
l_t RLE_encoding(LinkedList* in,l_t* length,LinkedList* out,uint8_t window,bool free_in){

	l_t ans=0;
	uint8_t zero=0;
	uint8_t one=1;

	uint8_t size=(window+7)/8;
	printf("size: %u",size);

	uint8_t next[size];
	uint8_t curent[size];
	memset(next,0,size);
	memset(curent,0,size);

	bool pending_write=false;
	
	bc_t poped=pop_bits(in,window,next,free_in);
	printf("poped:%u\n",poped);
	*length-=poped;
	

	if(poped!=window){
			append_bits(out,poped,next);
			printf("exited first check\n");
			return poped;
		}

	memcpy(curent,next,size);

	int compares=0;

	while(window<=*length){
		// memset(next,0,size); //shouldnt be needed
		poped=pop_bits(in,window,next,free_in);
		*length-=poped;
		if(poped!=window){
			if(pending_write){
				//wright the last run token
				append_bits(out, 1, &one);
				append_bits(out,window,curent); 
				ans+=1+window;
			}
			//wright the remainder
			append_bits(out,poped,next);
			ans+=poped;
			printf("exited because of pop:%d\n",poped);
			return ans; 

		}
		printf("compare: %d next: ", compares);
		for(int i = 0; i < size; i++) {
		    printf("%02x ", next[i]);
		}
		printf("curent: ");
		for(int i = 0; i < size; i++) {
		    printf("%02x ", curent[i]);
		}
		printf("\n");

		compares+=1;
		if(memcmp(next,curent,size)==0){
			pending_write=true;
			append_bits(out, 1, &zero);
			ans+=1;
		}

		else{
			pending_write=false;
			append_bits(out, 1, &one);
			append_bits(out,window,curent);
			ans+=1+window;
			memcpy(curent,next,size); 
		}
		
	}

	if(pending_write){
		//wright the last run token
		append_bits(out, 1, &one);
		append_bits(out,window,curent); 
		ans+=1+window;
	}
	
	//remainder
	poped=pop_bits(in,*length,next,free_in);
	*length-=poped;
	append_bits(out,poped,next);
	ans+=poped;

	printf("exited because of length\n");
	return ans; 
} 

l_t RLE_decoding(LinkedList* in, l_t* length, LinkedList* out, uint8_t window, bool free_in) {
    printf("length: %u \n",*length );
    l_t ans = 0;
    l_t run=0;
    uint8_t zero = 0;
    uint8_t one = 1;

    uint8_t size = (window + 7) / 8;

    uint8_t next[size];
    uint8_t curent[size];
    memset(next, 0, size);

    bc_t poped;
    while (window+1 <= *length) {
        poped = pop_bits(in, 1, next, free_in);
        if(!poped){
        	printf("wtf!!! \n" );
        	printf("length: %u \n",*length );
        	return ans;
        }
        *length -= poped;

        if (!(next[0] & 1)) {
        	printf("zero continuing\n");
            run+=1;
        } else {
        	//func
        	printf("one droping  ");
            poped = pop_bits(in, window, next, free_in);
            *length -= poped;
            printf("length: %u \n",*length );
            show_arr(next,size);
            for(l_t i=0;i<run+1;i++){
	            append_bits(out, window, next);  
            }
            ans += window*run;
            run=0;
        }
    }

    // Remainder
    poped = pop_bits(in, *length, next, free_in);
    *length -= poped;
    printf("length: %u \n",*length );
    show_arr(next,size);
    append_bits(out, poped, next);
    ans += poped;
    return ans;
}

l_t padded_length(l_t x){
	int padding=8-(x%8)-3;//3 bits for the padding counter;
	if(padding<0){
		padding+=8;
	}
	return x+padding+3;
}

//tested:
void PAD(LinkedList* in){
	LinkedList copy=*in;
	while(copy.tail->next){
		copy.tail=copy.tail->next;
	} 
	//printf("last block:%d   ",copy.last_block_length);
	int padding=8-(copy.last_block_length%8)-3;
	//printf("padding number:%d",padding);

	uint8_t stock=0;
	if(padding<0){
		padding+=8;
		//printf("  changed padding number:%d",padding);
		// append_bits(&copy,8, &stock);
	}
	//printf("\n");
	append_bits(&copy,padding, &stock);
	stock=padding;
	append_bits(&copy,3, &stock);
	in->last_block_length=copy.last_block_length;
	//return padding+3;
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
	//printf("last_block_length :%d   ",copy.last_block_length);
	if(copy.last_block_length%8)return false;
	
	Node* prelast=NULL;
	while(copy.tail->next){
		prelast=copy.tail;
		copy.tail=copy.tail->next;
	}

	//finding the padding
	int idx=copy.last_block_length-3;
	
	//printf("initial idx:%d",idx);
	if(idx<0) return false; //idx should always be at least 4 in all valid blocks
	

	copy.current_bit=idx;
	uint8_t padding=0;
	pop_bits(&copy,3,&padding ,false);

	//setting
	idx-=padding;
	//printf("   minus padding:%d",idx);
	if(idx<=0){
		//this is where the bug is at
		if(!prelast){
			prelast=get_prelast(&copy);
		}
		copy.tail=prelast;
		if(in->tail==prelast->next){
			in->tail=prelast;
		}
		free(prelast->next);
		prelast->next=NULL;
		idx+=MAX_BIT_SIZE;
		padding-=8;
		//printf("  fixed overflow:%d",idx);
	}

	//printf("\n");
	in->last_block_length=idx;

	//reseting the pad to 0 
	copy.last_block_length=idx; 
	padding+=3; //acounting for the 3 bits that right the pading
	uint8_t zero[2]={0};
	//printf("num zeros:%d\n",padding);
	append_bits(&copy, padding, zero);
	return true;
}