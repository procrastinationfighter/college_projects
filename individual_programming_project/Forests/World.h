#ifndef FORESTS_WORLD_H
#define FORESTS_WORLD_H

#include "String.h"
#include "BeingTree.h"
#include <stdbool.h>

struct world {
    BeingTree forests;
};
typedef struct world World;

/* Initializes all variables in world to default values. */
void initializeWorld(World *world);

/* Frees all memory related to world. */
void deinitializeWorld(World *world);

/* Check whether being specified in instruction exists.
 * If a parameter is an asterisk, all beings of its type should be searched.
 * It is assumed that instruction is valid. */
void checkExistanceOfBeings(World *world, Instruction *instruction);

/* Adds beings specified in instruction to world.
 * Array areParametersUsed[] has information whether parameters in instruction were used to add new beings. */
void addBeings(World *world, Instruction *instruction, bool areParametersUsed[]);

/* Removes being specified in instruction. */
void deleteBeings(World *world, Instruction *instruction);

/* Prints all beings located in place specified by instruction in lexicographical order. */
void printExistingBeings(World *world, Instruction *instruction);

#endif //FORESTS_WORLD_H
