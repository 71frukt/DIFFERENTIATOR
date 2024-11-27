#include <stdio.h>

#include <math.h>

#include "diff_tree.h"
#include "tex_work.h"
#include "diff_debug.h"
#include "derivative.h"
#include "simplifier.h"

FILE *OutputFile = NULL;  // TODO: change

int main(const int argc, const char *argv[])
{
    OutputFile = GetOutputFile(argc, argv);

    fprintf(stderr, "START!\n");

    Tree orig = {};
    
    TreeCtor(&orig, START_TREE_SIZE, "orig_expression");

    GetTreeFromFile(&orig, "file.txt");
    DIFF_DUMP(&orig);

    char tex[TEX_EXPRESSION_LEN] = {};
    GetTexTreeData(orig.root_ptr, tex, false);
    fprintf(OutputFile, "orig \\[ %s \\]\n\n", tex);
    fprintf(stderr, "orig \\[ %s \\]\n\n", tex);

    SimplifyExpr(&orig);

    char tex1[TEX_EXPRESSION_LEN] = {};
    GetTexTreeData(orig.root_ptr, tex1, false);
    fprintf(OutputFile, "simpl 1 \\[ %s \\]\n\n", tex1);
    fprintf(stderr, "simpl 1 \\[ %s \\]\n\n", tex1);

    // SimplifyExpr(&orig);
    // char tex2[TEX_EXPRESSION_LEN] = {};
    // GetTexTreeData(orig.root_ptr, tex2, false);
    // fprintf(OutputFile, "simp2 \\[ %s \\]\n\n", tex2);
    // fprintf(stderr, "simp2 \\[ %s \\]\n\n", tex2);

    fprintf(stderr, "before diff\n");

//     Tree solving = {};
//     TreeCtor(&solving, START_TREE_SIZE, "solving");

//     solving.root_ptr = TakeDifferential(&orig, orig.root_ptr, &solving);

//     char diff_str[TEX_EXPRESSION_LEN] = {};
//     GetTexTreeData(solving.root_ptr, diff_str, false);
//     fprintf(OutputFile, "diff: \\[ %s \\]\n\n", diff_str);
// fprintf(stderr, "diff: \\[ %s \\]\n\n", diff_str);
//     SimplifyExpr(&solving);

//     char tex3[TEX_EXPRESSION_LEN] = {};
//     GetTexTreeData(solving.root_ptr, tex3, false);
//     fprintf(OutputFile, "simp3 \\[ %s \\]\n\n", tex3);
// fprintf(stderr, "simp3 \\[ %s \\]\n\n", tex3);

//     SimplifyExpr(&solving);

//     char tex4[TEX_EXPRESSION_LEN] = {};
//     GetTexTreeData(solving.root_ptr, tex4, false);
//     fprintf(OutputFile, "simp4 \\[ %s \\]\n\n", tex4);
// DIFF_DUMP(&solving);
//     TreeDtor(&solving);
    TreeDtor(&orig);

    fprintf(stderr, "END!\n");
    return 0;
}