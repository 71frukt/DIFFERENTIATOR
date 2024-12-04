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

    // Expression expr = {.data = "5 * 6$"};
    // Expression expr = {.data = "2 ^ (2 * x) + 2 ^ (2) / 36 / (x - 8 * x ^ (3 ^ (1 / 2)))$"};
    Expression expr = {.data = "3 * x ^ 8 - 8 * x ^ 6 - 3 * x + 45$"};
    
    OutputFile = GetOutputFile(argc, argv);

    Tree orig = {};
    
    TreeCtor(&orig, START_TREE_SIZE, "orig_expression");
    GetExpr(&expr, &orig);

    DIFF_DUMP(&orig);

    char tex_orig[TEX_EXPRESSION_LEN] = {};
    GetTexTreeData(orig.root_ptr, tex_orig, false);
    fprintf(OutputFile, "The original expression has the form \\[f(x) = %s \\]\n\n", tex_orig);


    DIFF_DUMP(&orig);
    Tree orig_simpl = {};
    TreeCtor(&orig_simpl, START_TREE_SIZE, "orig_simpl");
    orig_simpl.root_ptr = TreeCopyPaste(&orig, &orig_simpl, orig.root_ptr);

    DIFF_DUMP(&orig_simpl);

    SimplifyExpr(&orig_simpl, orig_simpl.root_ptr);
    // SplitTree(&orig_simpl, orig_simpl.root_ptr);

    char tex_split[TEX_EXPRESSION_LEN] = {};
    GetTexTreeData(orig_simpl.root_ptr, tex_split, false);
    fprintf(OutputFile, "after making substitutions, we will get: \\newline\n \\[f(x) = %s\\]\\newline\n", tex_split);
    PrintChangedVarsTex(&orig_simpl, OutputFile);

    DIFF_DUMP(&orig_simpl);

    // tex_orig_simpl[0] = '\0';
    // GetTexTreeData(orig_simpl.root_ptr, tex_orig_simpl, false);
    // fprintf(OutputFile, "The simple original expression has new form \\[ %s \\]\n\n", tex_orig_simpl);
    // PrintChangedVarsTex(&orig_simpl, OutputFile);

    fprintf(stderr, "before diff\n");

    Tree derivative = {};
    TreeCtor(&derivative, START_TREE_SIZE, "derivative");
    derivative.root_ptr = derivative.node_ptrs[0];              // TODO: нахуй
    derivative.root_ptr = TakeHighDerivative(&orig_simpl, orig_simpl.root_ptr, &derivative, 5);

    char tex_derivative[TEX_EXPRESSION_LEN] = {};
    GetTexTreeData(derivative.root_ptr, tex_derivative, false);

    fprintf(OutputFile, "In total, we imeem(poimeem): \\newline\\[f'(x) = %s\\]\n\\newline\n", tex_derivative);
    PrintChangedVarsTex(&derivative, OutputFile);

    // Tree derivative_2 = {};
    // TreeCtor(&derivative_2, START_TREE_SIZE, "derivative_2");
    // derivative_2.root_ptr = derivative_2.node_ptrs[0];              // TODO: нахуй
    // derivative_2.root_ptr = TakeDerivative(&derivative, derivative.root_ptr, &derivative_2);

    // char tex_derivative_2[TEX_EXPRESSION_LEN] = {};
    // GetTexTreeData(derivative_2.root_ptr, tex_derivative_2, false);

    // fprintf(OutputFile, "In total, we imeem(poimeem): \\newline\\[f'(x) = %s\\]\n\\newline\n", tex_derivative_2);
    // PrintChangedVarsTex(&derivative_2, OutputFile);

// DIFF_DUMP(&derivative_2);
    // fprintf(OutputFile, "Having counted the most obvious derivative, which the Soviet spermatozoa were actually able to calculate in their minds, we get: \\[ %s \\]\n \\newline ", tex_derivative);
    // DIFF_DUMP(&derivative);
    // SimplifyExpr(&derivative, derivative.root_ptr);

    // char tex_derivative_simpl[TEX_EXPRESSION_LEN] = {};
    // GetTexTreeData(derivative.root_ptr, tex_derivative_simpl, false);

    // fprintf(OutputFile, "By simple mathematical transformations: \\[ %s \\]\n", tex_derivative_simpl);
    // PrintChangedVarsTex(&derivative, OutputFile);

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
    TreeDtor(&derivative);

    fprintf(stderr, "END!\n");
    return 0;
}