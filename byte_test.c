#include <stdio.h>
#include "file_buffer.c"  // or whatever the necessary include is to get the function prototypes

char* byte_to_binary(uint8_t byte) {
    static char bit_string[9];
    bit_string[8] = '\0';
    
    for (int i = 7; i >= 0; i--) {
        bit_string[7 - i] = (byte & (1 << i)) ? '1' : '0';
    }
    
    return bit_string;
}

// Tests...
void test_room_finder() {
    printf("Testing room_finder:\n");
    bc_t max = 8; // arbitrary maximum

    // Test Case 1:
    bc_t a1 = 2, b1 = 4;
    bc_t res1 = room_finder(a1, b1, max);
    printf("For a1=%hu, b1=%hu, max=%hu -> Expected: 4, Got: %hu\n", a1, b1, max, res1);

    // Test Case 2:
    bc_t a2 = 7, b2 = 5;
    bc_t res2 = room_finder(a2, b2, max);
    printf("For a2=%hu, b2=%hu, max=%hu -> Expected: 1, Got: %hu\n", a2, b2, max, res2);

    // Test Case 3:
    max = 3;
    bc_t a3 = 4, b3 = 4;
    bc_t res3 = room_finder(a3, b3, max);
    printf("For a3=%hu, b3=%hu, max=%hu -> Expected: 3, Got: %hu\n", a3, b3, max, res3);
}

void test_bitCopy() {
    printf("\nTesting bitCopy:\n");

    // Test Case 1:
    uint8_t dest1 = 0x00;
    uint8_t src1 = 0xFF;
    int destPos1 = 0, srcPos1 = 7, size1 = 1;
    bitCopy(&dest1, src1, destPos1, srcPos1, size1);
    
    printf("Test 1:\n");
    printf("dest (Initial): %s\n", byte_to_binary(0x00));
    printf("src:            %s\n", byte_to_binary(src1));
    printf("destPos: %d, srcPos: %d, size: %d\n", destPos1, srcPos1, size1);
    printf("Got: %s\n\n", byte_to_binary(dest1));

    // Test Case 2:
    uint8_t dest2 = 0x0F;
    uint8_t src2 = 0xF0;
    int destPos2 = 5, srcPos2 = 4, size2 = 3;
    bitCopy(&dest2, src2, destPos2, srcPos2, size2);

    printf("Test 2:\n");
    printf("dest (Initial): %s\n", byte_to_binary(0x0F));
    printf("src:            %s\n", byte_to_binary(src2));
    printf("destPos: %d, srcPos: %d, size: %d\n", destPos2, srcPos2, size2);
    printf("Got: %s\n\n", byte_to_binary(dest2));

    // Test Case 3:
    uint8_t dest3 = 0x28;
    uint8_t src3 = 0xA3;
    int destPos3 = 1, srcPos3 = 6, size3 = 2;
    bitCopy(&dest3, src3, destPos3, srcPos3, size3);

    printf("Test 3:\n");
    printf("dest (Initial): %s\n", byte_to_binary(0x28));
    printf("src:            %s\n", byte_to_binary(src3));
    printf("destPos: %d, srcPos: %d, size: %d\n", destPos3, srcPos3, size3);
    printf("Got: %s\n\n", byte_to_binary(dest3));
}




int main() {
    test_room_finder();
    test_bitCopy();
    printf("(0xFF  >> (8 - 3): %s\n",byte_to_binary((0xFF >>(8 - 3))));
    return 0;
}
