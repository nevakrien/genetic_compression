#include "bit_buffer.c"

//each function comes with a reserve instruction. make sure to have X+reserve memory in the new destivation; 

//worse casenario sizes PAD:X+10 UNPAD:X-3 RLE_ENCODE(window):X*(window+1)/window RLE_DECODE(window)#:(X-window)*window

//(reserve 10) modifys the original
uint8_t PAD(bit_buffer_t* buffer,bit_c_t size){
	uint8_t pad=8-((size)%8); //needs more work?
	if(pad<3){
		pad+=8;
	}
	size+=pad-1; //minus 1 is just so we hit middle of the byte always
	uint8_t* padding_place= buffer->blocks[size/ BLOCK_BITS]+((size%BLOCK_BITS)/8);
	//put the padding in;
	// *padding_place=*padding_place>>3<<3;//0xF8;
	*padding_place &= 0x1F;
	*padding_place|=(pad-3)<<5;
	return pad;
}

//(no reserve)
uint8_t UNPAD(bit_buffer_t* buffer, bit_c_t size) {
    size -= 2; // Hitting the middle bit of the padding number
    uint8_t* padding_place = buffer->blocks[size / BLOCK_BITS] + ((size % BLOCK_BITS) / 8);
    uint8_t pad = (*padding_place) >> 5;
    return pad + 3;  // Adjusting for the offset of 3 used in PAD
}

typedef uint16_t window_t; 
static bool window_equal(bit_buffer_t* in,window_t window,bit_c_t first){
	for(bit_c_t i=first;i<first+window;i++){
		if(read_from(in,i)!=read_from(in,i+window)){
			return false;
		}
	}
	return true;
}

//reserve(X/window)
bit_c_t RLE_encode(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos,window_t window){
	bool pending_write=false;
	bit_c_t i;
	//printf("Encode:\nloop from %lu to %lu:\n",start,end);
	for(i=start;i+2*window<end+1;i+=window){ //made a read of uinitilized memory 
		//printf("\nstarting loop with %lu\n",i);

		if(window_equal(in,window,i)){
			//printf("equal\n");
			pending_write=true;
			write_to(out,pos,false);
			//printf("writing 0 to %lu\n",pos);
			pos+=1;
		}
		else{
			//printf("diffrent\n");
			pending_write=false;
			write_to(out,pos,true);
			//printf("writing 1 to %lu\n",pos);
			pos+=1;
			for(bit_c_t k=i;k<i+window;k++){
				write_to(out,pos,read_from(in,k));
				pos+=1;
			}
		}
	}
	//printf("ended loop with %lu\n",i);
	if(pending_write){
		//printf("pending write\n");
		//!!!!!!! write_to(out, pos-1, true);
		write_to(out, pos, true);
		//printf("writing 1 to %lu\n",pos);
		pos+=1;		
		for(bit_c_t cap=i+window; i<cap; i++){
			write_to(out, pos, read_from(in, i));
			//printf("writing %u to %lu\n",read_from(in, i),pos);
			pos+=1;
		}
	}
	else{
		if(i+window<end){
			//printf("adding 1 to %lu\n",pos);
			write_to(out, pos, true);
			pos+=1;
		}
	}
	//printf("remainder\n");
	for(; i<end; i++){
		write_to(out, pos, read_from(in, i));
		//printf("writing %u to %lu\n",read_from(in, i),pos);
		pos+=1;
	}
	//printf("done encoding\n\n");
	return pos;
} 

//reserve total:((X-1)*window)
bit_c_t RLE_decode(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos,window_t window){
	bit_c_t run=0;
	bit_c_t i=start;
	//printf("\n\nDecode\nloop from %lu to %lu:\n",start,end);
	while(i+window<end){//window+1
		//printf("\nstarting loop with %lu\n",i);
		//printf("read: %u from %lu\n",read_from(in, i),i);
		run+=1;
		if(read_from(in, i)){
			i+=1;
			//printf("writing %lu bits\n",(window)*run);
			//write run times the next window tokens.
			for(bit_c_t cap=i+window;i<cap;i++){
				bool temp=read_from(in,i);
				//printf("read %u from %lu\n",temp,i);
				for(bit_c_t k=0;k<run; k++){
					write_to(out,pos+k*window,temp);
					//printf("writing %u to %lu\n",temp,pos+k);
				}
				pos+=1;
			}
			//printf("done writing with %lu\n",i);
			pos+=window*(run-1);
			run=0;
		}
		else{
			i+=1;
		}
	}
	//printf("ended loop with %lu\n",i);
	if(run!=0){
		//printf("still have run: %lu\n",run);
		//write run times the next window tokens.
		for(bit_c_t cap=i+window;i<cap;i++){
			bool temp=read_from(in,i);
			for(bit_c_t k=0;k<run; k++){
				write_to(out,pos+k,temp);
				//printf("writing %u to %lu\n",temp,pos+k);
			}
			pos+=1;
		}
		pos+=window*(run-1);
		run=0;
	}

	//printf("final remainder\n");
	for(; i<end; i++){
		write_to(out, pos, read_from(in, i));
		//printf("writing %u to %lu\n",read_from(in, i),pos);
		pos+=1;
	}
	//printf("done decoding\n\n");
	return pos;
}
