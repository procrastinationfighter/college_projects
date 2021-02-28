#ifndef FORESTS_BEINGTREE_H
#define FORESTS_BEINGTREE_H

#include "String.h"

struct being;
typedef struct being *BeingTree;
struct being {
    char *name;
    BeingTree left;
    BeingTree right;
    BeingTree value;
};

/* Creates and returns new BeingTree node. */
BeingTree createTreeNode(String *name);

/* Removes given node and frees all memory related to it. */
void removeTreeNode(BeingTree *tree);

/* Removes all nodes in given tree. */
void destroyTree(BeingTree *tree);

/* Prints name of every node in a tree in lexicographical order. */
void printTreeInfixLR(BeingTree tree);

/* Returns pointer to tree node with given name. */
BeingTree *findTreeNodePointer(BeingTree *tree, String *name);

#endif //FORESTS_BEINGTREE_H
