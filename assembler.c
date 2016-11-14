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
    unsigned int uintOffset: 16;
    int intOffset;
} offset;

typedef union {
    struct {
        int offset: 16;
        unsigned int regB: 3;
        unsigned int regA: 3;
        unsigned int opcode: 3;
        unsigned int empty: 7;
    } field;
    int intRepresentation;
}  bitset;

typedef struct {
    char label[MAXLINELENGTH];
    char opcode[MAXLINELENGTH];
    char arg0[MAXLINELENGTH];
    char arg1[MAXLINELENGTH];
    char arg2[MAXLINELENGTH];
    bitset inst;
} instruction;

instruction instMem[65536];

#define OPCODE_NUM 8
char possibleOpcode[OPCODE_NUM][MAXLINELENGTH] = {
    "and",
    "nand",
    "lw",
    "sw",
    "beq",
    "jalr",
    "halt",
    "noop"
};

typedef enum { false, true } bool;

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
    size_t size;
    while ( readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {

        if (strlen(label) < 0 || strlen(label) > 6) exit(1); // label field length exceeded
        if (strlen(label) != 0 && isdigit(label[0])) exit(1); // label is started by number
        // bool validOpcode = false;
        // for (int k = 0; k != OPCODE_NUM; ++k)
        // {
        //     printf ("in opcode = %s\npossibleOpcode[%d] = %s\n", opcode, k, possibleOpcode[k]);
        //     if (strcmp(opcode, possibleOpcode[k]) == 0) validOpcode = true;
        // }
        // if (!validOpcode) exit(1);

        size = sizeof (instMem[lines].label);
        strncpy(instMem[lines].label, label, size);
        instMem[lines].label[size - 1] = '\0';

        size = sizeof (instMem[lines].opcode);
        strncpy(instMem[lines].opcode, opcode, size);
        instMem[lines].opcode[size - 1] = '\0';

        size = sizeof (instMem[lines].arg0);
        strncpy(instMem[lines].arg0, arg0, size);
        instMem[lines].arg0[size - 1] = '\0';

        size = sizeof (instMem[lines].arg1);
        strncpy(instMem[lines].arg1, arg1, size);
        instMem[lines].arg1[size - 1] = '\0';

        size = sizeof (instMem[lines].arg2);
        strncpy(instMem[lines].arg2, arg2, size);
        instMem[lines].arg2[size - 1] = '\0';

        // printf("%s\t%s\t%s\t%s\t%s\n", instMem[lines].label, instMem[lines].opcode, instMem[lines].arg0, instMem[lines].arg1, instMem[lines].arg2);

        // printf("---> instMem[%d].label = %s ------\n", lines, instMem[lines].label);
        // printf("[LINE] = %d --------------\n", lines);
        ++lines;
    }
    /* reached end of file */

    for (int i = 0;i != lines; ++i)
    {
        if (strcmp(instMem[i].opcode, ".fill") == 0)
        {
            char *endptr;
            char *number = instMem[i].arg0;
            strtol(number, &endptr, 10);
            if (*endptr != '\0') // arg0 is not a number
            {
                // printf("%s is not a number\n", instMem[i].arg0);
                for (int jAddr = 0; jAddr != lines; ++jAddr)
                {
                    if (strcmp(instMem[jAddr].label, instMem[i].arg0) == 0)
                    {
                        size = sizeof (instMem[i].arg1);
                        strncpy(instMem[i].arg1, instMem[jAddr].label, size);
                        instMem[i].arg1[size - 1] = '\0';
                        // printf("-------> instMem[%d].arg1 = %s\n", i, instMem[i].arg1);
                        snprintf(instMem[i].arg1, sizeof instMem[i].arg1, "%d", jAddr);
                        snprintf(instMem[i].arg0, sizeof instMem[i].arg0, "%d", jAddr);
                        // printf(" ----------- jAddr of %d = %d\n", i, jAddr);
                        // printf(" ----------- instMem[%d].arg0 = %s\n", i, instMem[i].arg0);
                        break;
                    }
                }
            }
        }
        else if (strcmp(instMem[i].opcode, "lw") == 0 || 
                strcmp(instMem[i].opcode, "sw") == 0)
        {
            for (int branchAddr = 0; branchAddr != lines; ++branchAddr)
            {
                if (strcmp(instMem[branchAddr].label, instMem[i].arg2) == 0)
                {
                    // printf (" --> @ instMem[%d].label = %d\n", branchAddr, branchAddr);
                    snprintf(instMem[i].arg2, sizeof instMem[i].arg2, "%d", branchAddr);
                    // printf(" ----------- branchAddr of %d = %d\n", i, branchAddr);
                    // printf(" ----------- instMem[%d].arg2 = %s\n", i, instMem[i].arg2);
                    break;
                }
            }
        }
        else if (strcmp(instMem[i].opcode, "beq") == 0)
        {
            char *endptr;
            char *number = instMem[i].arg2;
            strtol(number, &endptr, 10);
            if (*endptr != '\0') // arg2 is not a number
            {
                for (int jAddr = 0; jAddr != lines; ++jAddr)
                {
                    if (strcmp(instMem[jAddr].label, instMem[i].arg2) == 0)
                    {
                        int branchOffset = 0 - jAddr - 1;
                        snprintf(instMem[i].arg2, sizeof instMem[i].arg2, "%d", branchOffset);
                    }
                }
            }
        }

        if (strlen(instMem[i].label) != 0)
        {
            for (int j = i + 1; j < lines; ++j)
            {
                // printf ("instMem[%d]].label = %s\ninstMem[%d].label = %s\n", i, instMem[i].label, j, instMem[j].label);
                if (strcmp(instMem[i].label, instMem[j].label) == 0 &&
                    strlen(instMem[j].label) != 0)
                    exit(1);
            }
        }
    }


    // printf("%s\t%s\t%s\t%s\t%s\n", label, opcode, arg0, arg1, arg2);

    for (int i = 0; i != lines; ++i)
    {
        /* after doing a readAndParse, you may want to do the following to test the
            opcode */
        // printf("[%d]@ opcode = %s\n", i, instMem[i].opcode);
        instMem[i].inst.field.empty = 0; // bit 31-25 of all instructions are
        // add (R-type format) 000
        if (strcmp(instMem[i].opcode, "add") == 0) {
            /* do whatever you need to do for opcode "add" */
            instMem[i].inst.field.offset = atoi(instMem[i].arg2);
            instMem[i].inst.field.regB = atoi(instMem[i].arg1);
            instMem[i].inst.field.regA = atoi(instMem[i].arg0);
            instMem[i].inst.field.opcode = 0;
        }
        // nand (R-type format) 001
        else if (strcmp(instMem[i].opcode, "nand") == 0) {
            /* do whatever you need to do for opcode "nand" */
            instMem[i].inst.field.offset = atoi(instMem[i].arg2);
            instMem[i].inst.field.regB = atoi(instMem[i].arg1);
            instMem[i].inst.field.regA = atoi(instMem[i].arg0);
            instMem[i].inst.field.opcode = 1;
        }
        // lw (I-type format) 010
        else if (strcmp(instMem[i].opcode, "lw") == 0) {
            /* do whatever you need to do for opcode "lw" */
            instMem[i].inst.field.offset = atoi(instMem[i].arg2);
            instMem[i].inst.field.regB = atoi(instMem[i].arg1);
            instMem[i].inst.field.regA = atoi(instMem[i].arg0);
            instMem[i].inst.field.opcode = 2;
        }
        // sw (I-type format) 011
        else if (strcmp(instMem[i].opcode, "sw") == 0) {
            /* do whatever you need to do for opcode "sw" */
            instMem[i].inst.field.offset = atoi(instMem[i].arg2);
            instMem[i].inst.field.regB = atoi(instMem[i].arg1);
            instMem[i].inst.field.regA = atoi(instMem[i].arg0);
            instMem[i].inst.field.opcode = 3;
        }
        // beq (I-type format) 100
        else if (strcmp(instMem[i].opcode, "beq") == 0) {
            /* do whatever you need to do for opcode "beq" */
            instMem[i].inst.field.offset = atoi(instMem[i].arg2);
            instMem[i].inst.field.regB = atoi(instMem[i].arg1);
            instMem[i].inst.field.regA = atoi(instMem[i].arg0);
            instMem[i].inst.field.opcode = 4;
        }
        // jalr (J-type format) 101
        else if (strcmp(instMem[i].opcode, "jalr") == 0) {
            /* do whatever you need to do for opcode "jalr" */
            instMem[i].inst.field.offset = 0;
            instMem[i].inst.field.regB = atoi(instMem[i].arg1);
            instMem[i].inst.field.regA = atoi(instMem[i].arg0);
            instMem[i].inst.field.opcode = 5;
        }
        // halt (O-type format) 110
        else if (strcmp(instMem[i].opcode, "halt") == 0) {
            /* do whatever you need to do for opcode "halt" */
            instMem[i].inst.field.offset = 0;
            instMem[i].inst.field.regB = 0;
            instMem[i].inst.field.regA = 0;
            instMem[i].inst.field.opcode = 6;
        }
        // noop (O-type format) 111
        else if (strcmp(instMem[i].opcode, "noop") == 0) {
            /* do whatever you need to do for opcode "noop" */
            instMem[i].inst.field.offset = 0;
            instMem[i].inst.field.regB = 0;
            instMem[i].inst.field.regA = 0;
            instMem[i].inst.field.opcode = 7;
        }
        // .fill (special format) for symbolic address and immediate
        else if (strcmp(instMem[i].opcode, ".fill") == 0) {
            // printf("instMem[i].arg0 = %s\n", instMem[i].arg0);
            instMem[i].inst.intRepresentation = (int) atoi(instMem[i].arg0);
        }
        else exit(1);
        fprintf(outFilePtr, "%d\n", instMem[i].inst.intRepresentation);
        printf("(address %d): %d (hex 0x%x)\n", i, instMem[i].inst.intRepresentation, instMem[i].inst.intRepresentation);
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