#include <stdio.h>

#include <math.h>

#include "diff_tree.h"
#include "reader.h"
#include "tex_work.h"
#include "diff_debug.h"
#include "derivative.h"
#include "simplifier.h"
#include "gnuplot.h"

FILE *OutputFile = NULL;
FILE *InputFile  = NULL;

void DerivativeMode (FILE *source, FILE *dest);
void TailorMode(FILE *source, FILE *dest, size_t order);

int main(const int argc, const char *argv[])
{
    fprintf(stderr, "START!\n");

    InputFile  = GetInputFile  (argc, argv);
    OutputFile = GetOutputFile (argc, argv);

    #if defined(TAYLOR)
    TailorMode(InputFile, OutputFile, 5);
    #elif defined(DERIVATIVE)
    DerivativeMode(InputFile, OutputFile);
    #endif

    fprintf(stderr, "END!\n");
    return 0;
}

void DerivativeMode(FILE *source, FILE *dest)
{
    Tree orig = {};
    TreeCtor(&orig, START_TREE_SIZE ON_DIFF_DEBUG(, "orig_expression"));
    GetTreeData(&orig, source);
    DIFF_DUMP(&orig);

    char tex_orig[STR_EXPRESSION_LEN] = {};
    GetStrTreeData(orig.root_ptr, tex_orig, false, TEX);
    fprintf(dest, "The original expression has the form \\[f(x) = %s \\]\n\n", tex_orig);

    Tree orig_simpl = {};
    TreeCtor(&orig_simpl, START_TREE_SIZE, "orig_simpl");
    orig_simpl.root_ptr = TreeCopyPaste(&orig, &orig_simpl, orig.root_ptr);

    SimplifyExpr(&orig_simpl, orig_simpl.root_ptr);

    // MakeChanges(&orig_simpl, orig_simpl.root_ptr, dest);

    Tree diff_tree = {};
    TreeCtor(&diff_tree, START_TREE_SIZE ON_DIFF_DEBUG(, "diff_tree"));

    diff_tree.root_ptr = TakeDerivative(&orig_simpl, orig_simpl.root_ptr, &diff_tree);

    SimplifyExpr(&orig_simpl, orig_simpl.root_ptr);
    SimplifyExpr(&orig_simpl, orig_simpl.root_ptr);
    SimplifyExpr(&orig_simpl, orig_simpl.root_ptr);

    DIFF_DUMP(&orig_simpl);

    char tex_diff[STR_EXPRESSION_LEN] = {};
    GetStrTreeData(diff_tree.root_ptr, tex_diff, false, TEX);
    fprintf(dest, "The derivative \\[f(x) = %s \\]\n\n", tex_diff);

    PrintChangedVarsTex(&orig_simpl, dest);

    TreeDtor(&orig);
    TreeDtor(&orig_simpl);
}

void TailorMode(FILE *source, FILE *dest, size_t order)
{
    Tree orig = {};
    TreeCtor(&orig, START_TREE_SIZE ON_DIFF_DEBUG(, "orig_expression"));
    GetTreeData(&orig, source);
    DIFF_DUMP(&orig);

    char tex_orig[STR_EXPRESSION_LEN] = {};
    GetStrTreeData(orig.root_ptr, tex_orig, false, TEX);
    fprintf(dest, "The original expression has the form \\[f(x) = %s \\]\n\n", tex_orig);

    Tree orig_simpl = {};
    TreeCtor(&orig_simpl, START_TREE_SIZE, "orig_simpl");
    orig_simpl.root_ptr = TreeCopyPaste(&orig, &orig_simpl, orig.root_ptr);
    
    DIFF_DUMP(&orig_simpl);
    SimplifyExpr(&orig_simpl, orig_simpl.root_ptr);
    DIFF_DUMP(&orig_simpl);

    Tree tailor = {};
    TreeCtor(&tailor, START_TREE_SIZE ON_DIFF_DEBUG(, "tailor"));
    CalculateTailor(&orig_simpl, orig_simpl.root_ptr, &tailor, order);

    MakeTailorChart(&orig_simpl, &tailor, TEX_FOLDER CHART_NAME);

    char tex_tailor[STR_EXPRESSION_LEN] = {};
    GetStrTreeData(tailor.root_ptr, tex_tailor, false, TEX);
    
    fprintf(dest, "tailor: \n\\[f(x) = %s\\]\n", tex_tailor);

    DrawChart(dest, CHART_NAME);   
}