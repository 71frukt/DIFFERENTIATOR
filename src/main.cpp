#include <stdio.h>
#include "diff_tree.h"

int main()
{
    fprintf(stderr, "START!\n");

    Tree tree = {};

    TreeCtor(&tree, START_DATATREE_SIZE);

    fprintf(stderr, "END!\n");
    return 0;
}