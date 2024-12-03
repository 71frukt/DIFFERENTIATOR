#include <stdio.h>

#include <math.h>

#include "diff_tree.h"
#include "reader.h"
#include "tex_work.h"
#include "diff_debug.h"
#include "derivative.h"
#include "simplifier.h"

FILE *OutputFile = NULL;  // TODO: change

int main(const int argc, const char *argv[])
{
    fprintf(stderr, "START!\n");

    // Expression expr = {.data = "1 / x$"};
    Expression expr = {.data = "x ^ (2 * x ^ ln(x / 2 ^ 3 ^ 4 ^ 5 ^ 6 ^ 7 ^ 8 ^ 9 ^ 10 ^ 11 ^ 12))$"};
    
    OutputFile = GetOutputFile(argc, argv);

    Tree orig = {};
    
    TreeCtor(&orig, START_TREE_SIZE, "orig_expression");
    GetExpr(&expr, &orig);

    DIFF_DUMP(&orig);

    char tex_orig[TEX_EXPRESSION_LEN] = {};
    GetTexTreeData(orig.root_ptr, tex_orig, false);
    fprintf(OutputFile, "The original expression has the form \\[ %s \\]\n\n", tex_orig);

    SplitTree(&orig, orig.root_ptr);

    tex_orig[0] = '\0';
    GetTexTreeData(orig.root_ptr, tex_orig, false);
    fprintf(OutputFile, "The original expression has new form \\[ %s \\]\n\n", tex_orig);

    DIFF_DUMP(&orig);

    PrintChangedVarsTex(&orig, OutputFile);
/*
    // SplitTree(&orig, orig.root_ptr);
    DIFF_DUMP(&orig);
    Tree orig_simpl = {};
    TreeCtor(&orig_simpl, START_TREE_SIZE, "orig_simpl");
    orig_simpl.root_ptr = TreeCopyPaste(&orig, &orig_simpl, orig.root_ptr);

    DIFF_DUMP(&orig_simpl);

    SimplifyExpr(&orig_simpl);

    char tex_orig_simpl[TEX_EXPRESSION_LEN] = {};
    GetTexTreeData(orig_simpl.root_ptr, tex_orig_simpl, false);

    fprintf(OutputFile, "By simple mathematical transformations: \\[ %s \\]\n\n \\newline ", tex_orig_simpl);


    fprintf(stderr, "before diff\n");

    Tree derivative = {};
    TreeCtor(&derivative, START_TREE_SIZE, "derivative");
    derivative.root_ptr = TakeDifferential(&orig_simpl, orig_simpl.root_ptr, &derivative);

    char tex_derivative[TEX_EXPRESSION_LEN] = {};
    GetTexTreeData(derivative.root_ptr, tex_derivative, false);

    fprintf(OutputFile, "Having counted the most obvious derivative, which the Soviet spermatozoa were actually able to calculate in their minds, we get: \\[ %s \\]\n \\newline ", tex_derivative);
    DIFF_DUMP(&derivative);
    SimplifyExpr(&derivative);

    char tex_derivative_simpl[TEX_EXPRESSION_LEN] = {};
    GetTexTreeData(derivative.root_ptr, tex_derivative_simpl, false);

    fprintf(OutputFile, "By simple mathematical transformations: \\[ %s \\]\n", tex_derivative_simpl);

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
    TreeDtor(&orig_simpl);
    // TreeDtor(&derivative);
*/

    fprintf(stderr, "END!\n");
    return 0;
}