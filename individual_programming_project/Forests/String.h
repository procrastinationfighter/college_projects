#ifndef FORESTS_STRING_H
#define FORESTS_STRING_H
#define MULTIPLIER 2

#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

struct string {
    char *string;
    size_t size;
    size_t length;
};
typedef struct string String;

struct instruction {
    String command;
    String forest;
    String tree;
    String animal;
};
typedef struct instruction Instruction;

/* Initializes all variables in given String to default values. */
void initializeString(String *str);

/* Frees all memory related to given String. */
void deinitializeString(String *str);

/* Allocates MULTIPLIER times more memory than is currently allocated for string. */
void increaseStringSize(String *str);

/* Adds new character at the end of string. */
void pushBackInString(String *str, char ch);

/* Initializes all Strings in given Instruction to default value. */
void initializeInstruction(Instruction *instruction);

/* Frees Strings located in given Instruction. Does not free memory related to parameters used elsewhere. */
void deinitializeInstruction(Instruction *instruction, const bool shouldAllBeDeinitialized[]);

/* Returns true if string contains only '\0' character. */
bool isStringEmpty(String *str);

/* Returns true if string contains only a single '*' character. */
bool isAnAsterisk(String *str);

#endif //FORESTS_STRING_H
