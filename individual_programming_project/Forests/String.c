#include "String.h"

/* Initializes all variables in given String to default values. */
void initializeString(String *str) {
    str->string = malloc(MULTIPLIER * sizeof(char));
    if (str->string == NULL)
        exit(1);

    str->length = 0;
    str->size = MULTIPLIER;
}

/* Frees all memory related to given String. */
void deinitializeString(String *str) {
    free(str->string);
}

/* Allocates MULTIPLIER times more memory than is currently allocated for string. */
void increaseStringSize(String *str) {
    str->size = MULTIPLIER * str->size;
    str->string = realloc(str->string, (str->size * sizeof(char)));
    if (str->string == NULL) {
        exit(1);
    }
}

/* Adds new character at the end of string. */
void pushBackInString(String *str, char ch) {
    if (str->size == str->length) {
        increaseStringSize(str);
    }
    str->string[str->length] = ch;
    (str->length)++;
}

/* Initializes all Strings in given Instruction to default value. */
void initializeInstruction(Instruction *instruction) {
    initializeString(&(instruction->command));
    initializeString(&(instruction->forest));
    initializeString(&(instruction->tree));
    initializeString(&(instruction->animal));
}

/* Frees Strings located in given Instruction. Does not free memory related to parameters used elsewhere. */
void deinitializeInstruction(Instruction *instruction, const bool areParametersUsed[]) {
    deinitializeString(&(instruction->command));
    if (!areParametersUsed[0]) {
        deinitializeString(&(instruction->forest));
    }
    if (!areParametersUsed[1]) {
        deinitializeString(&(instruction->tree));
    }
    if (!areParametersUsed[2]) {
        deinitializeString(&(instruction->animal));
    }
}

/* Returns true if string contains only '\0' character. */
bool isStringEmpty(String *str) {
    return (strcmp(str->string, "\0") == 0);
}

/* Returns true if string contains only a single '*' character. */
bool isAnAsterisk(String *str) {
    return (strcmp(str->string, "*") == 0);
}