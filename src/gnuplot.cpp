#include <stdio.h>
#include <Windows.h>

#include "diff_tree.h"
#include "tex_work.h"
#include "gnuplot.h"

void MakeTailorChart(Tree *expr_tree, Tree *tailor_tree, const char *chart_name)
{
    FILE *script = fopen(TMP_SCRIPT_FILE_NAME, "w");

    char expr_str[STR_EXPRESSION_LEN] = {};
    GetStrTreeData(expr_tree->root_ptr, expr_str, true, GNUPLOT);

    char tailor_str[STR_EXPRESSION_LEN] = {};
    GetStrTreeData(tailor_tree->root_ptr, tailor_str, true, GNUPLOT);

    fprintf(script, "set terminal pngcairo enhanced font \"Verdana,12\"                                                                                                 \n"      
                    "set output \"%s\"                                                                                                                                  \n"
                    "set samples 10000                                                                                                                                  \n"
                    "set title \"Taylor decomposition\" font \"Verdana,14\" textcolor rgb \"blue\"                                                                      \n"
                    "set xzeroaxis                                                                                                                                      \n"
                    "set yzeroaxis                                                                                                                                      \n"
                    "set xrange [-%d:%d]                                                                                                                                \n"
                    "set yrange [-%d:%d]                                                                                                                                \n"
                    "set key top right box                                                                                                                              \n"
                    "plot %s title \"expr\"  lc rgb \"%s\", %s title \"tailor\" lc rgb \"%s\", (%s - %s) title \"difference\" lc rgb \"%s\"                             \n"
                    , chart_name, X_RANGE, X_RANGE, Y_RANGE, Y_RANGE, expr_str, EXPR_COLOR, tailor_str, TAILOR_COLOR, expr_str, tailor_str, DIFFERENCE_COLOR);

    // remove()
    fclose(script);

    system("gnuplot " TMP_SCRIPT_FILE_NAME);
}