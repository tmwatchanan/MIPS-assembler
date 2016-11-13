/* Assembler code fragment */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int decimalToBinary(int n);

/* Reads a file and returns the number of lines in this file. */
int countLines(FILE *file) {
    int lines = 0;
    int c;
    int last = '\n';
    while (EOF != (c = fgetc(file))) {
        if (c == '\n' && last != '\n') {
            ++lines;
        }
        last = c;
    }
    /* Reset the file pointer to the start of the file */
    rewind(file);
    return lines;
}

typedef union {
    struct {
        unsigned int offset: 16;
        unsigned int regB: 3;
        unsigned int regA: 3;
        unsigned int opcode: 3;
        unsigned int empty: 7;
    } field;
    int intRepresentation;
}  bitset;

typedef struct {
    char label[MAXLINELENGTH];
    char arg0[MAXLINELENGTH];
    bitset inst;
} instruction;

instruction instMem[65536];

int main(int argc, char *argv[])
{
    // printf("short is %d bits\n",     CHAR_BIT * sizeof( short )   );
    // printf("int is %d bits\n",       CHAR_BIT * sizeof( int  )    );
    // printf("long is %d bits\n",      CHAR_BIT * sizeof( long )    );
    // printf("long long is %d bits\n", CHAR_BIT * sizeof(long long) );

    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
         arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
               argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    int reg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    bitset testInst;
    int lines = 0;

    // printf("Total lines = %d\n", countLines(inFilePtr));

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);

    // printf("label\topcode\targ0\targ1\targ2\n");

    /* here is an example for how to use readAndParse to read a line from
    inFilePtr */
    while ( readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
        size_t size = sizeof (instMem[lines].label);
        strncpy(instMem[lines].label, label, size);
        instMem[lines].label[size - 1] = '\0';
        // printf("---> instMem[%d].label = %s ------\n", lines, instMem[lines].label);
        // printf("[LINE] = %d --------------\n", lines);
        /* reached end of file */
        // printf("%s\t%s\t%s\t%s\t%s\n", label, opcode, arg0, arg1, arg2);

        /* after doing a readAndParse, you may want to do the following to test the
            opcode */
        testInst.field.empty = 0; // bit 31-25 of all instructions are 
        // add (R-type format) 000
        if (strcmp(opcode, "add") == 0) {
            /* do whatever you need to do for opcode "add" */
            testInst.field.offset = atoi(arg2);
            testInst.field.regB = atoi(arg1);
            testInst.field.regA = atoi(arg0);
            testInst.field.opcode = 0;
        }
        // nand (R-type format) 001
        else if (strcmp(opcode, "nand") == 0) {
            /* do whatever you need to do for opcode "nand" */
            testInst.field.offset = atoi(arg2);
            testInst.field.regB = atoi(arg1);
            testInst.field.regA = atoi(arg0);
            testInst.field.opcode = 1;
        }
        // lw (I-type format) 010
        else if (strcmp(opcode, "lw") == 0) {
            /* do whatever you need to do for opcode "lw" */
            testInst.field.offset = atoi(arg2);
            testInst.field.regB = atoi(arg1);
            testInst.field.regA = atoi(arg0);
            testInst.field.opcode = 2;
        }
        // sw (I-type format) 011
        else if (strcmp(opcode, "sw") == 0) {
            /* do whatever you need to do for opcode "sw" */
            testInst.field.offset = atoi(arg2);
            testInst.field.regB = atoi(arg1);
            testInst.field.regA = atoi(arg0);
            testInst.field.opcode = 3;
        }
        // beq (I-type format) 100
        else if (strcmp(opcode, "beq") == 0) {
            /* do whatever you need to do for opcode "beq" */
            testInst.field.offset = atoi(arg2);
            testInst.field.regB = atoi(arg1);
            testInst.field.regA = atoi(arg0);
            testInst.field.opcode = 4;
        }
        // jalr (J-type format) 101
        else if (strcmp(opcode, "jalr") == 0) {
            /* do whatever you need to do for opcode "jalr" */
            testInst.field.offset = 0;
            testInst.field.regB = atoi(arg1);
            testInst.field.regA = atoi(arg0);
            testInst.field.opcode = 5;
        }
        // halt (O-type format) 110
        else if (strcmp(opcode, "halt") == 0) {
            /* do whatever you need to do for opcode "halt" */
            testInst.field.offset = 0;
            testInst.field.regB = 0;
            testInst.field.regA = 0;
            testInst.field.opcode = 6;
        }
        // noop (O-type format) 111
        else if (strcmp(opcode, "noop") == 0) {
            /* do whatever you need to do for opcode "noop" */
            testInst.field.offset = 0;
            testInst.field.regB = 0;
            testInst.field.regA = 0;
            testInst.field.opcode = 7;
        }
        // .fill (special format) for symbolic address and immediate
        else if (strcmp(opcode, ".fill") == 0) {
            char *endptr;
            char *number = arg0;
            strtol(number, &endptr, 10);
            if (*endptr == '\0')
            {
                testInst.intRepresentation = (int) atoi(arg0);
            }
            else
            {
                
            }
        }
        else exit(1);
        printf("%d\n", testInst.intRepresentation);
        ++lines;
    }
    return (0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
                 char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* reached end of file */
        return (0);
    }

    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]",
           opcode, arg0, arg1, arg2);
    return (1);
}

int isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return ( (sscanf(string, "%d", &i)) == 1);
}

/* Function to convert a decinal number to binary number */
int decimalToBinary(int n)
{
    int remainder;
    int binary = 0, i = 1;

    while (n != 0)
    {
        remainder = n % 2;
        n = n / 2;
        binary = binary + (remainder * i);
        i = i * 10;
    }
    return binary;
}