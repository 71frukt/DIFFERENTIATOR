#include <stdio.h>

#include "diff_tree.h"
#include "diff_debug.h"
#include "derivative.h"

FILE *OutputFile = NULL;  // TODO: change

int main(const int argc, const char *argv[])
{
    OutputFile = GetOutputFile(argc, argv);

    fprintf(stderr, "START!\n");

    Tree orig = {};
    
    TreeCtor(&orig, START_TREE_SIZE, "orig_expression");

    GetTreeFromFile(&orig, "file.txt");
    DIFF_DUMP(&orig);

    char str[100] = {};
    GetStrTreeData(orig.root_ptr, str);
    fprintf(stderr, "str = %s\n\n", str);

    char tex[100] = {};
    GetTexTreeData(orig.root_ptr, tex, false);
    fprintf(stderr, "tex = %s\n\n", tex);

    Tree solving = {};
    TreeCtor(&solving, START_TREE_SIZE, "solving");

    TakeDifferential(&orig, orig.root_ptr, &solving);

    TreeDtor(&solving);
    TreeDtor(&orig);

    fprintf(stderr, "END!\n");
    return 0;
}