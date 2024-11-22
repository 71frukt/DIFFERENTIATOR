#include <stdio.h>
#include "diff_tree.h"
#include "diff_debug.h"

FILE *OutputFile = NULL;  // TODO: change

int main(const int argc, const char *argv[])
{
    OutputFile = GetOutputFile(argc, argv);

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



    GetTreeFromFile(&tree, "file.txt");
    DIFF_DUMP(&tree);

    DIFF_DUMP(&tree);

    char str[100] = {};
    GetStrTreeData(tree.root_ptr, str);
    fprintf(stderr, "str = %s\n\n", str);


    char tex[100] = {};
    GetTexTreeData(tree.root_ptr, tex, false);
    fprintf(stderr, "tex = %s\n\n", tex);


    TreeDtor(&tree);
    fprintf(stderr, "END!\n");
    return 0;
}