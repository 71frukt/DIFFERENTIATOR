#include <stdio.h>
#include "diff_tree.h"
#include "diff_debug.h"

int main()
{
    fprintf(stderr, "START!\n");

    Tree tree = {};
    
    TreeCtor(&tree, START_TREE_SIZE);

    // Node *x   = NewNode(&tree, VAR, VAR_X, NULL, NULL);

    // DIFF_DUMP(&tree);

    // Node *n1  = NewNode(&tree, NUM,    3,   NULL, NULL);
    // DIFF_DUMP(&tree);

    // Node *add = NewNode(&tree, OP,    ADD,     x,   n1);
    // DIFF_DUMP(&tree);

    // Node *n2  = NewNode(&tree, NUM,  1000,  NULL, NULL);

    // Node *n3  = NewNode(&tree, NUM,     7,  NULL, NULL);

    // Node *sub = NewNode(&tree, OP,    SUB,   n2,    n3);

    // Node *div = NewNode(&tree, OP,    DIV,   add,  sub);

    // char data[100] = {};
    // GetStrTreeData(tree.root_ptr, data);

    // fprintf(stderr, "%s\n", data);


    char *source = "(/(+(x)(3))(-(1000)(7)))";

    GetTreeFromFile(&tree, "file.txt");

    DIFF_DUMP(&tree);

    TreeDtor(&tree);
    fprintf(stderr, "END!\n");
    return 0;
}