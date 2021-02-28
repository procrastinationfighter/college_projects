#include "World.h"

enum BeingsType {
    FORESTS, TREES, ANIMALS, INVALID_TYPE
};
typedef enum BeingsType BeingsType;

/* Initializes all variables in world to default values. */
void initializeWorld(World *world) {
    world->forests = NULL;
}

/* Frees all memory related to world. */
void deinitializeWorld(World *world) {
    destroyTree(&world->forests);
}

/* Returns parameter in instruction related to given type of beings. */
String *convertBeingsTypeToBeingName(Instruction *instruction, BeingsType typeOfBeings) {
    switch (typeOfBeings) {
        case FORESTS:
            return &instruction->forest;
        case TREES:
            return &instruction->tree;
        case ANIMALS:
            return &instruction->animal;
        default:
            return NULL;
    }
}

/* Returns type of beings that are located inside of given type of beings. */
BeingsType nextBeing(BeingsType typeOfBeings) {
    switch (typeOfBeings) {
        case FORESTS:
            return TREES;
        case TREES:
            return ANIMALS;
        case ANIMALS:
        case INVALID_TYPE:
        default:
            return INVALID_TYPE;
    }
}

/* Checks whether being specified in instruction exists inside any node of given tree.*/
bool checkAll(BeingTree tree, Instruction *instruction, BeingsType typeOfBeings) {
    if (tree != NULL && typeOfBeings != ANIMALS) {
        bool found;
        String *wantedBeing = convertBeingsTypeToBeingName(instruction, nextBeing(typeOfBeings));

        // If wanted being name is an asterisk, current case is CHECK * * <animalName>,
        // search for <animalName> in tree of trees in current forest.
        if (isAnAsterisk(wantedBeing)) {
            found = checkAll(tree->value, instruction, nextBeing(typeOfBeings));
        }
        else {
            BeingTree *foundNode = findTreeNodePointer(&tree->value, wantedBeing);

            // Search deeper only if foundNode points to a tree (a being) and animal is still to be found.
            if (typeOfBeings == FORESTS && !isStringEmpty(&instruction->animal) && *foundNode != NULL) {
                foundNode = findTreeNodePointer(&(*foundNode)->value, &instruction->animal);
            }
            found = (*foundNode != NULL);
        }

        //If being wasn't found in current node, search in its children.
        if (!found) {
            found = checkAll(tree->left, instruction, typeOfBeings);
        }
        if (!found) {
            found = checkAll(tree->right, instruction, typeOfBeings);
        }

        return found;
    }
    else {
        return false;
    }
}

/* Prints results of check command. */
void printCheckResult(bool found) {
    if (found) {
        printf("YES\n");
    }
    else {
        printf("NO\n");
    }
}

/* Check whether being specified in instruction exists.
 * If a parameter is an asterisk, all beings of its type should be searched.
 * It is assumed that instruction is valid. */
void checkExistanceOfBeings(World *world, Instruction *instruction) {
    bool found = false;

    if (isAnAsterisk(&instruction->forest)) {
        found = checkAll(world->forests, instruction, FORESTS);
    }
    else {
        // Looking for a forest.
        BeingTree *curr = findTreeNodePointer(&world->forests, &instruction->forest);

        if (*curr == NULL) {
            found = false;
        }
        else if (isStringEmpty(&instruction->tree)) {
            found = true;
        }
        else if (isAnAsterisk(&instruction->tree)) {
            found = checkAll((*curr)->value, instruction, TREES);
        }
        else {
            // Looking for a tree.
            curr = findTreeNodePointer(&(*curr)->value, &instruction->tree);

            if (isStringEmpty(&instruction->animal)) {
                found = *curr != NULL;
            }
            else if (*curr != NULL) {
                // Looking for an animal.
                curr = findTreeNodePointer(&(*curr)->value, &instruction->animal);
                found = *curr != NULL;
            }
        }
    }
    printCheckResult(found);
}

/* Checks whether being with given name exists in given tree and returns pointer to it.
 * If it does not exist, creates a node with this name. */
BeingTree *findAndAddElement(BeingTree *tree, String *name, bool *isParameterUsed) {
    BeingTree *curr = findTreeNodePointer(&(*tree)->value, name);
    if (*curr == NULL) {
        *curr = createTreeNode(name);
        *isParameterUsed = true;
    }
    return curr;
}

/* Adds beings specified in instruction to world.
 * Array areParametersUsed[] has information whether parameters in instruction were used to add new beings. */
void addBeings(World *world, Instruction *instruction, bool areParametersUsed[]) {
    BeingTree *curr = findTreeNodePointer(&world->forests, &instruction->forest);
    if (*curr == NULL) {
        *curr = createTreeNode(&instruction->forest);
        areParametersUsed[0] = true;
    }
    if (!isStringEmpty(&instruction->tree)) {
        curr = findAndAddElement(curr, &instruction->tree, &areParametersUsed[1]);
        if (!isStringEmpty(&instruction->animal)) {
            findAndAddElement(curr, &instruction->animal, &areParametersUsed[2]);
        }
    }

    printf("OK\n");
}

/* Removes being specified in instruction. */
void deleteBeings(World *world, Instruction *instruction) {
    if (isStringEmpty(&instruction->forest)) {
        destroyTree(&world->forests);
    }
    else {
        // Looking for a forest.
        BeingTree *curr = findTreeNodePointer(&world->forests, &instruction->forest);

        // If such forest does not exist, it is the end of deleting.
        if (*curr == NULL) {
            printf("OK\n");
            return;
        }
        else if (isStringEmpty(&instruction->tree)) { // Target found.
            removeTreeNode(curr);
        }
        else {
            // Looking for a tree.
            curr = findTreeNodePointer(&(*curr)->value, &instruction->tree);

            // If such tree does not exist, it is the end of deleting.
            if (*curr == NULL) {
                printf("OK\n");
                return;
            }
            else if (isStringEmpty(&instruction->animal)) { // Target found.
                removeTreeNode(curr);
            }
            else {
                // Looking for an animal.
                curr = findTreeNodePointer(&(*curr)->value, &instruction->animal);
                removeTreeNode(curr);
            }
        }
    }
    printf("OK\n");
}

/* Prints all beings located in place specified by instruction in lexicographical order. */
void printExistingBeings(World *world, Instruction *instruction) {
    if (isStringEmpty(&instruction->forest)) {
        printTreeInfixLR(world->forests);
    }
    else {
        BeingTree *curr = findTreeNodePointer(&world->forests, &instruction->forest);
        if (*curr == NULL) {
            return;
        }
        else if (isStringEmpty(&instruction->tree)) {
            printTreeInfixLR((*curr)->value);
        }
        else {
            curr = findTreeNodePointer(&(*curr)->value, &instruction->tree);
            if (*curr != NULL) {
                printTreeInfixLR((*curr)->value);
            }
        }
    }
}
