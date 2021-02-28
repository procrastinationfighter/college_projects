#include "BeingTree.h"

/* Creates and returns new BeingTree node. */
BeingTree createTreeNode(String *name) {
    BeingTree newTreeNode = malloc(sizeof(struct being));
    if (newTreeNode == NULL) {
        exit(1);
    }

    newTreeNode->right = NULL;
    newTreeNode->left = NULL;
    newTreeNode->name = name->string;
    newTreeNode->value = NULL;

    return newTreeNode;
}

/* Removes node with minimal name in lexicographical order from given tree and returns it. */
BeingTree cutMinOutOfTree(BeingTree *tree) {
    if (*tree == NULL) {
        return *tree;
    }
    else if ((*tree)->left == NULL) {
        BeingTree temp = *tree;
        (*tree) = temp->right;
        return temp;
    }
    else {
        return cutMinOutOfTree(&(*tree)->left);
    }
}

/* Removes given node and frees all memory related to it. */
void removeTreeNode(BeingTree *tree) {
    if (*tree != NULL) {
        free((*tree)->name);
        destroyTree(&(*tree)->value);

        BeingTree temp = *tree;

        if ((*tree)->left == NULL) {
            *tree = (*tree)->right;
        }
        else if ((*tree)->right == NULL) {
            *tree = (*tree)->left;
        }
        else {
            *tree = cutMinOutOfTree(&(*tree)->right);
            (*tree)->left = temp->left;
            (*tree)->right = temp->right;
        }
        free(temp);
    }
}

/* Removes all nodes in given tree. */
void destroyTree(BeingTree *tree) {
    if (*tree != NULL) {
        destroyTree(&(*tree)->left);
        destroyTree(&(*tree)->right);
        removeTreeNode(tree);
    }
}

/* Prints name of every node in a tree in lexicographical order. */
void printTreeInfixLR(BeingTree tree) {
    if (tree != NULL) {
        printTreeInfixLR(tree->left);
        printf("%s\n", tree->name);
        printTreeInfixLR(tree->right);
    }
}

/* Returns pointer to tree node with given name. */
BeingTree *findTreeNodePointer(BeingTree *tree, String *name) {
    if (*tree == NULL) {
        return tree;
    }

    int difference = strcmp(name->string, (*tree)->name);

    if (difference > 0) {
        return findTreeNodePointer(&(*tree)->right, name);
    }
    else if (difference < 0) {
        return findTreeNodePointer(&(*tree)->left, name);
    }
    else {
        return tree;
    }
}