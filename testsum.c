#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdint.h>

#define MAXLINELENGTH 1000

typedef union {
    struct {
        int32_t offset: 16;
        uint32_t regB: 3;
        uint32_t regA: 3;
        uint32_t opcode: 3;
        uint32_t empty: 7;
    } field;
    int32_t intRepresentation; 
}  bitset;

/* Function to convert a decinal number to binary number */
int32_t decimalToBinary(int32_t n)
{
    int32_t remainder;
    int32_t binary = 0, i = 1;

    while (n != 0)
    {
        remainder = n % 2;
        n = n / 2;
        binary = binary + (remainder * i);
        i = i * 10;
    }
}

int32_t main(int32_t argc, char *argv[])
{
	bitset inst1;
	inst1.intRepresentation = 16842754;

	// printf ("opcode = %d\n", inst1.field.opcode);
	printf ("opcode = %d | %d\n", inst1.field.opcode, decimalToBinary(inst1.field.opcode));
	printf("reg A = %d\n", inst1.field.regA);
	printf("reg B = %d\n", inst1.field.regB);
	printf("offset = %d\n", inst1.field.offset);
	return 0;
}