#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdint.h>

#define MAXLINELENGTH 1000

typedef enum { false, true } bool;

int32_t readAndParse(FILE *, char *, char *, char *, char *, char *);
bool isNumber(char *);

#define COPY_INSTRUCTION(src, dest)\
    size = sizeof (dest);\
    strncpy(dest, src, size);\
    dest[size - 1] = '\0';

#define FIELD(inst_field) instMem[i].inst.field.inst_field

/* Reads a file and returns the number of lines in this file. */
int32_t countLines(FILE *file) {
    int32_t lines = 0;
    int32_t c;
    int32_t last = '\n';
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
        int32_t offset: 16;
        uint32_t regB: 3;
        uint32_t regA: 3;
        uint32_t opcode: 3;
        uint32_t empty: 7;
    } field;
    int32_t intRepresentation; 
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

#define OPCODE_NUM 9
char possibleOpcode[OPCODE_NUM][MAXLINELENGTH] = {
    "add",
    "nand",
    "lw",
    "sw",
    "beq",
    "jalr",
    "halt",
    "noop",
    ".fill"
};

int32_t main(int32_t argc, char *argv[])
{
    // printf("short is %d bits\n",     CHAR_BIT * sizeof( short )   );
    // printf("int32_t is %d bits\n",       CHAR_BIT * sizeof( int32_t  )    );
    // printf("int32_t is %d bits\n",       CHAR_BIT * sizeof( int32_t  )    );
    // printf("long is %d bits\n",      CHAR_BIT * sizeof( long )    );
    // printf("long long is %d bits\n", CHAR_BIT * sizeof(long long) );

    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
         arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
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

    int32_t reg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    bitset testInst;
    int32_t lines = 0;

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);

    /* here is an example for how to use readAndParse to  from
    inFilePtr */
    size_t size;
    while ( readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) // while loop for reading every single line
    {
        if (strlen(label) < 0 || strlen(label) > 6) exit(1); // label field length exceeded
        if (strlen(label) != 0 && isdigit(label[0])) exit(1); // label is started by number
        bool validOpcode = false;
        for (int32_t k = 0; k != OPCODE_NUM; ++k)
        {
            if (strcmp(opcode, possibleOpcode[k]) == 0)
            {
                validOpcode = true;
                break;
            }
        }
        if (!validOpcode) exit(1); // Invalid opcode checking

        COPY_INSTRUCTION(label, instMem[lines].label);
        COPY_INSTRUCTION(opcode, instMem[lines].opcode);
        COPY_INSTRUCTION(arg0, instMem[lines].arg0);
        COPY_INSTRUCTION(arg1, instMem[lines].arg1);
        COPY_INSTRUCTION(arg2, instMem[lines].arg2);
        ++lines;
    }
    /* reached end of file by reading every single line */

    for (int32_t i = 0;i != lines; ++i) // Checking opcode of each line
    {
        if (strcmp(instMem[i].opcode, ".fill") == 0) // opcode is .fill
        {
            if (isNumber(instMem[i].arg0)) // arg0 is not a number
            {
                for (int32_t jAddr = 0; jAddr != lines; ++jAddr)
                {
                    if (strcmp(instMem[jAddr].label, instMem[i].arg0) == 0)
                    {
                        size = sizeof (instMem[i].arg1);
                        strncpy(instMem[i].arg1, instMem[jAddr].label, size);
                        instMem[i].arg1[size - 1] = '\0';
                        snprintf(instMem[i].arg1, sizeof instMem[i].arg1, "%d", jAddr);
                        snprintf(instMem[i].arg0, sizeof instMem[i].arg0, "%d", jAddr);
                        break;
                    }
                }
            }
        }
        else if (strcmp(instMem[i].opcode, "lw") == 0 || // opcode is lw
                strcmp(instMem[i].opcode, "sw") == 0) // opcode is sw
        {
            int32_t offsetTest = atoi(instMem[i].arg2);
            if (offsetTest < -32768 || offsetTest > 32767) exit(1);
            for (int32_t branchAddr = 0; branchAddr != lines; ++branchAddr)
            {
                if (strcmp(instMem[branchAddr].label, instMem[i].arg2) == 0)
                {
                    snprintf(instMem[i].arg2, sizeof instMem[i].arg2, "%d", branchAddr);
                    break;
                }
            }
        }
        else if (strcmp(instMem[i].opcode, "beq") == 0) // opcode is beq
        {
            int32_t offsetTest = atoi(instMem[i].arg2);
            if (offsetTest < -32768 || offsetTest > 32767) exit(1);
            if (isNumber(instMem[i].arg2)) // arg2 is not a number
            {
                for (int32_t jAddr = 0; jAddr != lines; ++jAddr)
                {
                    if (strcmp(instMem[jAddr].label, instMem[i].arg2) == 0)
                    {
                        int32_t branchOffset;
                        if (jAddr < 0) branchOffset = (-1) * (jAddr - i + 1);
                        else branchOffset = jAddr - i - 1;
                        snprintf(instMem[i].arg2, sizeof instMem[i].arg2, "%d", branchOffset);
                    }
                }
            }
        }

        if (strlen(instMem[i].label) != 0) // duplicated label checking
        {
            for (int32_t j = i + 1; j < lines; ++j)
            {
                if (strcmp(instMem[i].label, instMem[j].label) == 0 &&
                    strlen(instMem[j].label) != 0)
                    exit(1);
            }
        }
    }

    for (int32_t i = 0; i != lines; ++i)
    {
        /* after doing a readAndParse, you may want to do the following to test the
            opcode */
        instMem[i].inst.field.empty = 0; // bit 31-25 of all instructions are
        // add (R-type format) 000
        if (strcmp(instMem[i].opcode, "add") == 0) {
            FIELD(offset) = atoi(instMem[i].arg2);
            FIELD(regB) = atoi(instMem[i].arg1);
            FIELD(regA) = atoi(instMem[i].arg0);
            FIELD(opcode) = 0;
        }
        // nand (R-type format) 001
        else if (strcmp(instMem[i].opcode, "nand") == 0) {
            FIELD(offset) = atoi(instMem[i].arg2);
            FIELD(regB) = atoi(instMem[i].arg1);
            FIELD(regA) = atoi(instMem[i].arg0);
            FIELD(opcode) = 1;
        }
        // lw (I-type format) 010
        else if (strcmp(instMem[i].opcode, "lw") == 0) {
            FIELD(offset) = atoi(instMem[i].arg2);
            FIELD(regB) = atoi(instMem[i].arg1);
            FIELD(regA) = atoi(instMem[i].arg0);
            FIELD(opcode) = 2;
        }
        // sw (I-type format) 011
        else if (strcmp(instMem[i].opcode, "sw") == 0) {
            FIELD(offset) = atoi(instMem[i].arg2);
            FIELD(regB) = atoi(instMem[i].arg1);
            FIELD(regA) = atoi(instMem[i].arg0);
            FIELD(opcode) = 3;
        }
        // beq (I-type format) 100
        else if (strcmp(instMem[i].opcode, "beq") == 0) {
            FIELD(offset) = atoi(instMem[i].arg2);
            FIELD(regB) = atoi(instMem[i].arg1);
            FIELD(regA) = atoi(instMem[i].arg0);
            FIELD(opcode) = 4;
        }
        // jalr (J-type format) 101
        else if (strcmp(instMem[i].opcode, "jalr") == 0) {
            FIELD(offset) = 0;
            FIELD(regB) = atoi(instMem[i].arg1);
            FIELD(regA) = atoi(instMem[i].arg0);
            FIELD(opcode) = 5;
        }
        // halt (O-type format) 110
        else if (strcmp(instMem[i].opcode, "halt") == 0) {
            FIELD(offset) = 0;
            FIELD(regB) = 0;
            FIELD(regA) = 0;
            FIELD(opcode) = 6;
        }
        // noop (O-type format) 111
        else if (strcmp(instMem[i].opcode, "noop") == 0) {
            FIELD(offset) = 0;
            FIELD(regB) = 0;
            FIELD(regA) = 0;
            FIELD(opcode) = 7;
        }
        // .fill (special format) for symbolic address and immediate
        else if (strcmp(instMem[i].opcode, ".fill") == 0) {
            instMem[i].inst.intRepresentation = (int32_t) atoi(instMem[i].arg0);
        }
        else exit(1); // undefined opcode exit

        // write the result machine codes into output file
        fprintf(outFilePtr, "%d\n", instMem[i].inst.intRepresentation);

        // printf result
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
int32_t readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
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

bool isNumber(char *string)
{
    /* return 1 if string is a number */
    char *endptr;
    char *number = string;
    strtol(number, &endptr, 10);
    return (*endptr != '\0'); // string is not a number
}