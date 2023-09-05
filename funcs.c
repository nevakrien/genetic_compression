#include "bit_buffer.c"

//modifys the original
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

uint8_t UNPAD(bit_buffer_t* buffer, bit_c_t size) {
    size -= 2; // Hitting the middle bit of the padding number
    uint8_t* padding_place = buffer->blocks[size / BLOCK_BITS] + ((size % BLOCK_BITS) / 8);
    uint8_t pad = (*padding_place) >> 5;
    return pad + 3;  // Adjusting for the offset of 3 used in PAD
}
